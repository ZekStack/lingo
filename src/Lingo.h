#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace lingo_internal {

struct LingoRawEntry {
	uint16_t key = 0;
	const char *value = nullptr;
};

template <typename T, bool = std::is_enum_v<T>> struct LingoEnumTraits {
	static constexpr bool valid = false;
};

template <typename T> struct LingoEnumTraits<T, true> {
	using Underlying = std::underlying_type_t<T>;
	static constexpr bool valid =
	    std::is_unsigned_v<Underlying> && sizeof(Underlying) <= sizeof(uint16_t);
};

template <typename T> inline constexpr bool isLingoEnum = LingoEnumTraits<T>::valid;

template <typename T> struct LingoDomainToken {
	static inline uint8_t value = 0;
};

template <typename T> const void *domainToken() {
	static_assert(isLingoEnum<T>, "Lingo enums must use an unsigned underlying type up to 16 bits");
	return &LingoDomainToken<T>::value;
}

template <typename T> constexpr uint16_t enumValue(T value) {
	static_assert(isLingoEnum<T>, "Lingo enums must use an unsigned underlying type up to 16 bits");
	return static_cast<uint16_t>(value);
}

struct LingoRegisteredTable;

} // namespace lingo_internal

enum class LingoStatus : uint8_t {
	Ok,
	NotInitialized,
	AlreadyInitialized,
	InvalidConfig,
	InvalidArgument,
	AllocationFailed,
	TableCapacityExceeded,
	TableAlreadyRegistered,
	DuplicateKey,
	InvalidTranslation,
	LanguageNotRegistered,
};

struct LingoResult {
	LingoStatus status = LingoStatus::Ok;
	const char *message = "ok";

	explicit operator bool() const {
		return status == LingoStatus::Ok;
	}

	static LingoResult success(const char *message = "ok") {
		return {LingoStatus::Ok, message};
	}

	static LingoResult failure(LingoStatus status, const char *message) {
		return {status, message};
	}
};

class LingoLanguage {
  public:
	constexpr LingoLanguage() = default;

	template <typename TLanguage>
	constexpr LingoLanguage(TLanguage language) : _value(lingo_internal::enumValue(language)) {
	}

	template <typename TLanguage> constexpr LingoLanguage &operator=(TLanguage language) {
		_value = lingo_internal::enumValue(language);
		return *this;
	}

	constexpr uint16_t value() const {
		return _value;
	}

  private:
	uint16_t _value = 0;
};

struct LingoConfig {
	LingoLanguage defaultLanguage{};
	size_t maxTables = 64;
	const char *missingTranslation = "";
	bool preferPsram = true;
};

template <typename TKey> class LingoEntry {
  public:
	using KeyType = TKey;

	constexpr LingoEntry(TKey key, const char *value)
	    : _entry{lingo_internal::enumValue(key), value} {
	}

	constexpr lingo_internal::LingoRawEntry raw() const {
		return _entry;
	}

  private:
	lingo_internal::LingoRawEntry _entry;
};

template <typename TKey> LingoEntry(TKey, const char *) -> LingoEntry<TKey>;

template <typename TKey, size_t N> class LingoTable {
  public:
	using KeyType = TKey;
	static constexpr size_t EntryCount = N;

	template <typename... TRest>
	constexpr explicit LingoTable(LingoEntry<TKey> first, TRest... rest)
	    : _entries{first.raw(), rest.raw()...} {
		static_assert(sizeof...(TRest) + 1 == N);
		static_assert(
		    (std::is_same_v<LingoEntry<TKey>, std::remove_cvref_t<TRest>> && ...),
		    "all LingoTable entries must use the same key enum type"
		);
	}

	constexpr const lingo_internal::LingoRawEntry *data() const {
		return _entries.data();
	}

	static constexpr size_t size() {
		return N;
	}

  private:
	std::array<lingo_internal::LingoRawEntry, N> _entries;
};

template <typename TKey, typename... TRest>
LingoTable(LingoEntry<TKey>, TRest...) -> LingoTable<TKey, sizeof...(TRest) + 1>;

class Lingo {
  public:
	Lingo() = default;
	~Lingo();

	Lingo(const Lingo &) = delete;
	Lingo &operator=(const Lingo &) = delete;

	LingoResult init(const LingoConfig &config = LingoConfig());
	LingoResult end();
	bool initialized() const;

	template <typename TLanguage, typename TKey, size_t N>
	LingoResult addTable(TLanguage language, const LingoTable<TKey, N> &table) {
		static_assert(
		    lingo_internal::isLingoEnum<TLanguage>,
		    "Lingo language enums must use an unsigned underlying type up to 16 bits"
		);
		return addTableRaw(
		    lingo_internal::enumValue(language),
		    lingo_internal::domainToken<TKey>(),
		    table.data(),
		    table.size()
		);
	}

	template <typename TLanguage> LingoResult setDefaultLanguage(TLanguage language) {
		static_assert(
		    lingo_internal::isLingoEnum<TLanguage>,
		    "Lingo language enums must use an unsigned underlying type up to 16 bits"
		);
		return setDefaultLanguageRaw(lingo_internal::enumValue(language));
	}

	template <typename TKey> const char *find(TKey key) const {
		static_assert(
		    lingo_internal::isLingoEnum<TKey>,
		    "Lingo key enums must use an unsigned underlying type up to 16 bits"
		);
		return findRaw(
		    lingo_internal::domainToken<TKey>(),
		    lingo_internal::enumValue(key),
		    _defaultLanguage.load(std::memory_order_acquire)
		);
	}

	template <typename TKey, typename TLanguage>
	const char *find(TKey key, TLanguage language) const {
		static_assert(
		    lingo_internal::isLingoEnum<TKey>,
		    "Lingo key enums must use an unsigned underlying type up to 16 bits"
		);
		static_assert(
		    lingo_internal::isLingoEnum<TLanguage>,
		    "Lingo language enums must use an unsigned underlying type up to 16 bits"
		);
		return findRaw(
		    lingo_internal::domainToken<TKey>(),
		    lingo_internal::enumValue(key),
		    lingo_internal::enumValue(language)
		);
	}

	template <typename TKey> const char *get(TKey key) const {
		const char *translation = find(key);
		return translation == nullptr ? missingTranslation() : translation;
	}

	template <typename TKey, typename TLanguage>
	const char *get(TKey key, TLanguage language) const {
		static_assert(
		    lingo_internal::isLingoEnum<TKey>,
		    "Lingo key enums must use an unsigned underlying type up to 16 bits"
		);
		static_assert(
		    lingo_internal::isLingoEnum<TLanguage>,
		    "Lingo language enums must use an unsigned underlying type up to 16 bits"
		);

		const uint16_t requestedLanguage = lingo_internal::enumValue(language);
		const void *domain = lingo_internal::domainToken<TKey>();
		const uint16_t keyValue = lingo_internal::enumValue(key);

		const char *translation = findRaw(domain, keyValue, requestedLanguage);
		if (translation != nullptr) {
			return translation;
		}

		const uint16_t defaultLanguage = _defaultLanguage.load(std::memory_order_acquire);
		if (requestedLanguage != defaultLanguage) {
			translation = findRaw(domain, keyValue, defaultLanguage);
			if (translation != nullptr) {
				return translation;
			}
		}

		return missingTranslation();
	}

	size_t tableCount() const;
	size_t tableCapacity() const;
	bool preferPsram() const;

  private:
	LingoResult addTableRaw(
	    uint16_t language,
	    const void *domain,
	    const lingo_internal::LingoRawEntry *entries,
	    size_t count
	);
	LingoResult setDefaultLanguageRaw(uint16_t language);
	const char *findRaw(const void *domain, uint16_t key, uint16_t language) const;
	const char *missingTranslation() const;
	bool languageRegistered(uint16_t language) const;

	lingo_internal::LingoRegisteredTable *_tables = nullptr;
	size_t _tableCount = 0;
	size_t _tableCapacity = 0;
	const char *_missingTranslation = "";
	bool _preferPsram = true;
	bool _initialized = false;
	std::atomic<uint16_t> _defaultLanguage{0};
};

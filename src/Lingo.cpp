#include "Lingo.h"

#include <limits>
#include <memory>

namespace lingo_internal {

struct LingoRegisteredTable {
	uint16_t language = 0;
	const void *domain = nullptr;
	const LingoRawEntry *entries = nullptr;
	size_t count = 0;
};

} // namespace lingo_internal

namespace {

constexpr const char *kEmptyTranslation = "";

} // namespace

Lingo::~Lingo() {
	end();
}

LingoResult Lingo::init(const LingoConfig &config) {
	if (_initialized) {
		return LingoResult::failure(
		    LingoStatus::AlreadyInitialized,
		    "lingo is already initialized"
		);
	}

	if (config.maxTables == 0 || config.missingTranslation == nullptr ||
	    !Strata::validMemoryPolicy(config.memory)) {
		return LingoResult::failure(LingoStatus::InvalidConfig, "invalid lingo configuration");
	}

	if (config.maxTables >
	    std::numeric_limits<size_t>::max() / sizeof(lingo_internal::LingoRegisteredTable)) {
		return LingoResult::failure(
		    LingoStatus::InvalidConfig,
		    "translation table capacity is too large"
		);
	}

	auto *tables = Strata::allocateArray<lingo_internal::LingoRegisteredTable>(
	    config.maxTables,
	    config.memory.allocation
	);
	if (tables == nullptr) {
		return LingoResult::failure(
		    LingoStatus::AllocationFailed,
		    "translation table registry allocation failed"
		);
	}

	for (size_t index = 0; index < config.maxTables; ++index) {
		std::construct_at(&tables[index]);
	}

	_tables = tables;
	_tableCount = 0;
	_tableCapacity = config.maxTables;
	_missingTranslation = config.missingTranslation;
	_registryPlacement = config.memory.allocation;
	_defaultLanguage.store(config.defaultLanguage.value(), std::memory_order_release);
	_initialized = true;

	return LingoResult::success();
}

LingoResult Lingo::end() {
	if (!_initialized) {
		return LingoResult::success("lingo is already stopped");
	}

	for (size_t index = 0; index < _tableCapacity; ++index) {
		std::destroy_at(&_tables[index]);
	}

	Strata::free(_tables);
	_tables = nullptr;
	_tableCount = 0;
	_tableCapacity = 0;
	_missingTranslation = kEmptyTranslation;
	_registryPlacement = Strata::Placement::PreferExternal;
	_defaultLanguage.store(0, std::memory_order_release);
	_initialized = false;

	return LingoResult::success();
}

bool Lingo::initialized() const {
	return _initialized;
}

LingoResult Lingo::addTableRaw(
    uint16_t language,
    const void *domain,
    const lingo_internal::LingoRawEntry *entries,
    size_t count
) {
	if (!_initialized) {
		return LingoResult::failure(LingoStatus::NotInitialized, "lingo is not initialized");
	}

	if (domain == nullptr || entries == nullptr || count == 0) {
		return LingoResult::failure(LingoStatus::InvalidArgument, "invalid translation table");
	}

	for (size_t index = 0; index < _tableCount; ++index) {
		const auto &registered = _tables[index];
		if (registered.language == language && registered.domain == domain) {
			return LingoResult::failure(
			    LingoStatus::TableAlreadyRegistered,
			    "translation table is already registered for language and domain"
			);
		}
	}

	if (_tableCount >= _tableCapacity) {
		return LingoResult::failure(
		    LingoStatus::TableCapacityExceeded,
		    "translation table registry is full"
		);
	}

	for (size_t index = 0; index < count; ++index) {
		if (entries[index].value == nullptr) {
			return LingoResult::failure(
			    LingoStatus::InvalidTranslation,
			    "translation value must not be null"
			);
		}

		for (size_t other = index + 1; other < count; ++other) {
			if (entries[index].key == entries[other].key) {
				return LingoResult::failure(
				    LingoStatus::DuplicateKey,
				    "translation table contains duplicate keys"
				);
			}
		}
	}

	_tables[_tableCount] = {
	    language,
	    domain,
	    entries,
	    count,
	};
	++_tableCount;

	return LingoResult::success();
}

LingoResult Lingo::setDefaultLanguageRaw(uint16_t language) {
	if (!_initialized) {
		return LingoResult::failure(LingoStatus::NotInitialized, "lingo is not initialized");
	}

	if (!languageRegistered(language)) {
		return LingoResult::failure(
		    LingoStatus::LanguageNotRegistered,
		    "language has no registered translation tables"
		);
	}

	_defaultLanguage.store(language, std::memory_order_release);
	return LingoResult::success();
}

const char *Lingo::findRaw(const void *domain, uint16_t key, uint16_t language) const {
	if (!_initialized || domain == nullptr) {
		return nullptr;
	}

	for (size_t tableIndex = 0; tableIndex < _tableCount; ++tableIndex) {
		const auto &table = _tables[tableIndex];
		if (table.language != language || table.domain != domain) {
			continue;
		}

		for (size_t entryIndex = 0; entryIndex < table.count; ++entryIndex) {
			if (table.entries[entryIndex].key == key) {
				return table.entries[entryIndex].value;
			}
		}

		return nullptr;
	}

	return nullptr;
}

const char *Lingo::missingTranslation() const {
	if (!_initialized || _missingTranslation == nullptr) {
		return kEmptyTranslation;
	}
	return _missingTranslation;
}

bool Lingo::languageRegistered(uint16_t language) const {
	for (size_t index = 0; index < _tableCount; ++index) {
		if (_tables[index].language == language) {
			return true;
		}
	}
	return false;
}

size_t Lingo::tableCount() const {
	return _tableCount;
}

size_t Lingo::tableCapacity() const {
	return _tableCapacity;
}

LingoDiag Lingo::getDiagnostics() const {
	return LingoDiag{
	    .tableCount = _tableCount,
	    .tableCapacity = _tableCapacity,
	    .registryPlacement = _registryPlacement,
	    .registryRegion = _tables == nullptr ? Strata::Region::Unknown : Strata::regionOf(_tables),
	};
}

#include <Lingo.h>

#include <cassert>
#include <cstring>
#include <iostream>

enum class Language : uint16_t {
	Hu,
	En,
	De,
};

enum class CommonKey : uint16_t {
	Save = 0,
	Cancel = 1,
};

enum class SoftwareKey : uint16_t {
	Save = 0,
	UpdateAvailable = 1,
	Install = 2,
};

enum class TimeKey : uint16_t {
	Save = 0,
	Timezone = 7,
};

constexpr LingoTable COMMON_HU{
    LingoEntry(CommonKey::Save, "Mentés"),
    LingoEntry(CommonKey::Cancel, "Mégse"),
};

constexpr LingoTable COMMON_EN{
    LingoEntry(CommonKey::Save, "Save"),
    LingoEntry(CommonKey::Cancel, "Cancel"),
};

constexpr LingoTable SOFTWARE_HU{
    LingoEntry(SoftwareKey::Save, "Mentés"),
    LingoEntry(SoftwareKey::UpdateAvailable, "Frissítés érhető el"),
};

constexpr LingoTable SOFTWARE_EN{
    LingoEntry(SoftwareKey::Save, "Save update"),
    LingoEntry(SoftwareKey::UpdateAvailable, "Update available"),
    LingoEntry(SoftwareKey::Install, "Install"),
};

constexpr LingoTable TIME_HU{
    LingoEntry(TimeKey::Save, "Idő mentése"),
    LingoEntry(TimeKey::Timezone, "Időzóna"),
};

constexpr LingoTable TIME_EN{
    LingoEntry(TimeKey::Save, "Save time"),
    LingoEntry(TimeKey::Timezone, "Timezone"),
};

void testLifecycle() {
	Lingo lingo;

	assert(!lingo.initialized());
	assert(std::strcmp(lingo.get(CommonKey::Save), "") == 0);
	assert(lingo.find(CommonKey::Save) == nullptr);

	LingoConfig invalid;
	invalid.maxTables = 0;
	assert(!lingo.init(invalid));

	LingoConfig invalidMissing;
	invalidMissing.missingTranslation = nullptr;
	assert(!lingo.init(invalidMissing));

	LingoConfig config;
	config.defaultLanguage = Language::Hu;
	config.maxTables = 8;
	config.missingTranslation = "<missing>";
	config.memory.allocation = Strata::Placement::PreferExternal;

	assert(lingo.init(config));
	assert(lingo.initialized());
	assert(lingo.tableCapacity() == 8);
	const LingoDiag diag = lingo.getDiagnostics();
	assert(diag.tableCount == 0);
	assert(diag.tableCapacity == 8);
	assert(diag.registryPlacement == Strata::Placement::PreferExternal);
	assert(!lingo.init(config));

	assert(lingo.end());
	assert(!lingo.initialized());
	assert(lingo.getDiagnostics().registryRegion == Strata::Region::Unknown);
	assert(lingo.end());

	assert(lingo.init(config));
	assert(lingo.end());
}

void testRegistrationAndDomainIsolation() {
	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::Hu;
	config.maxTables = 6;
	assert(lingo.init(config));

	assert(lingo.addTable(Language::Hu, COMMON_HU));
	assert(lingo.addTable(Language::En, COMMON_EN));
	assert(lingo.addTable(Language::Hu, SOFTWARE_HU));
	assert(lingo.addTable(Language::En, SOFTWARE_EN));
	assert(lingo.addTable(Language::Hu, TIME_HU));
	assert(lingo.addTable(Language::En, TIME_EN));

	assert(lingo.tableCount() == 6);
	assert(lingo.getDiagnostics().tableCount == 6);

	assert(std::strcmp(lingo.get(CommonKey::Save), "Mentés") == 0);
	assert(std::strcmp(lingo.get(SoftwareKey::Save), "Mentés") == 0);
	assert(std::strcmp(lingo.get(TimeKey::Save), "Idő mentése") == 0);

	assert(std::strcmp(lingo.get(CommonKey::Save, Language::En), "Save") == 0);
	assert(std::strcmp(lingo.get(SoftwareKey::Save, Language::En), "Save update") == 0);
	assert(std::strcmp(lingo.get(TimeKey::Save, Language::En), "Save time") == 0);

	LingoResult duplicate = lingo.addTable(Language::Hu, COMMON_HU);
	assert(!duplicate);
	assert(duplicate.status == LingoStatus::TableAlreadyRegistered);
}

void testLookupFallbackAndLanguageSwitching() {
	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.maxTables = 4;
	config.missingTranslation = "<missing>";
	assert(lingo.init(config));

	assert(lingo.addTable(Language::Hu, SOFTWARE_HU));
	assert(lingo.addTable(Language::En, SOFTWARE_EN));

	assert(std::strcmp(lingo.get(SoftwareKey::Install, Language::Hu), "Install") == 0);
	assert(lingo.find(SoftwareKey::Install, Language::Hu) == nullptr);

	assert(lingo.setDefaultLanguage(Language::Hu));
	assert(std::strcmp(lingo.get(SoftwareKey::UpdateAvailable), "Frissítés érhető el") == 0);
	assert(std::strcmp(lingo.get(SoftwareKey::Install), "<missing>") == 0);

	assert(!lingo.setDefaultLanguage(Language::De));
}

void testValidationAndCapacity() {
	enum class DuplicateKey : uint16_t {
		First = 1,
		Second = 1,
	};

	enum class NullKey : uint16_t {
		Value,
	};

	enum class ExtraKey : uint16_t {
		Value,
	};

	const LingoTable duplicateTable{
	    LingoEntry(DuplicateKey::First, "one"),
	    LingoEntry(DuplicateKey::Second, "two"),
	};
	const LingoTable nullTable{
	    LingoEntry(NullKey::Value, nullptr),
	};
	const LingoTable extraTable{
	    LingoEntry(ExtraKey::Value, "extra"),
	};

	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::Hu;
	config.maxTables = 1;
	assert(lingo.init(config));

	LingoResult duplicate = lingo.addTable(Language::Hu, duplicateTable);
	assert(!duplicate);
	assert(duplicate.status == LingoStatus::DuplicateKey);

	LingoResult nullResult = lingo.addTable(Language::Hu, nullTable);
	assert(!nullResult);
	assert(nullResult.status == LingoStatus::InvalidTranslation);

	assert(lingo.addTable(Language::Hu, COMMON_HU));

	LingoResult full = lingo.addTable(Language::Hu, extraTable);
	assert(!full);
	assert(full.status == LingoStatus::TableCapacityExceeded);
}

void testUtf8() {
	enum class Utf8Key : uint16_t {
		Hungarian,
	};

	constexpr LingoTable table{
	    LingoEntry(Utf8Key::Hungarian, "Árvíztűrő tükörfúrógép"),
	};

	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::Hu;
	assert(lingo.init(config));
	assert(lingo.addTable(Language::Hu, table));
	assert(std::strcmp(lingo.get(Utf8Key::Hungarian), "Árvíztűrő tükörfúrógép") == 0);
}

int main() {
	testLifecycle();
	testRegistrationAndDomainIsolation();
	testLookupFallbackAndLanguageSwitching();
	testValidationAndCapacity();
	testUtf8();

	std::cout << "Lingo host tests passed\n";
	return 0;
}

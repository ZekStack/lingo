#include <Arduino.h>
#include <Lingo.h>

Lingo lingo;

enum class Language : uint16_t {
	Hu,
	En,
};

enum class CommonKey : uint16_t {
	Save = 0,
};

enum class SoftwareKey : uint16_t {
	Save = 0,
	UpdateAvailable = 1,
};

enum class TimeKey : uint16_t {
	Save = 0,
	Timezone = 1,
};

constexpr LingoTable COMMON_HU{
    LingoEntry(CommonKey::Save, "Mentés"),
};

constexpr LingoTable COMMON_EN{
    LingoEntry(CommonKey::Save, "Save"),
};

constexpr LingoTable SOFTWARE_HU{
    LingoEntry(SoftwareKey::Save, "Frissítés mentése"),
    LingoEntry(SoftwareKey::UpdateAvailable, "Frissítés érhető el"),
};

constexpr LingoTable SOFTWARE_EN{
    LingoEntry(SoftwareKey::Save, "Save update"),
    LingoEntry(SoftwareKey::UpdateAvailable, "Update available"),
};

constexpr LingoTable TIME_HU{
    LingoEntry(TimeKey::Save, "Idő mentése"),
    LingoEntry(TimeKey::Timezone, "Időzóna"),
};

constexpr LingoTable TIME_EN{
    LingoEntry(TimeKey::Save, "Save time"),
    LingoEntry(TimeKey::Timezone, "Timezone"),
};

void setup() {
	Serial.begin(115200);

	LingoConfig config;
	config.defaultLanguage = Language::Hu;
	config.maxTables = 8;
	config.memory.allocation = Strata::Placement::PreferExternal;

	if (!lingo.init(config)) {
		return;
	}

	lingo.addTable(Language::Hu, COMMON_HU);
	lingo.addTable(Language::En, COMMON_EN);
	lingo.addTable(Language::Hu, SOFTWARE_HU);
	lingo.addTable(Language::En, SOFTWARE_EN);
	lingo.addTable(Language::Hu, TIME_HU);
	lingo.addTable(Language::En, TIME_EN);

	Serial.println(lingo.get(CommonKey::Save));
	Serial.println(lingo.get(SoftwareKey::Save));
	Serial.println(lingo.get(TimeKey::Save));
}

void loop() {
	delay(1000);
}

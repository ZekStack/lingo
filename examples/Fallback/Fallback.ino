#include <Arduino.h>
#include <Lingo.h>

Lingo lingo;

enum class Language : uint16_t {
	Hu,
	En,
};

enum class SoftwareKey : uint16_t {
	UpdateAvailable,
	Install,
};

constexpr LingoTable SOFTWARE_HU{
    LingoEntry(SoftwareKey::UpdateAvailable, "Frissítés érhető el"),
};

constexpr LingoTable SOFTWARE_EN{
    LingoEntry(SoftwareKey::UpdateAvailable, "Update available"),
    LingoEntry(SoftwareKey::Install, "Install"),
};

void setup() {
	Serial.begin(115200);

	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.missingTranslation = "<missing>";

	if (!lingo.init(config)) {
		return;
	}

	lingo.addTable(Language::Hu, SOFTWARE_HU);
	lingo.addTable(Language::En, SOFTWARE_EN);

	// Hungarian does not define Install, so get() falls back to the default English table.
	Serial.println(lingo.get(SoftwareKey::Install, Language::Hu));

	// find() is strict and does not fall back.
	const char *strictHungarian = lingo.find(SoftwareKey::Install, Language::Hu);
	Serial.println(strictHungarian == nullptr ? "not translated" : strictHungarian);
}

void loop() {
	delay(1000);
}

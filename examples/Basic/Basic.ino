#include <Arduino.h>
#include <Lingo.h>

Lingo lingo;

enum class Language : uint16_t {
	Hu,
	En,
};

enum class CommonKey : uint16_t {
	Key,
	Example,
};

constexpr LingoTable HU_TABLE{
    LingoEntry(CommonKey::Key, "Kulcs"),
    LingoEntry(CommonKey::Example, "Példa"),
};

constexpr LingoTable EN_TABLE{
    LingoEntry(CommonKey::Key, "Key"),
    LingoEntry(CommonKey::Example, "Example"),
};

void setup() {
	Serial.begin(115200);

	LingoConfig config;
	config.defaultLanguage = Language::Hu;
	config.memory.allocation = Strata::Placement::PreferExternal;

	if (!lingo.init(config)) {
		return;
	}

	lingo.addTable(Language::Hu, HU_TABLE);
	lingo.addTable(Language::En, EN_TABLE);

	Serial.println(lingo.get(CommonKey::Key));
	Serial.println(lingo.get(CommonKey::Example, Language::En));

	const LingoDiag diag = lingo.getDiagnostics();
	Serial.printf(
	    "registry placement=%u region=%u\n",
	    static_cast<unsigned>(diag.registryPlacement),
	    static_cast<unsigned>(diag.registryRegion)
	);

	lingo.setDefaultLanguage(Language::En);
	Serial.println(lingo.get(CommonKey::Key));
}

void loop() {
	delay(1000);
}

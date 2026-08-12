#include <Lingo.h>

#include <esp_heap_caps.h>

#include <cassert>
#include <iostream>

enum class Language : uint16_t {
	En,
};

void testPsramPreferredWhenAvailable() {
	lingo_psram_test::reset();
	lingo_psram_test::spiramTotal = 1024 * 1024;

	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.preferPsram = true;

	assert(lingo.init(config));
	assert(lingo_psram_test::spiramAllocations == 1);
	assert(lingo_psram_test::internalAllocations == 0);
}

void testInternalHeapFallbackWhenPsramAllocationFails() {
	lingo_psram_test::reset();
	lingo_psram_test::spiramTotal = 1024 * 1024;
	lingo_psram_test::failSpiramAllocation = true;

	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.preferPsram = true;

	assert(lingo.init(config));
	assert(lingo_psram_test::spiramAllocations == 1);
	assert(lingo_psram_test::internalAllocations == 1);
}

void testInternalHeapWhenPsramUnavailable() {
	lingo_psram_test::reset();
	lingo_psram_test::spiramTotal = 0;

	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.preferPsram = true;

	assert(lingo.init(config));
	assert(lingo_psram_test::spiramAllocations == 0);
	assert(lingo_psram_test::internalAllocations == 1);
}

void testInternalHeapWhenPsramPreferenceDisabled() {
	lingo_psram_test::reset();
	lingo_psram_test::spiramTotal = 1024 * 1024;

	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.preferPsram = false;

	assert(lingo.init(config));
	assert(lingo_psram_test::spiramAllocations == 0);
	assert(lingo_psram_test::internalAllocations == 1);
}

int main() {
	testPsramPreferredWhenAvailable();
	testInternalHeapFallbackWhenPsramAllocationFails();
	testInternalHeapWhenPsramUnavailable();
	testInternalHeapWhenPsramPreferenceDisabled();

	std::cout << "Lingo PSRAM allocation tests passed\n";
	return 0;
}

#include <Lingo.h>

#include <cassert>
#include <iostream>

enum class Language : uint16_t {
	En,
};

void testDefaultPreservesExternalPreference() {
	LingoConfig config;
	assert(config.memory.allocation == Strata::Placement::PreferExternal);
	assert(config.memory.taskStack == Strata::Placement::Internal);

	Lingo lingo;
	config.defaultLanguage = Language::En;
	assert(lingo.init(config));

	const LingoDiag diag = lingo.getDiagnostics();
	assert(diag.registryPlacement == Strata::Placement::PreferExternal);
	assert(diag.registryRegion == Strata::Region::Unknown);
}

void testExplicitInternalPlacement() {
	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.memory.allocation = Strata::Placement::Internal;

	assert(lingo.init(config));
	assert(lingo.getDiagnostics().registryPlacement == Strata::Placement::Internal);
}

void testExplicitBackendDefaultPlacement() {
	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.memory.allocation = Strata::Placement::Default;

	assert(lingo.init(config));
	assert(lingo.getDiagnostics().registryPlacement == Strata::Placement::Default);
}

void testRequiredExternalFailsWithoutExternalMemory() {
	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.memory.allocation = Strata::Placement::RequireExternal;

	const LingoResult result = lingo.init(config);
	assert(!result);
	assert(result.status == LingoStatus::AllocationFailed);
	assert(!lingo.initialized());
}

void testInvalidMemoryPolicyRejected() {
	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.memory.allocation = static_cast<Strata::Placement>(0xff);

	const LingoResult allocationResult = lingo.init(config);
	assert(!allocationResult);
	assert(allocationResult.status == LingoStatus::InvalidConfig);

	config.memory.allocation = Strata::Placement::PreferExternal;
	config.memory.taskStack = static_cast<Strata::Placement>(0xff);
	const LingoResult stackResult = lingo.init(config);
	assert(!stackResult);
	assert(stackResult.status == LingoStatus::InvalidConfig);
}

void testEndResetsRegistryDiagnostics() {
	Lingo lingo;
	LingoConfig config;
	config.defaultLanguage = Language::En;
	config.memory.allocation = Strata::Placement::Internal;

	assert(lingo.init(config));
	assert(lingo.end());

	const LingoDiag diag = lingo.getDiagnostics();
	assert(diag.tableCount == 0);
	assert(diag.tableCapacity == 0);
	assert(diag.registryPlacement == Strata::Placement::PreferExternal);
	assert(diag.registryRegion == Strata::Region::Unknown);
}

int main() {
	testDefaultPreservesExternalPreference();
	testExplicitInternalPlacement();
	testExplicitBackendDefaultPlacement();
	testRequiredExternalFailsWithoutExternalMemory();
	testInvalidMemoryPolicyRejected();
	testEndResetsRegistryDiagnostics();

	std::cout << "Lingo Strata memory policy tests passed\n";
	return 0;
}

# Lingo

Lingo is a strongly typed, feature-oriented translation library for Arduino ESP32.

It is designed for firmware where each feature owns a small translation table, lookup must stay allocation-free, and the bounded runtime registry should follow the same Strata memory policy used across ZekStack libraries.

[![CI](https://github.com/ZekStack/lingo/actions/workflows/ci.yml/badge.svg)](https://github.com/ZekStack/lingo/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/ZekStack/lingo?sort=semver)](https://github.com/ZekStack/lingo/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE.md)

## Why use Lingo?

* **Feature-owned tables** - register many small translation domains per language instead of one global dictionary.
* **Strongly typed keys** - each enum type is an isolated translation domain, so identical numeric values cannot collide across features.
* **Strata memory policy** - registry allocation uses `Strata::MemoryPolicy` and defaults to external-preferred allocation with safe internal fallback.
* **Allocation-free lookup** - translation tables and strings are referenced directly; `get()` and `find()` do not allocate.
* **No RTTI** - enum domains use unique static type tokens without `typeid` or `std::type_index`.
* **Safe C strings** - `get()` always returns a valid `const char *`, suitable for logging and embedded display APIs.
* **Explicit fallback** - requested-language lookup can fall back to the configured default language within the same feature domain.
* **Placement diagnostics** - requested registry placement and the observed memory region are reported separately.

## Install

Lingo v0.2.0 depends on [Strata](https://github.com/ZekStack/strata) v0.1.2.

### PlatformIO

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

lib_deps =
  https://github.com/ZekStack/lingo.git

build_flags =
  -std=gnu++20
build_unflags =
  -std=gnu++11
```

Lingo's `library.json` pins Strata v0.1.2.

### Arduino IDE

Lingo is not published to Arduino Library Manager yet. Install both repositories into the Arduino libraries directory:

```txt
Arduino/libraries/Lingo
Arduino/libraries/Strata
```

Use Strata v0.1.2 with Lingo v0.2.0.

## Quick start

```cpp
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
    LingoEntry(SoftwareKey::Install, "Telepítés"),
};

constexpr LingoTable SOFTWARE_EN{
    LingoEntry(SoftwareKey::UpdateAvailable, "Update available"),
    LingoEntry(SoftwareKey::Install, "Install"),
};

void setup() {
	LingoConfig config;
	config.defaultLanguage = Language::Hu;
	config.memory.allocation = Strata::Placement::PreferExternal;

	if (!lingo.init(config)) {
		return;
	}

	lingo.addTable(Language::Hu, SOFTWARE_HU);
	lingo.addTable(Language::En, SOFTWARE_EN);

	const char *current = lingo.get(SoftwareKey::Install);
	const char *english = lingo.get(SoftwareKey::Install, Language::En);

	const LingoDiag diag = lingo.getDiagnostics();
	Strata::Placement requested = diag.registryPlacement;
	Strata::Region actual = diag.registryRegion;

	lingo.setDefaultLanguage(Language::En);
}
```

`PreferExternal` is Lingo's default, so setting it explicitly is optional. It preserves the v0.1.x PSRAM-first behavior while allowing internal fallback.

A second feature can define its own enum starting at the same numeric values without collisions:

```cpp
enum class TimeKey : uint16_t {
	Save = 0,
	Timezone = 1,
};

lingo.addTable(Language::Hu, TIME_HU);
lingo.addTable(Language::En, TIME_EN);
```

## Memory placement

Lingo owns one dynamic allocation: the bounded translation-table registry created during `init()`.

```cpp
config.memory.allocation = Strata::Placement::Default;
config.memory.allocation = Strata::Placement::Internal;
config.memory.allocation = Strata::Placement::PreferExternal;
config.memory.allocation = Strata::Placement::RequireExternal;
```

`RequireExternal` never silently falls back. If Strata cannot satisfy the placement, `init()` returns `LingoStatus::AllocationFailed` and Lingo remains uninitialized.

Lingo creates no tasks, so `config.memory.taskStack` currently has no Lingo-owned task stack to control. The field remains part of the shared `Strata::MemoryPolicy` contract.

## Important notes

> [!IMPORTANT]
> Registered tables and their translation strings are not copied. Keep them alive until `lingo.end()`. `constexpr` or static tables containing string literals are the intended pattern.

* Exactly one table may be registered for a `{language, key enum type}` pair.
* Size `maxTables` for the total number of registered language/domain pairs; each feature table in each language consumes one slot.
* `get()` never returns `nullptr`; `find()` is the strict nullable lookup API.
* `get(key, language)` falls back to the default language only within the same enum domain.
* `init()` allocates the complete bounded registry once through Strata.
* `addTable()`, `get()`, `find()`, and `setDefaultLanguage()` do not allocate.
* After startup registration, concurrent `get()`/`find()` calls are supported. Do not mutate the table registry concurrently with lookups.
* Translation bytes are passed through unchanged, including UTF-8 text.

## Examples

| Example | Description |
| --- | --- |
| `Basic` | Initialize Lingo with Strata placement, register HU/EN tables, inspect registry diagnostics, and switch the default language. |
| `FeatureTables` | Register multiple feature domains per language with overlapping numeric key values. |
| `Fallback` | Demonstrate strict lookup and same-domain default-language fallback. |

Start with:

```txt
examples/Basic
```

## Documentation

| Document | Description |
| --- | --- |
| [`docs/getting-started.md`](docs/getting-started.md) | First integration, Strata memory placement, and table registration. |
| [`docs/api.md`](docs/api.md) | Public types, methods, and diagnostics. |
| [`docs/configuration.md`](docs/configuration.md) | Memory policy, registry sizing, and fallback string. |
| [`docs/tables-and-domains.md`](docs/tables-and-domains.md) | Feature-domain identity and registration rules. |
| [`docs/fallback.md`](docs/fallback.md) | Exact `get()` and `find()` resolution behavior. |
| [`docs/memory.md`](docs/memory.md) | Static translations, bounded allocation, and Strata ownership. |
| [`docs/thread-safety.md`](docs/thread-safety.md) | Supported runtime concurrency and lifecycle rules. |
| [`docs/troubleshooting.md`](docs/troubleshooting.md) | Common registration and lookup failures. |

## API overview

```cpp
LingoResult init(const LingoConfig &config);

lingo.addTable(Language::Hu, SOFTWARE_HU);
lingo.addTable(Language::En, SOFTWARE_EN);

const char *value = lingo.get(SoftwareKey::Install);
const char *hu = lingo.get(SoftwareKey::Install, Language::Hu);
const char *strict = lingo.find(SoftwareKey::Install, Language::Hu);

LingoDiag diag = lingo.getDiagnostics();
lingo.setDefaultLanguage(Language::En);
```

## Migrating from v0.1.x

```cpp
// v0.1.x: prefer PSRAM with internal fallback
config.preferPsram = true;

// v0.2.0
config.memory.allocation = Strata::Placement::PreferExternal;
```

```cpp
// v0.1.x: internal heap
config.preferPsram = false;

// v0.2.0
config.memory.allocation = Strata::Placement::Internal;
```

`Lingo::preferPsram()` was removed. Use `getDiagnostics().registryPlacement` for requested policy and `getDiagnostics().registryRegion` for the observed location.

## Compatibility

| Item | Support |
| --- | --- |
| Framework | Arduino ESP32 |
| Platform | `espressif32` |
| Language | C++20 |
| Dependencies | Strata v0.1.2 |
| RTTI | not required |
| Exceptions | not used for public error handling |
| Registry default | `Strata::Placement::PreferExternal` |
| Runtime lookup allocation | none |
| Status | v0.2.0 Strata memory-policy migration |

## License

MIT - see [`LICENSE.md`](LICENSE.md).

## ZekStack

Part of the ZekStack ESP32 library stack.

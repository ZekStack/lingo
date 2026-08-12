# Lingo

Lingo is a strongly typed, feature-oriented translation library for Arduino ESP32.

It is designed for firmware where each feature owns a small translation table, lookup must stay allocation-free, and runtime registry storage should prefer PSRAM when available.

[![CI](https://github.com/ZekStack/lingo/actions/workflows/ci.yml/badge.svg)](https://github.com/ZekStack/lingo/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/ZekStack/lingo?sort=semver)](https://github.com/ZekStack/lingo/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE.md)

## Why use Lingo?

* **Feature-owned tables** - register many small translation domains per language instead of one global dictionary.
* **Strongly typed keys** - each enum type is an isolated translation domain, so identical numeric values cannot collide across features.
* **PSRAM-first registry** - `preferPsram` defaults to `true` and falls back to normal ESP32 heap when needed.
* **Allocation-free lookup** - translation tables and strings are referenced directly; `get()` and `find()` do not allocate.
* **No RTTI** - enum domains use unique static type tokens without `typeid` or `std::type_index`.
* **Safe C strings** - `get()` always returns a valid `const char *`, suitable for logging and embedded display APIs.
* **Explicit fallback** - requested-language lookup can fall back to the configured default language within the same feature domain.

## Install

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

### Arduino IDE

Lingo is not published to Arduino Library Manager yet.

Install it by downloading the repository ZIP or cloning it into:

```txt
Arduino/libraries/Lingo
```

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
	config.preferPsram = true;

	if (!lingo.init(config)) {
		return;
	}

	lingo.addTable(Language::Hu, SOFTWARE_HU);
	lingo.addTable(Language::En, SOFTWARE_EN);

	const char *current = lingo.get(SoftwareKey::Install);
	const char *english = lingo.get(SoftwareKey::Install, Language::En);

	lingo.setDefaultLanguage(Language::En);
}
```

A second feature can define its own enum starting at the same numeric values without collisions:

```cpp
enum class TimeKey : uint16_t {
	Save = 0,
	Timezone = 1,
};

lingo.addTable(Language::Hu, TIME_HU);
lingo.addTable(Language::En, TIME_EN);
```

## Important notes

> [!IMPORTANT]
> Registered tables and their translation strings are not copied. Keep them alive until `lingo.end()`. `constexpr` or static tables containing string literals are the intended pattern.

* Exactly one table may be registered for a `{language, key enum type}` pair.
* `get()` never returns `nullptr`; `find()` is the strict nullable lookup API.
* `get(key, language)` falls back to the default language only within the same enum domain.
* `init()` allocates a bounded registry once. With `preferPsram = true`, ESP32 PSRAM is attempted first and normal heap is the fallback.
* After startup registration, concurrent `get()`/`find()` calls are supported. Do not mutate the table registry concurrently with lookups.
* Translation bytes are passed through unchanged, including UTF-8 text.

## Examples

| Example | Description |
| --- | --- |
| `Basic` | Initialize Lingo, register HU/EN tables, and switch the default language. |
| `FeatureTables` | Register multiple feature domains per language with overlapping numeric key values. |
| `Fallback` | Demonstrate strict lookup and same-domain default-language fallback. |

Start with:

```txt
examples/Basic
```

## Documentation

| Document | Description |
| --- | --- |
| [`docs/getting-started.md`](docs/getting-started.md) | First integration and table registration. |
| [`docs/api.md`](docs/api.md) | Public types and methods. |
| [`docs/configuration.md`](docs/configuration.md) | Registry sizing, fallback string, and PSRAM preference. |
| [`docs/tables-and-domains.md`](docs/tables-and-domains.md) | Feature-domain identity and registration rules. |
| [`docs/fallback.md`](docs/fallback.md) | Exact `get()` and `find()` resolution behavior. |
| [`docs/memory.md`](docs/memory.md) | Static translations, bounded allocation, and PSRAM behavior. |
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

lingo.setDefaultLanguage(Language::En);
```

## Compatibility

| Item | Support |
| --- | --- |
| Framework | Arduino ESP32 |
| Platform | `espressif32` |
| Language | C++20 |
| Dependencies | none |
| RTTI | not required |
| Exceptions | not used for public error handling |
| PSRAM | preferred for registry allocation by default |
| Runtime lookup allocation | none |
| Status | initial `0.1.0` development |

## License

MIT - see [`LICENSE.md`](LICENSE.md).

## ZekStack

Part of the ZekStack ESP32 library stack.

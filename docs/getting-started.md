# Getting started

Lingo maps strongly typed enum keys to static translation strings and uses Strata for its bounded registry allocation.

## 1. Define languages

```cpp
enum class Language : uint16_t {
	Hu,
	En,
};
```

## 2. Define a feature domain

Each key enum is an independent translation domain. This lets every application feature own a small table without sharing one global key enum.

```cpp
enum class SoftwareKey : uint16_t {
	UpdateAvailable,
	Install,
};
```

## 3. Define tables

```cpp
constexpr LingoTable SOFTWARE_HU{
    LingoEntry(SoftwareKey::UpdateAvailable, "Frissítés érhető el"),
    LingoEntry(SoftwareKey::Install, "Telepítés"),
};

constexpr LingoTable SOFTWARE_EN{
    LingoEntry(SoftwareKey::UpdateAvailable, "Update available"),
    LingoEntry(SoftwareKey::Install, "Install"),
};
```

Keep registered tables and their strings alive for the entire time they are registered. `constexpr` or static tables backed by string literals are the intended pattern.

## 4. Initialize and register

```cpp
Lingo lingo;

LingoConfig config;
config.defaultLanguage = Language::Hu;
config.memory.allocation = Strata::Placement::PreferExternal;

if (!lingo.init(config)) {
	return;
}

lingo.addTable(Language::Hu, SOFTWARE_HU);
lingo.addTable(Language::En, SOFTWARE_EN);
```

`PreferExternal` is already Lingo's default and preserves the v0.1.x PSRAM-first behavior. Set `Internal` when the registry must stay in internal RAM, or `RequireExternal` when initialization must fail instead of falling back.

## 5. Translate

```cpp
const char *current = lingo.get(SoftwareKey::Install);
const char *hungarian = lingo.get(SoftwareKey::Install, Language::Hu);
const char *english = lingo.get(SoftwareKey::Install, Language::En);
```

`get()` never returns `nullptr`. Use `find()` when the caller needs a strict nullable lookup.

## 6. Inspect registry placement

```cpp
const LingoDiag diag = lingo.getDiagnostics();

// Requested policy.
Strata::Placement requested = diag.registryPlacement;

// Actual memory region, when the platform can identify it.
Strata::Region actual = diag.registryRegion;
```

Requested placement and observed region are intentionally separate because `PreferExternal` may fall back to internal memory.

# Getting started

Lingo maps strongly typed enum keys to static translation strings.

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
config.preferPsram = true;

if (!lingo.init(config)) {
	return;
}

lingo.addTable(Language::Hu, SOFTWARE_HU);
lingo.addTable(Language::En, SOFTWARE_EN);
```

## 5. Translate

```cpp
const char *current = lingo.get(SoftwareKey::Install);
const char *hungarian = lingo.get(SoftwareKey::Install, Language::Hu);
const char *english = lingo.get(SoftwareKey::Install, Language::En);
```

`get()` never returns `nullptr`. Use `find()` when the caller needs a strict nullable lookup.

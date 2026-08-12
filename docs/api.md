# API

## `LingoConfig`

```cpp
struct LingoConfig {
	LingoLanguage defaultLanguage{};
	size_t maxTables = 64;
	const char *missingTranslation = "";
	bool preferPsram = true;
};
```

`preferPsram` controls the registry allocation. On ESP32, Lingo first attempts PSRAM and falls back to normal 8-bit capable heap. On host builds it uses normal heap allocation.

## `LingoResult`

Control operations return `LingoResult`.

```cpp
LingoResult result = lingo.init(config);
if (!result) {
	// result.status and result.message describe the failure.
}
```

## Lifecycle

```cpp
LingoResult init(const LingoConfig &config = LingoConfig());
LingoResult end();
bool initialized() const;
```

## Tables

```cpp
template<typename TLanguage, typename TKey, size_t N>
LingoResult addTable(
	TLanguage language,
	const LingoTable<TKey, N> &table
);
```

Only one table may be registered for a given `{language, key enum type}` pair.

## Language selection

```cpp
template<typename TLanguage>
LingoResult setDefaultLanguage(TLanguage language);
```

The selected language must already have at least one registered table.

## Lookup

```cpp
template<typename TKey>
const char *get(TKey key) const;

template<typename TKey, typename TLanguage>
const char *get(TKey key, TLanguage language) const;

template<typename TKey>
const char *find(TKey key) const;

template<typename TKey, typename TLanguage>
const char *find(TKey key, TLanguage language) const;
```

`find()` is strict and returns `nullptr` when no translation exists.

`get(key)` looks only in the selected default language and returns `missingTranslation` when absent.

`get(key, language)` first checks the requested language, then the default language, then returns `missingTranslation`.

## Registry information

```cpp
size_t tableCount() const;
size_t tableCapacity() const;
bool preferPsram() const;
```

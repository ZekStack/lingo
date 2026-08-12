# Configuration

## `defaultLanguage`

The default language is stored during `init()`. It does not need to have a table registered yet, which allows normal boot-time initialization before features register their translations.

```cpp
config.defaultLanguage = Language::Hu;
```

Use `setDefaultLanguage()` after registration to change the selected language at runtime.

## `maxTables`

`maxTables` bounds the number of registered `{language, feature-domain}` pairs.

```cpp
config.maxTables = 64;
```

Lingo allocates the registry once during `init()`. Table registration and translation lookup do not allocate.

For two languages, 64 slots allow 32 feature domains.

## `missingTranslation`

`get()` always returns a valid C string. Configure the value used when neither the requested nor fallback table contains the key.

```cpp
config.missingTranslation = "";
```

The pointer must remain valid while Lingo is initialized.

## `preferPsram`

PSRAM is preferred by default:

```cpp
config.preferPsram = true;
```

On ESP32, Lingo attempts `MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT` first when SPIRAM is available. If that allocation fails or PSRAM is unavailable, it falls back to normal `MALLOC_CAP_8BIT` heap.

Set it to `false` to allocate the registry from normal 8-bit capable heap directly.

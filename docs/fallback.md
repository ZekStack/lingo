# Fallback behavior

Lingo provides strict lookup through `find()` and safe fallback lookup through `get()`.

## Strict lookup

```cpp
const char *value = lingo.find(SoftwareKey::Install, Language::Hu);
```

Only the requested Hungarian `SoftwareKey` table is searched. Missing table or key returns `nullptr`.

Without an explicit language, `find(key)` searches only the currently selected default language.

## Safe lookup

```cpp
const char *value = lingo.get(SoftwareKey::Install, Language::Hu);
```

Resolution order:

1. requested language + same key domain,
2. default language + same key domain,
3. `config.missingTranslation`.

Fallback never crosses feature domains.

`get()` always returns a valid C string and can be passed directly to C-style APIs such as loggers or display renderers.

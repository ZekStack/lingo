# Memory

Lingo is designed around static translation data and one bounded runtime registry.

## Static translation data

`LingoTable` stores only numeric keys and pointers to caller-owned translation strings.

```cpp
constexpr LingoTable EN{
    LingoEntry(Key::Save, "Save"),
};
```

The string literal and table live in static storage. Lookup does not allocate or copy the translation.

## Registry allocation

`init()` allocates `maxTables` registry slots once.

With `preferPsram = true`, ESP32 allocation follows the same PSRAM-first policy used across ZekStack libraries:

1. use `MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT` when SPIRAM exists,
2. fall back to `MALLOC_CAP_8BIT`,
3. return `LingoStatus::AllocationFailed` if neither allocation succeeds.

Host builds use `malloc`.

## Runtime allocation behavior

After successful `init()`:

- `addTable()` does not allocate,
- `find()` does not allocate,
- `get()` does not allocate,
- `setDefaultLanguage()` does not allocate.

Only `init()` allocates registry storage and `end()` releases it.

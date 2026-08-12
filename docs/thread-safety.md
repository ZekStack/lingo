# Thread safety

Lingo is intended to be configured during application startup and read concurrently afterward.

## Runtime reads

After all feature tables have been registered, multiple tasks may call `find()` and `get()` concurrently. Translation tables are immutable from Lingo's point of view.

The default language is stored atomically, so `setDefaultLanguage()` can change the selected language while readers are performing lookups.

## Lifecycle operations

Do not call these concurrently with translation lookups:

- `init()`
- `addTable()`
- `end()`

The intended lifecycle is:

```text
boot
  -> init
  -> register feature tables
  -> normal runtime lookups
  -> end
```

No FreeRTOS task or mutex is owned by Lingo.

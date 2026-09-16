# Configuration

## `memory`

Lingo uses the shared ZekStack `Strata::MemoryPolicy` configuration contract.

```cpp
LingoConfig config;
config.memory.allocation = Strata::Placement::PreferExternal;
```

Lingo owns one bounded dynamic allocation: the translation-table registry created by `init()`. `memory.allocation` controls that registry.

The default is `Strata::Placement::PreferExternal`, which preserves the v0.1.x behavior: prefer external memory and allow safe fallback to internal memory.

Available placements are:

- `Strata::Placement::Default` - use the Strata backend's default allocation policy.
- `Strata::Placement::Internal` - allocate the registry from internal memory.
- `Strata::Placement::PreferExternal` - prefer external memory and allow fallback.
- `Strata::Placement::RequireExternal` - require external memory; `init()` returns `LingoStatus::AllocationFailed` when it cannot be satisfied.

Lingo does not own any tasks, so `memory.taskStack` currently controls no Lingo allocation. It remains part of the config for ecosystem consistency and the full memory policy is validated by `init()`.

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

Lingo allocates the complete registry once during `init()`. Table registration and translation lookup do not allocate.

For two languages, 64 slots allow 32 feature domains.

## `missingTranslation`

`get()` always returns a valid C string. Configure the value used when neither the requested nor fallback table contains the key.

```cpp
config.missingTranslation = "";
```

The pointer must remain valid while Lingo is initialized.

## Migrating from v0.1.x

Replace the old PSRAM boolean with Strata placement:

```cpp
// v0.1.x
config.preferPsram = true;

// v0.2.0
config.memory.allocation = Strata::Placement::PreferExternal;
```

and:

```cpp
// v0.1.x
config.preferPsram = false;

// v0.2.0
config.memory.allocation = Strata::Placement::Internal;
```

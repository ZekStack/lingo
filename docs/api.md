# API

## `LingoConfig`

```cpp
struct LingoConfig {
	Strata::MemoryPolicy memory{
	    .allocation = Strata::Placement::PreferExternal,
	    .taskStack = Strata::Placement::Internal,
	};
	LingoLanguage defaultLanguage{};
	size_t maxTables = 64;
	const char *missingTranslation = "";
};
```

`memory.allocation` controls the one bounded registry allocation owned by Lingo. The default is `Strata::Placement::PreferExternal`, preserving the v0.1.x PSRAM-first behavior with internal-memory fallback.

Lingo does not create tasks, so `memory.taskStack` currently has no runtime storage to control. It remains part of the config to use the same `Strata::MemoryPolicy` contract as other ZekStack libraries. The complete policy is validated by `init()`.

Useful allocation placements are:

- `Strata::Placement::Default` - use Strata's backend-default allocation policy.
- `Strata::Placement::Internal` - require internal memory.
- `Strata::Placement::PreferExternal` - prefer external memory and allow safe fallback.
- `Strata::Placement::RequireExternal` - require external memory; initialization fails if unavailable.

## `LingoResult`

Control operations return `LingoResult`.

```cpp
LingoResult result = lingo.init(config);
if (!result) {
	// result.status and result.message describe the failure.
}
```

Invalid memory policies return `LingoStatus::InvalidConfig`. An otherwise valid policy that cannot satisfy the registry allocation returns `LingoStatus::AllocationFailed`.

## Lifecycle

```cpp
LingoResult init(const LingoConfig &config = LingoConfig());
LingoResult end();
bool initialized() const;
```

`init()` performs Lingo's only owned dynamic allocation. `end()` releases it through Strata. Re-initialization after `end()` is supported.

## Tables

```cpp
template<typename TLanguage, typename TKey, size_t N>
LingoResult addTable(
	TLanguage language,
	const LingoTable<TKey, N> &table
);
```

Only one table may be registered for a given `{language, key enum type}` pair. Registration does not allocate.

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

Lookup remains allocation-free.

## Registry information and diagnostics

```cpp
size_t tableCount() const;
size_t tableCapacity() const;
LingoDiag getDiagnostics() const;
```

`LingoDiag` keeps requested policy separate from observed memory location:

```cpp
struct LingoDiag {
	size_t tableCount;
	size_t tableCapacity;
	Strata::Placement registryPlacement;
	Strata::Region registryRegion;
};
```

For example, `registryPlacement == Strata::Placement::PreferExternal` may legitimately result in `registryRegion == Strata::Region::Internal` when external memory is unavailable and fallback is allowed.

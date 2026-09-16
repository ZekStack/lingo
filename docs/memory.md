# Memory

Lingo is designed around static translation data and one bounded runtime registry. All Lingo-owned dynamic memory routes through Strata.

## Static translation data

`LingoTable` stores only numeric keys and pointers to caller-owned translation strings.

```cpp
constexpr LingoTable EN{
    LingoEntry(Key::Save, "Save"),
};
```

The string literal and table live in static storage. Lingo does not copy them.

## Registry allocation

`init()` allocates `maxTables` registry slots once through Strata.

```cpp
LingoConfig config;
config.memory.allocation = Strata::Placement::PreferExternal;
```

The default is `PreferExternal`, preserving Lingo v0.1.x behavior: external memory is preferred and internal memory is a valid fallback.

Use `Internal` to keep the registry in internal RAM, `Default` to use the backend default, or `RequireExternal` when internal fallback is not acceptable.

If the requested placement cannot be satisfied, `init()` returns `LingoStatus::AllocationFailed` and Lingo remains uninitialized.

## Requested placement and observed region

`getDiagnostics()` reports both the requested placement and the actual observed region:

```cpp
const LingoDiag diag = lingo.getDiagnostics();

Strata::Placement requested = diag.registryPlacement;
Strata::Region actual = diag.registryRegion;
```

These are intentionally separate. For example, `PreferExternal` may resolve to `Region::Internal` when external allocation fails and fallback is allowed.

## Runtime allocation behavior

After successful `init()`:

- `addTable()` does not allocate,
- `find()` does not allocate,
- `get()` does not allocate,
- `setDefaultLanguage()` does not allocate.

Only `init()` allocates registry storage and `end()` releases it through Strata.

## Ownership boundary

Lingo production sources must not call ESP-IDF heap capability APIs, `ps_malloc`, or raw C heap allocation directly. CI enforces this so future Lingo-owned allocations continue to use Strata.

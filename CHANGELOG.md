# Changelog

All notable changes to Lingo are documented in this file.

## 0.2.0

- Add Strata v0.1.2 as Lingo's memory ownership dependency.
- Replace `LingoConfig::preferPsram` with the shared `Strata::MemoryPolicy` configuration contract.
- Preserve the v0.1.x default registry behavior with `memory.allocation = Strata::Placement::PreferExternal`.
- Route the bounded translation-table registry through Strata typed allocation and `Strata::free()`.
- Add explicit `Default`, `Internal`, `PreferExternal`, and `RequireExternal` placement support.
- Add `LingoDiag` with requested registry placement and observed registry region.
- Validate Strata memory policies during `init()` and continue returning `LingoStatus::AllocationFailed` for unsatisfied allocations.
- Remove the bespoke ESP-IDF heap allocator, PSRAM test stubs, and direct `heap_caps_*` dependency.
- Add CI source contracts preventing direct heap allocation paths from returning to Lingo-owned code.
- Add host memory-policy coverage and Arduino CLI builds alongside the existing PlatformIO ESP32 matrix.

### Migration from 0.1.x

```cpp
// 0.1.x
config.preferPsram = false;

// 0.2.0
config.memory.allocation = Strata::Placement::Internal;
```

The former default `preferPsram = true` maps to `Strata::Placement::PreferExternal` and remains the Lingo default in v0.2.0.

## 0.1.0

- Add strongly typed enum-key translation domains without RTTI.
- Add multiple feature-specific translation tables per language.
- Add bounded registry allocation with PSRAM-first behavior and normal-heap fallback.
- Add allocation-free `get()` and `find()` lookup paths.
- Add same-domain default-language fallback and configurable missing translation text.
- Add host behavior tests, ESP32 example builds, documentation, and release workflows.

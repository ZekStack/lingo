# Troubleshooting

## `InvalidConfig`

`init()` rejects zero `maxTables`, a null `missingTranslation`, an overflowing registry size, or an invalid `Strata::MemoryPolicy`. Use one of the defined `Strata::Placement` values for both policy fields.

## `AllocationFailed`

Strata could not allocate the bounded registry with the requested policy.

If `config.memory.allocation == Strata::Placement::RequireExternal`, external memory is mandatory and Lingo does not fall back to internal RAM. Use `PreferExternal` when internal fallback is acceptable or `Internal` when the registry should stay in internal memory.

## `TableAlreadyRegistered`

Only one table may be registered for each `{language, key enum type}` pair. Merge that feature's translations into one table for the language.

## `DuplicateKey`

A table contains the same numeric enum value more than once. Each key value must be unique inside one enum domain.

## `TableCapacityExceeded`

Increase `config.maxTables`. Remember that each language/domain pair consumes one registry slot.

## Missing translation

Use `find()` to distinguish a genuinely missing value from the configured `missingTranslation` fallback.

## Invalid translation

A `LingoEntry` contained a `nullptr` translation. Use an empty string when an intentionally empty translation is required.

## Dangling strings or tables

Lingo stores references to caller-owned static translation data. Do not register tables or string buffers whose lifetime ends before `lingo.end()`.

# Troubleshooting

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

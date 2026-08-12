# Tables and domains

Lingo is designed for feature-owned translation tables.

## Enum type is the domain

These two keys both have numeric value `0`:

```cpp
enum class SoftwareKey : uint16_t {
	Save = 0,
};

enum class TimeKey : uint16_t {
	Save = 0,
};
```

They do not collide. Lingo treats the enum type itself as the translation domain, so the internal identity is effectively:

```text
language + enum type + enum value
```

No RTTI or `std::type_index` is required. Each enum type receives a unique static compile-time domain token.

## Multiple tables per language

A language can contain many feature-specific tables:

```cpp
lingo.addTable(Language::Hu, COMMON_HU);
lingo.addTable(Language::Hu, SOFTWARE_HU);
lingo.addTable(Language::Hu, TIME_HU);
```

The same domains can then be registered independently for English:

```cpp
lingo.addTable(Language::En, COMMON_EN);
lingo.addTable(Language::En, SOFTWARE_EN);
lingo.addTable(Language::En, TIME_EN);
```

## Registration invariant

Exactly one table may exist for a `{language, enum type}` pair.

Registering a second Hungarian `SoftwareKey` table returns `LingoStatus::TableAlreadyRegistered`.

## Table lifetime

Lingo does not copy translation tables or strings. Registered table storage and translation string pointers must remain valid until `end()`.

The intended firmware pattern is `constexpr` or static tables containing string literals.

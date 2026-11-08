# Versioning and compatibility

**Status:** Draft

Moonlight uses SemVer for module and public API versions. Major changes may
break consumers; minor versions add backwards-compatible capabilities; patches
fix behavior without changing contracts. ABI major versions are independent
and breaking. Schema versions use explicit migration functions, not SemVer
inference.

Compatibility is checked against ranges in manifests and ABI tables:
`>=1.2 <2.0`. A provider must not claim compatibility without passing its
contract fixtures. Deprecations include replacement, first release and removal
release; at least one supported release provides a warning.

## Release lifecycle

`Development → Compatible → Deprecated → Removed`. Generated catalogs and
run manifests record exact module, ABI, schema, compiler and asset versions.
Lockfiles pin dependency resolution; reproducible builds record toolchain
digests.

Failures are explicit: `UnsupportedVersion`, `MigrationRequired`,
`IncompatibleAbi`, `MissingCapability`. Do not silently downgrade semantics.
Test current/previous-version readers and writers, ABI minor negotiation,
migrations, lockfile resolution and deterministic catalog output. See
[data formats](data-formats.md), [module system](module-system.md) and
[ADR 0003](adr/0003-abi-and-plugin-loading.md).

## Compatibility decision table

| Change | Reader | Writer | Required action |
| --- | --- | --- | --- |
| Add optional field/capability | Old | New | Omit or default for old consumers |
| Change required field semantics | Old | New | New schema/API major and migration |
| Add ABI table suffix | Old | New | Guard with `struct_size` and minor negotiation |
| Remove/deprecate capability | New | Old | Keep compatibility window or reject explicitly |
| Change canonical/hash meaning | Any | Any | New schema/version; never reinterpret bytes |

The precondition for claiming compatibility is passing the relevant golden,
migration, and ABI fixtures. The postcondition is identical documented
semantics, not merely successful parsing.

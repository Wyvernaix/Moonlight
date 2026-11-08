# Adding a component

Components are data contracts, not arbitrary bags of engine state. Before implementation, write down the stable identifier, semantic version, units, defaults, required/optional fields, serialization representation, and compatibility policy. Search existing component definitions and tests so identifiers are not duplicated.

## Implementation sequence

1. Define the immutable public schema and validation rules.
2. Add registration in the owning module, with a deterministic identifier.
3. Implement serialization/deserialization with explicit version handling.
4. Add round-trip, missing-field, invalid-value, and old-version migration tests.
5. Export a read-only snapshot for systems that consume the component.
6. Run module validation and the complete build.

```sh
python scripts/moonlight.py validate
python python/build.py --configuration debug
```

## Ownership and lifetime

Atlas owns component storage. Systems may request extraction, but must consume immutable snapshots and must not mutate Atlas memory or retain references past the documented snapshot lifetime. The creator owns handles/resources; destruction is idempotent and occurs on the owning thread unless the component explicitly documents otherwise. Serialization must not invoke user callbacks, access render/audio devices, or perform unbounded allocation.

Validate at the boundary: reject wrong units, NaN/infinite values, invalid enum values, and incompatible versions with an actionable error. Preserve unknown fields when forward compatibility is promised; otherwise fail closed and document the policy. Never silently reinterpret a field or change defaults without a version/migration note.

## Contract tests

A component is complete only when tests prove deterministic defaults, canonical serialization, round-trip equality, invalid input rejection, migration behavior, and snapshot lifetime. Test on all CI platforms where binary layout, path handling, or endianness could matter. Include the exact command and result in the change description; generated fixtures belong in ignored build output unless they are intentional small test assets.

# Atlas: scene graph and ECS

**Status:** Design target
**Sources:** `design-specification.md`

Atlas is Moonlight's authoritative scene and world-management module. It
provides a familiar GameObject and Hierarchy workflow for the C# Workbench while
using an ECS internally for efficient queries, simulation, and rendering.

> Atlas owns what exists in the world and where it is. Other modules own what
> those things do.

Spectral receives geometry, materials, lights, and camera data. Orcha receives
sources, listeners, geometry, and acoustic materials. Flywheel receives
animation targets and skeletons. Physics receives bodies, colliders, and
constraints. All consumers use the same persistent identities and transform
conventions.

## Responsibilities

| Subsystem | Responsibility |
| --- | --- |
| ECS | Entity identities, component storage, queries, and scheduling integration |
| Scene graph | Parent-child relationships and hierarchy operations |
| Transform | Local/world transforms, conversion, dirty propagation |
| Authoring | Components, tags, layers, prefabs, scene loading and saving |
| Extraction | Immutable snapshots for rendering, audio, animation, and physics |

Atlas stores component data but does not implement renderer, audio, or physics
behavior. External modules can register component schemas, storage policies,
serialization functions, validation, migration, and Workbench metadata.

## GameObject identity

A GameObject is an editor-facing facade over an ECS entity. It has:

- One persistent `EntityGuid`.
- One runtime `EntityHandle` with an index and generation.
- Exactly one required `Transform`.
- Zero or more extension components.
- Active state, name, layer, tags, scene membership, and hierarchy links.

Do not use one integer for every identity concern:

| Identity | Use |
| --- | --- |
| `EntityGuid` | Persistence, prefab instances, references, scene revisions |
| `EntityHandle` | Fast runtime lookup with stale-reference protection |
| `ComponentTypeId` | Stable component schema identity |

Generation counters ensure a stale handle cannot accidentally reference a new
entity after slot reuse.

## ECS storage

Use hybrid storage:

- Dense archetype tables for hot component combinations.
- Sparse pools for rare, managed, irregular, or extension components.
- Deferred command buffers for structural changes during iteration.

Components must not be persisted by copying their C++ memory layout. Scene
serialization uses reflected fields, schema versions, and migration functions.
If a module is missing, Atlas preserves the raw component payload as an
unresolved record instead of deleting it.

## Hierarchy and transforms

Atlas uses a strict parent-child tree. Each entity has at most one parent and
cycles are rejected. The baseline convention is right-handed, Y-up, metres, and
radians unless a project-level decision changes it.

```text
world = parent_world * local
root_world = local
```

Required behavior:

- Parent movement updates descendant world transforms.
- Child movement does not mutate its parent.
- Reparenting explicitly chooses local-space or world-space preservation.
- Invalid parents and hierarchy cycles are rejected.
- Dirty propagation is incremental.
- Nonuniform scale and rotation preserve derived matrix information.

Keep hierarchy traversal in a dedicated structure rather than forcing the
Workbench to scan ECS storage.

## Scene, world, and prefab model

A scene asset is serialized authoring data. A World is runtime state and may
contain multiple loaded scenes for streaming or additive loading. Entities have
one defined scene owner.

Proposed project assets:

```text
Scenes/*.moonscene
Prefabs/**/*.moonprefab
Assets/{Meshes,Materials,Textures,Audio,Animation}/
Project/{project.moonproject,asset-catalog.json}
```

Prefabs retain source identity and instance overrides. Supported operations are
instantiate, nest, override, apply, revert, migrate, and validate. Asset
references use stable GUIDs rather than absolute paths or memory addresses.

## Snapshot contract

Atlas commits immutable, revisioned snapshots. A snapshot identifies the world,
scene, frame, and transform revisions it represents. Consumers reject or
refresh stale snapshots according to their frame policy.

Render extraction includes geometry, transforms, materials, visibility, camera,
lights, skinning, environment, and capture settings. Acoustic extraction
includes sources, listeners, acoustic geometry, materials, portals, zones, and
dynamic/static status. Physics and animation have explicit writeback phases;
multiple systems must not write the same transform without scheduler ownership.

## Measurement and reproducibility boundary

An extraction snapshot is the reproducibility boundary for downstream science.
It carries `world_revision`, `scene_revision`, `frame_index`, `time_s`, units,
coordinate convention, asset revision IDs, and a content hash. Consumers must
record that identity rather than serializing mutable ECS pointers. A snapshot
is immutable after publication; late streaming assets produce a new revision.

```json
{
  "schema": "atlas.snapshot/1",
  "world_revision": 42,
  "scene_revision": "sha256:…",
  "frame_index": 120,
  "time_s": 2.0,
  "convention": {"handedness": "right", "up": "Y", "length": "m"},
  "entities": [{"guid": "…", "transform": {"matrix": [/* 16 */]}}]
}
```

Validation rejects cycles, NaN transforms, non-finite bounds, and unresolved
asset references. Spectral and Orcha can therefore replay the same geometry
and provenance without depending on editor state.

## Proof obligations and failure semantics

Atlas is the authority for dimensions and identity at module boundaries.
Validation must check that every transform translation and bound is in metres,
angles are in radians, matrices are finite, and parent-child multiplication
preserves the declared handedness. A scale of zero is legal only when the
consumer explicitly supports singular transforms; otherwise extraction fails
with `ATLAS_SINGULAR_TRANSFORM` rather than producing an inverse or normal
matrix with undefined meaning.

The hierarchy invariant is that each entity has at most one parent, no entity
is its own ancestor, and every published world transform has exactly one path
from its scene root. Reparent tests compare world-space preservation within
the configured tolerance and verify that local-space reparenting changes no
ancestor. Snapshot extraction is atomic: a failed validation publishes no
partial snapshot. Stale handles, missing assets, cycle attempts, non-finite
values, and revision mismatches are typed failures with the offending GUID
and revision; consumers must not silently substitute identity transforms.

CPU and GPU extraction paths consume the same canonical snapshot fixture.
Deterministic tests sort GUIDs and component fields before hashing, pin the
frame index and locale, and compare matrices with an absolute/relative
tolerance plus a hash of the canonicalized payload. The test suite includes
deep hierarchies, repeated reparenting, slot reuse/generation rollover,
negative and nonuniform scale, empty scenes, maximum coordinate magnitudes,
duplicate GUIDs, and adversarial cycle graphs. An extraction result records
the snapshot hash, precision, backend, and validation status.

Cross-module replay and adversarial cases follow the
[cross-module validation protocol](../../research/cross-module-validation.md)
and [reference scene suite](../../research/adversarial-reference-scenes.md).

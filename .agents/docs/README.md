# Moonlight documentation workspace

This directory is the agent-maintained engineering documentation workspace for
Moonlight. It mirrors the planned public documentation tree and records
architecture, implementation contracts, research assumptions, and executable
development workflows.

## Documentation status

Every statement must be one of these:

- **Implemented** — verified against source code, tests, generated output, or a
  reproducible command.
- **Design target** — accepted direction that is not fully implemented.
- **Draft** — incomplete and awaiting a decision, experiment, or evidence.
- **Deprecated** — retained only to explain migration or compatibility.

Do not silently promote a design target to implemented behaviour. Link the
implementation, test, benchmark, schema, or issue that supports a status
change.

## Documentation blocks

| Block | Contents |
| --- | --- |
| `architecture/` | Runtime, ABI, graph, ownership, platform, data, observability, and ADR documents |
| `development/` | Build, debugging, module authoring, profiling, release, and validation workflows |
| `modules/atlas/` | Scene graph, ECS, identity, transforms, serialization, and snapshots |
| `modules/spectral/` | Optical physics, reference rendering, voxel GI, and GPU backends |
| `modules/orcha/` | Acoustic propagation, spatialization, DSP, and validation |
| `research/` | Reproducibility rules, references, datasets, and experiment documentation |

## Authority and change order

The source of truth is layered:

1. Versioned schemas and public headers define machine-readable and ABI
   contracts.
2. Tests, benchmarks, and generated reports define observed behaviour.
3. Architecture decision records define cross-cutting choices and rationale.
4. Module specifications define algorithms, data ownership, and validation.
5. Overviews and tutorials explain how to use the contracts.

When layers disagree, update the authoritative layer first and then update
derived documentation. A documentation change that alters runtime behaviour
requires code and tests in the same change set.

## Reading paths

| Need | Start here |
| --- | --- |
| Understand the repository | `architecture/overview.md`, `architecture/repository-map.md` |
| Add a native module | `development/adding-a-module.md`, `architecture/module-system.md` |
| Add scene data | `modules/atlas/overview.md`, `development/adding-a-component.md` |
| Add rendering work | `modules/spectral/overview.md`, `modules/spectral/rendering-backends.md` |
| Add acoustic work | `modules/orcha/overview.md`, `development/adding-an-acoustic-solver.md` |
| Reproduce a result | `development/build-and-test.md`, `research/README.md` |
| Diagnose a failure | `development/troubleshooting.md`, `architecture/observability.md` |

## Technical writing requirements

Algorithm documentation must state inputs, outputs, units, coordinate systems,
precision, ownership, threading, failure behaviour, and validation strategy.
Numerical documentation must state assumptions, convergence criteria, error
budgets, and model validity. API examples are illustrative unless marked as
compiled and tested.

Use SI units internally unless a document explicitly defines a domain-specific
unit. Use `snake_case` for serialized keys, stable IDs for persistent objects,
and version every persisted format. Never include credentials, private
datasets, or unlicensed binary assets in this tree.

## External references

Research links are supporting evidence, not runtime dependencies. Pin a paper,
standard, or specification by title, publisher, and access date in the relevant
research document. Do not copy text or figures; summarize the result and explain
how it affects Moonlight. The current reference index is
`research/references.md`.

## Generated documentation

Generated documentation belongs in `.agents/docs/generated/` and must not be
edited by hand. Generators must record their input revision, generator version,
source hashes, and generation command. CI should regenerate into a temporary
directory and compare it with committed artifacts when those artifacts are
checked in.

## Detailed specifications

The former `.notion` specifications were migrated into these maintained
documents:

- `architecture/graph-system-specification.md`
- `modules/atlas/design-specification.md`
- `modules/orcha/design-specification.md`
- `modules/spectral/physics/optical-foundation.md`
- `modules/spectral/physics/rendering-and-voxel-gi.md`

The shorter overview files provide navigation and implementation summaries.
The detailed specifications retain equations, APIs, roadmaps, validation plans,
and assumptions.

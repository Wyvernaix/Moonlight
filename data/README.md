# Moonlight data tree

`data/` contains versioned, non-source inputs shared by the runtime, modules,
applications, and tooling. Schemas define contracts, configuration defines
defaults and profiles, registries define declarative discovery metadata, and
templates define generated project/module starting points.

Generated catalogs and compiled assets do not belong here. They are produced
under `build/` or `out/` and record their source hashes. User overrides belong
outside the repository defaults.

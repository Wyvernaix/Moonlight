# Research documentation

Research notes are organized by domain and must be reproducible. Start with
the [workflow](workflow.md), then create an
[experiment manifest](experiment-manifest.md), select a solver using
[solver selection](solver-selection.md), and validate with the
[validation protocol](validation-protocol.md). Material imports follow
[material provenance](material-provenance.md).

Every run records source commit, build and renderer ID, scene/material/data
hashes, conditions, random seeds, solver parameters, numerical settings,
output metadata, uncertainty, and licenses/provenance. Large datasets and
rendered results belong in controlled storage or CI artifacts; commit manifests,
checksums, protocols, analysis scripts, and regenerable figures.

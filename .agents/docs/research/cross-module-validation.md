# Cross-module validation

**Status:** Normative protocol  
**Scope:** Atlas snapshots consumed by Spectral and Orcha

This protocol proves that a result is based on the same world, units, and
revision across modules. It complements the domain-specific
[validation protocol](validation-protocol.md); it does not replace a solver's
analytic tests.

## Canonical fixture and invariants

Start with one immutable Atlas snapshot containing a root, a translated and
rotated child, a nonuniformly scaled mesh, one light, one sound source, one
listener/camera, and a material with independent optical and acoustic data.
Serialize transforms in metres/radians and sort all records by `EntityGuid`.

The harness checks:

- world transforms satisfy `world = parent_world * local` and preserve the
  selected handedness;
- geometry bounds, ray lengths, acoustic path lengths, and voxel extents have
  consistent metre dimensions;
- a scene revision and snapshot hash are identical in both result manifests;
- a source/listener delay is `path_length / speed`, with seconds and hertz
  conversions explicit;
- optical and acoustic material channels never alias, even when they share an
  asset GUID;
- changing one entity changes only the declared dependent extraction records;
- stale, partial, or mixed-revision snapshots are rejected.

For passive optical and acoustic fixtures, check energy/flux bounds separately.
Do not compare radiance and acoustic pressure as if they were the same
quantity. Reciprocity is tested by swapping source and receiver where the
medium is reciprocal; non-reciprocal media must declare why the test is
inapplicable.

## Procedure and acceptance

1. Validate and hash the Atlas snapshot before extraction.
2. Run CPU reference extraction and both module consumers.
3. Run GPU extraction with the same snapshot, seed, precision policy, and
   measurement definition.
4. Repeat a stochastic case with three pinned seeds and perform one resolution
   refinement or independent implementation comparison.
5. Compare canonical metadata exactly; compare physical observables with
   absolute/relative tolerances and 95% uncertainty intervals.
6. Store manifests, diagnostics, hashes, device details, and failed checks;
   never replace a failed artifact with a retuned tolerance.

Deterministic transforms, IDs, path classifications, and schema hashes must
match bit-for-bit. GPU reductions and Monte Carlo estimates may differ within
the declared physical tolerance, but a result must expose precision, driver,
fast-math/reduction mode, and backend. Any invariant, validity, convergence, or
provenance failure publishes no result and returns a typed error.

## Required edge cases

Exercise empty and maximum-size scenes, deep hierarchies, cycle attempts,
stale generation handles, duplicate GUIDs, degenerate triangles, zero-distance
source/listener, coincident camera, grazing interfaces, total internal
reflection, disconnected rooms, missing material channels, NaN/Inf input,
extreme coordinates, and a revision change during extraction. The expected
outcome for invalid cases is rejection with the entity/revision and rule that
failed, not an identity/default substitution.


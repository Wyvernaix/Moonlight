# Numerical robustness

**Status:** Normative protocol  
**Scope:** Shared numerical policy for Atlas, Spectral, and Orcha

Numerical stability is part of model validity. A plausible image or impulse
response is not evidence that a computation is stable. This protocol records
the checks required before a backend is used as a reference or production
path. It follows the units in
[Spectral conventions](../modules/spectral/physics/units-and-conventions.md)
and the error accounting in
[Spectral error budget](../modules/spectral/physics/error-budget.md).

## Checks

- Validate dimensions at API boundaries; reject unitless values where a unit is
  required and reject non-finite values before they enter a solver.
- Use float64 for reductions, phase accumulation, material preprocessing, and
  reference comparisons. A lower precision path requires a measured error
  budget, bounded dynamic range, and explicit metadata.
- Test cancellation-prone formulas with stable alternatives, scaled variables,
  compensated/pairwise sums, and a zero-safe relative error denominator.
- Record condition estimates, residual norms, iteration counts, stagnation,
  overflow/underflow counters, and denormal/flush-to-zero policy.
- For explicit wave solvers, enforce the declared Courant bound. For iterative
  solvers, require residual and observable convergence rather than iteration
  count alone. For Monte Carlo, report effective samples and confidence
  intervals.

## Convergence and uncertainty

Refine one numerical dimension at a time (samples, mesh/voxel spacing, time
step, ray depth/order, or wavelength spacing), holding all other inputs fixed.
Stop only when successive observables and their uncertainty satisfy the
manifest tolerances; report the convergence slope or refinement table.
Propagate material, calibration, statistical, discretization, and round-off
uncertainties separately. Combine in quadrature only with a documented
independence argument; otherwise retain covariance or use a conservative
bound. A non-converged or underdetermined uncertainty estimate is a failure,
not a zero-error estimate.

## CPU/GPU parity

The CPU reference and GPU backend use the same canonical fixture, serialized
units, seeds, sample sequence where supported, and acceptance observables.
Deterministic integer/transform fixtures require exact equality. Floating and
stochastic paths compare invariant quantities and confidence intervals, while
recording device, driver, compiler, precision, fused-operation, atomic, and
subgroup behavior. A backend must not silently enable fast math, change
reduction order, or clamp an out-of-range value to pass parity.

## Adversarial stability cases

Include zero and subnormal values, huge dynamic range, nearly equal numbers,
grazing angles, total internal reflection, coincident points, degenerate
elements, singular transforms, thin layers, highly absorbing or amplifying
media, nearly singular matrices, empty workloads, maximum recursion/order,
disconnected domains, and interrupted device submissions. Expected failures
carry a stable code and diagnostic context; partial buffers, stale caches, and
NaN-contaminated results are never presented as valid output.


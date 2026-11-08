# Module lifecycle

**Status:** Accepted

## State machine

```text
Discovered -> Validated -> Resolved -> Loading -> Loaded -> Starting -> Started
                                                   |          |
                                                   v          v
                                                 Failed <- Quiescing -> Stopped -> Unloaded
```

`Failed` is terminal for that host attempt. The host validates manifests and
resolves the graph before loading any code. It loads and starts dependencies
first, then registers services, then invokes `start`. Stop is reverse
topological order; unload is permitted only after `stop` and unregister have
completed.

## Hooks

```text
load(context) -> Status
register_services(registry) -> Status
start(context) -> Status
quiesce(deadline) -> Status
stop() -> Status
unregister_services() -> Status
unload() -> Status
```

Hooks are called once, on the host lifecycle thread, and must be idempotent
after a failed partial operation. `start` may schedule work but must publish
readiness only after required resources are available. A timeout forces
quiesce, records diagnostics, and prevents dependent startup.

## Invariants

- No service is visible before registration succeeds.
- No callback executes after unregister returns.
- Optional providers can be absent without changing the state machine.
- Editor and scripting cannot be required for runtime startup.
- Shutdown completes within the host deadline or reports leaked ownership.

Test success, each hook failure, dependency rollback, timeout, repeated stop,
optional absence, and concurrent service lookup. See [observability](observability.md)
for lifecycle events and [dependency rules](dependency-rules.md).

## Preconditions and postconditions

`load` requires an admitted, compatible manifest and no prior loaded instance;
success owns the library and allocates no host-owned service handles. `start`
requires all required services registered; success publishes readiness exactly
once. `quiesce` requires `Started` and prevents new work; `stop` requires
quiesced state and invalidates handles before unregister. `unload` requires no
callbacks, jobs, or owned handles. A failed hook leaves the module unavailable
and invokes only the rollback hooks permitted by the current state.

## Concurrency rules

Lifecycle hooks run on the lifecycle thread. Service lookup may be concurrent
only after registration, and returned handles remain valid until quiesce
begins. Unregister is the linearization point: lookups after it fail and all
in-flight callbacks are joined before it returns. Jobs submitted by a module
carry its cancellation token and must be drained or cancelled before unload.

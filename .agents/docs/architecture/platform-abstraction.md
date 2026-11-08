# Platform abstraction

**Status:** Draft

Runtime modules use platform services rather than OS APIs directly. The
platform provider supplies filesystem, clock, threads, windows/input, shared
libraries, GPU handles and secure randomness through capability interfaces.

```text
Platform::clock().monotonic_ns()
Platform::fs().open(path, mode)
Platform::window().create(desc)
Platform::loader().open(allowlisted_path)
```

Each service declares thread affinity, blocking behavior, error taxonomy and
ownership. Monotonic time is used for deadlines; wall time is metadata only.
Paths are normalized and sandboxed before filesystem access.

Provider startup is `Probed → Configured → Ready → Closing → Closed`; unsupported
features return `Unsupported`, not guessed behavior. Headless mode must provide
clock, filesystem and jobs while window/GPU services may be absent.

Platform tests run on Linux, macOS and Windows (native and headless), including
path encoding, clock monotonicity, loader failure, shutdown ordering and
permission errors. See [plugin ABI](plugin-abi.md) and [security](security.md).

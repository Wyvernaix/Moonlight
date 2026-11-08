# ABI compatibility testing

**Status:** Design target

This document makes the C ABI decision in [ADR 0003](adr/0003-abi-and-plugin-loading.md)
executable. It does not permit C++/Rust/C# layout types to cross the boundary.

## Compatibility matrix

| Host | Plugin | Expected result |
| --- | --- | --- |
| Same ABI major, supported minor | Same or older compatible minor | Admit after negotiation |
| Same ABI major, newer unsupported minor | Any | Reject `IncompatibleAbi` |
| Different ABI major | Any | Reject before calling plugin code |
| Same ABI, different pointer width/endianness | Any | Reject unless a fixture explicitly supports it |
| Valid table, smaller `struct_size` | Any | Read only fields within size; reject missing required fields |
| Valid metadata, wrong symbol/signature | Any | Reject before load/start |

The matrix is a contract: each row requires a fixture and a CI result on every
supported host/compiler combination. “Works on my platform” is not evidence.

## Preconditions and postconditions

`moonlight_plugin_get_api` is called only after path policy, signature/hash,
platform, and symbol checks. The host supplies initialized output storage and
supported ABI bounds. A successful call must return non-null identity strings,
a size covering all required fields, valid function pointers, and a compatible
version. The host then owns the admission decision; the plugin cannot mark
itself admitted.

On rejection, no hook is called and the library is unloaded. On successful
start, every handle is tagged with plugin identity and ABI generation. Stop
invalidates handles before unload; later use returns `Unavailable`, never
dereferences unloaded memory.

## Test families

- Compile fixtures with each supported C/C++/Rust compiler and warning level.
- Run old-host/new-plugin and new-host/old-plugin minor negotiation.
- Mutate `struct_size`, alignment, null pointers, strings, versions and
  function addresses.
- Exercise allocator ownership, callback reentrancy, thread affinity,
  concurrent stop/unload, deadline expiry, panic/exception containment, and
  process-host crashes.
- Compare exported symbol sets and generated header hashes against the
  versioned ABI manifest.

Fuzzing must remain in a subprocess, enforce CPU/memory/time limits, and treat
any sanitizer finding, out-of-bounds read, use-after-free, or unexpected hook
call as a failure. The corpus and ABI manifest are content-addressed.

## Evidence and release gate

CI publishes a matrix result containing host/plugin versions, compiler, target
triple, loader flags, sanitizer mode, fixture hash, and negotiated version.
An ABI-major change requires a new major fixture set and an ADR update; a
minor change requires all old fixtures to pass. See [plugin ABI](plugin-abi.md)
and [versioning](versioning.md).

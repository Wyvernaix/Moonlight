# Plugin ABI

**Status:** Draft

## Admission contract

Plugins export one symbol, `moonlight_plugin_get_api`, with C linkage. The host
passes its ABI major/minor and receives a table whose first fields are
`abi_major`, `abi_minor`, `struct_size`, `plugin_id`, `plugin_version` and
function pointers. The host rejects unknown major versions, undersized
structures, unsupported platforms, invalid signatures, duplicate IDs and
missing required capabilities before calling any hook.

```c
typedef struct MlPluginApi {
  uint32_t abi_major, abi_minor, struct_size;
  const char* plugin_id;
  const char* plugin_version;
  MlStatus (*load)(const MlHostApi*);
  MlStatus (*start)(void);
  MlStatus (*stop)(uint64_t deadline_ns);
  void (*unload)(void);
} MlPluginApi;
MlStatus moonlight_plugin_get_api(uint32_t host_major, uint32_t host_minor,
                                  MlPluginApi* out);
```

Only fixed-width integers, UTF-8 strings, opaque handles, POD structs and
explicit callbacks cross the boundary. No C++ classes, STL, Rust references,
exceptions, panics or compiler-owned allocations cross it. Callbacks are
serialized unless a contract says otherwise; the plugin must identify its
thread-affinity and reentrancy.

## State and failure

`Unloaded → Admitted → Loaded → Started → Stopping → Unloaded`; any admission
or load error leaves the library unloaded. Stop is bounded by a monotonic
deadline. Crashes are isolated by process hosting where configured; in-process
faults mark the plugin unavailable and invalidate its handles.

## Platform loading

Use signed, allow-listed paths, platform-native loader flags, and explicit
close/unload. Verify hash and ABI metadata before execution. See [security](security.md),
[memory ownership](memory-ownership.md), and [versioning](versioning.md).

Test ABI fixtures built by each supported compiler, old/new minor versions,
wrong sizes, allocator misuse, callback reentrancy, unload races and tampered
libraries.

# Memory ownership

**Status:** Accepted

## Rules

Every pointer has a lifetime, owner, mutability and release function in its
contract. Borrowed values are valid only for the call unless a lease is
explicitly returned. Handles are opaque, generation-checked IDs; stale handles
return `InvalidHandle`, never use-after-free.

Cross-language APIs use caller-owned buffers for inputs and an ABI-owned
`moonlight_free` for outputs. A buffer is `{data, size, capacity, allocator_id}`
and may only be released by the allocator that created it. C++ exceptions,
Rust panics and managed exceptions stop at the boundary and become error codes.

```c
typedef struct MlBytes { const uint8_t* data; uint64_t size; } MlBytes;
typedef struct MlOwnedBytes { uint8_t* data; uint64_t size; uint64_t capacity; } MlOwnedBytes;
MlStatus ml_asset_read(MlHandle, MlBytes path, MlOwnedBytes* out);
void ml_free(void* data, uint64_t capacity);
```

## Invariants

- Null and zero-length inputs are defined for every function.
- Output remains valid until `ml_free` or documented owner destruction.
- Callbacks cannot retain borrowed pointers or call after unregister.
- Destruction is idempotent; double release is detected in debug builds.
- Real-time threads use preallocated pools and never call general allocators.

## Lifecycle and failure

`Allocated → Published → Borrowed → Released` is the normal buffer lifecycle.
`Released` is terminal. Allocation failure returns `OutOfMemory` without
partially initialized objects. Crossing an ABI with the wrong allocator,
invalid size or stale handle returns `InvalidArgument`/`InvalidHandle` and
leaves ownership unchanged.

Test ASan/UBSan, Rust Miri where applicable, managed GC pressure, double-free
and stale-handle fixtures, zero-copy lifetime tests, and callback-unregister
race tests. See [plugin ABI](plugin-abi.md).

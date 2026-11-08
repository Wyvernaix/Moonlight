# Security

**Status:** Draft

Security boundaries are deny-by-default. Module and plugin manifests, binaries,
assets and external data are untrusted until schema, size, hash, signature and
policy checks pass. Runtime permissions are capabilities: filesystem roots,
network endpoints, GPU access, process spawning and reflection are separately
granted.

## Invariants

- Never execute code from an asset or document.
- Canonicalize paths and enforce sandbox roots after resolution.
- Verify plugin signatures/hashes before loading.
- Redact credentials and personal data from [observability](observability.md).
- Bound decompression, recursion, allocations, callbacks and request rates.
- Fail closed when policy or verification is unavailable.

Plugin admission and asset loading are state machines ending in `Rejected` on
failure. Security errors expose a stable category and remediation, not secret
details. The host isolates research/import workers where feasible and drops
privileges before processing untrusted content.

Tests include malformed schemas, traversal, symlink escapes, oversized inputs,
signature/hash tampering, denied capabilities, callback abuse, secret
redaction and dependency audit. See [assets and data](assets-and-data.md) and
[versioning](versioning.md).

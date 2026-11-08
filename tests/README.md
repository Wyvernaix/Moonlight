# Cross-module tests

This tree contains tests that do not belong to one module: ABI compatibility,
schema migrations, integration, end-to-end startup, asset pipelines, failure
injection, fuzzing, and regression fixtures. Tests must be runnable through
CTest, Cargo, or the .NET test host and must record unavailable platform/GPU
coverage explicitly.

# Disaster recovery

This runbook restores a reproducible development or release capability after source loss, corrupted generated state, a failed release, compromised credentials, or an unavailable build host. It does not restore user save data or production services.

## Recovery boundaries and prerequisites

The source of truth is the protected Git repository, reviewed commits, tags, lockfiles, workflow definitions, and documented secrets held by the repository administrator. Generated `build/`, `target/`, `bin/`, and `obj/` directories are disposable. Do not restore binaries or credentials from an unverified backup.

Record the incident ID, UTC time, last known good commit/tag, affected platforms, failed command, logs, and whether secrets or customer data may be exposed. Stop release publication and revoke suspected tokens before investigating them. Keep evidence access-controlled and scrubbed.

## Clean-room rebuild

```sh
git clone --no-checkout <repository-url> moonlight-recovery
git -C moonlight-recovery fetch --tags --force
git -C moonlight-recovery checkout <last-known-good-commit>
cd moonlight-recovery
git status --short
python scripts/moonlight.py validate
python python/build.py --configuration release
ctest --preset release --output-on-failure
cargo test --workspace --locked
dotnet test Moonlight.sln --configuration Release
```

The worktree must be clean, validation must report the expected module count, and every stage must exit 0. Repeat on each supported OS. If only generated state is damaged, remove the affected directory (`rm -rf build/release` or `Remove-Item -Recurse build/release`) and rerun; never remove source or lockfiles.

## Recovery decisions

| Symptom | Safe action | Exit condition |
|---|---|---|
| stale CMake/cache state | remove one affected build directory and reconfigure | CTest passes |
| dependency/toolchain drift | restore pinned metadata and use the documented toolchain | versions match CI |
| failed release artifact | keep release draft, rebuild from the tagged commit | archive hash and matrix agree |
| suspected credential exposure | revoke/rotate, preserve redacted evidence, audit workflows | security owner closes incident |
| corrupted repository/ref | use protected remote/tag and verify commit ancestry | clean clone reproduces |

Do not force-push, rewrite published tags, disable security checks, or upload unreviewed dumps. Roll forward with a reviewed fix; rollback to the last known good commit/tag when recovery cannot establish integrity.

## Evidence and post-recovery tests

Preserve redacted logs, checksums, commit IDs, tool versions, test reports, and the recovery commands. Run `git diff --check`, Python compilation, module validation, the complete build, and the release-verification procedure. Add a regression test or documentation correction for the failure mode. CI parity is the required workflow matrix; a single recovered workstation is insufficient evidence.

# Release verification

This procedure verifies the tag-driven source release produced by `.github/workflows/release.yml`. It does not claim that the repository is a binary installer.

## Clean-room preflight

Use a fresh clone, a clean worktree, and the pinned toolchain. Do not use uncommitted generated output.

```sh
git clone --no-checkout <repository-url> moonlight-release-check
git -C moonlight-release-check fetch --tags --force
git -C moonlight-release-check checkout <commit-or-tag>
cd moonlight-release-check
git status --short
python --version; cmake --version; ninja --version
cargo --version; rustc --version; dotnet --version
```

The worktree must be empty and the commit must be the intended release commit. Record OS, architecture, compiler, SDK, and versions. Repeat on Ubuntu, macOS, and Windows; use Developer PowerShell on Windows.

## Verification sequence

```sh
python scripts/moonlight.py validate
python python/build.py --configuration release
ctest --preset release --output-on-failure
cargo test --workspace --locked
dotnet test Moonlight.sln --configuration Release
mkdir -p build/artifacts
git archive --format=tar.gz --output build/artifacts/moonlight-source.tar.gz HEAD
tar -tzf build/artifacts/moonlight-source.tar.gz > build/artifacts/source-files.txt
```

Every command exits 0. The catalog validates, the complete build prints `Moonlight build completed successfully.`, and CTest/Rust/.NET report no failures. The archive must contain source and required metadata, must not contain `build/`, `target/`, `bin/`, `obj/`, credentials, or machine-specific files, and must identify the expected commit with `git rev-parse HEAD`.

## Evidence, rollback, and failure diagnosis

Attach command output, `build/release/Testing/Temporary/LastTest.log`, the tool-version record, archive SHA-256, and the exact commit/tag. Verify the archive with `sha256sum build/artifacts/moonlight-source.tar.gz` (PowerShell: `Get-FileHash`). A failed matrix job blocks publication. Fix the first failure and rerun from a fresh checkout; rollback by deleting the draft release/tag before publication or by reverting the release commit. Never rewrite a published tag.

Security review must confirm that logs and archives contain no tokens, private keys, customer data, absolute home paths, or unreviewed scripts. Release credentials remain in GitHub Actions secrets and are never copied into local files or command arguments. CI parity is the release workflow's Ubuntu/macOS/Windows matrix; local verification is evidence, not a bypass.

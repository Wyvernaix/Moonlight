# Release packaging

Releases are tag-driven by `.github/workflows/release.yml`; tags must match `v*.*.*`. The workflow builds Release on Ubuntu, macOS, and Windows, archives the checked-out source with `git archive`, uploads one artifact per OS, and publishes a GitHub release for version tags.

Before tagging, run:

```sh
git status --short
python python/build.py --configuration release
cargo test --workspace --locked
dotnet test Moonlight.sln --configuration Release
ctest --preset release --output-on-failure
```

Verify the changelog, module/API compatibility, supported-platform notes, and that no generated output or secret is tracked. Use an annotated tag only after owner approval; never rewrite a published tag. The release artifact is source packaging, not an installer—do not describe it as a binary distribution unless packaging is added and documented.

For a local archive check:

```sh
mkdir -p build/artifacts
git archive --format=tar.gz --output build/artifacts/moonlight-local.tar.gz HEAD
tar -tzf build/artifacts/moonlight-local.tar.gz | head
```

On Windows use `New-Item -ItemType Directory build/artifacts` and `tar -tf`. Confirm archives contain the intended commit and no credentials. Release failures are owned by release/CI maintainers; do not bypass failed platform jobs.

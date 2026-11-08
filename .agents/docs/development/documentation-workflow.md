# Documentation workflow

Development documentation lives under `.agents/docs/development/` and must describe the repository as it exists, not an aspirational toolchain. When commands, presets, manifests, CI, ownership, or compatibility rules change, update the relevant page in the same change. Prefer executable commands, expected output, platform notes, and failure diagnosis over vague guidance.

For a documentation change:

```sh
find .agents/docs/development -name '*.md' -print | sort
git diff --check
python scripts/moonlight.py validate
```

Run build/test commands when documentation claims they work; documentation-only edits need no full rebuild if the commands are unchanged, but state what was not run. Verify links and paths against the repository. Do not edit generated docs, files outside the requested documentation directory, or CI files merely to make an example pass.

Every page should identify ownership/lifetime and safety implications where relevant. Avoid secrets, customer data, unsupported promises, and commands that delete broad paths. Keep examples copy/pasteable on Linux/macOS/Windows or label shell-specific syntax. Reviewers should be able to reproduce the documented result from a clean checkout.

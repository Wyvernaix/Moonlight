# Documentation linting

Documentation is valid only when commands, paths, links, fences, and whitespace match the repository. This scope is `.agents/docs/development/`; do not change other files while fixing a documentation defect.

## Clean-room lint sequence

```sh
find .agents/docs/development -type f -name '*.md' -print | sort
python - <<'PY'
from pathlib import Path
root = Path('.agents/docs/development')
for path in sorted(root.glob('*.md')):
    text = path.read_text()
    assert text.endswith('\n'), f'{path}: missing final newline'
    assert '\t' not in text, f'{path}: tab character'
    assert '```' in text, f'{path}: no fenced command/evidence block'
    depth = sum(line.startswith('```') for line in text.splitlines())
    assert depth % 2 == 0, f'{path}: unbalanced fences'
print(f'linted {len(list(root.glob("*.md")))} development documents')
PY
git diff --check -- .agents/docs/development
python scripts/moonlight.py validate
```

The script prints the document count, exits 0, and `git diff --check` reports nothing. Review every relative Markdown link against the repository and ensure command paths exist. For links outside the repository, record the URL and intended purpose; do not claim network availability in tests.

## Evidence and CI parity

Record the commit, lint commands, output, and any unavailable external-link check. Review shell labels (`sh`, `bash`, `bat`, `powershell`) against the command syntax, expected artifacts, exit conditions, platform matrix, rollback, failure diagnosis, security boundary, static/fuzz/property-analysis guidance, test evidence, and corresponding workflow. Documentation-only changes do not require a full build when commands are unchanged; if a command or toolchain claim changes, run the smallest affected build/test command and report it.

Rollback by reverting only the documentation commit. Never “fix” linting by weakening the checker, hiding a command, or adding credentials. Generated logs and test artifacts remain under ignored `build/`; scrub secrets and personal paths before sharing.

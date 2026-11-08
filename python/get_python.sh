#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
PYTHON_DIR="$ROOT_DIR/.venv"

if command -v python3 >/dev/null 2>&1; then
    PYTHON_BIN=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON_BIN=python
else
    echo "Python 3.11 or newer is required." >&2
    exit 1
fi

"$PYTHON_BIN" -c 'import sys; raise SystemExit(0 if sys.version_info >= (3, 11) else 1)' ||
    { echo "Python 3.11 or newer is required."; exit 1; }

"$PYTHON_BIN" -m venv "$PYTHON_DIR"
"$PYTHON_DIR/bin/python" -m pip install --upgrade pip
"$PYTHON_DIR/bin/python" -m pip install -r "$ROOT_DIR/python/requirements.txt"
printf 'Python environment ready: %s\n' "$PYTHON_DIR"

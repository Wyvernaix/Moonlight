#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
PYTHON="$ROOT_DIR/.venv/bin/python"

if [ ! -x "$PYTHON" ]; then
    "$ROOT_DIR/python/get_python.sh"
fi

exec "$PYTHON" -m pip "$@"

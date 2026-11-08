#!/usr/bin/env python3
"""Run Moonlight project tooling from a source checkout."""

from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).parent))

from moonlight.cli import main


if __name__ == "__main__":
    raise SystemExit(main())
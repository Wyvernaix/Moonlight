#!/usr/bin/env python3
"""Build and validate the complete Moonlight foundation."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def run(command: list[str], *, label: str) -> None:
    print(f"\n==> {label}", flush=True)
    print("$ " + " ".join(command), flush=True)
    try:
        subprocess.run(command, cwd=ROOT, check=True)
    except FileNotFoundError as error:
        raise SystemExit(
            f"Required tool '{error.filename}' was not found. "
            "Install the project prerequisites and try again."
        ) from error
    except subprocess.CalledProcessError as error:
        raise SystemExit(
            f"{label} failed with exit code {error.returncode}."
        ) from error


def require_tools(commands: list[str]) -> None:
    missing = [command for command in commands if shutil.which(command) is None]
    if missing:
        joined = ", ".join(missing)
        raise SystemExit(f"Required tools are missing from PATH: {joined}")


def build_native(configuration: str) -> None:
    preset = configuration
    run(["cmake", "--preset", preset], label=f"Configure native ({configuration})")
    run(
        ["cmake", "--build", "--preset", preset],
        label=f"Build native ({configuration})",
    )
    run(
        ["ctest", "--preset", preset, "--output-on-failure"],
        label=f"Test native ({configuration})",
    )


def build_rust() -> None:
    run(["cargo", "fmt", "--all", "--", "--check"], label="Check Rust formatting")
    run(
        ["cargo", "clippy", "--workspace", "--all-targets", "--", "-D", "warnings"],
        label="Lint Rust",
    )
    run(["cargo", "check", "--workspace", "--locked"], label="Check Rust workspace")
    run(["cargo", "test", "--workspace", "--locked"], label="Test Rust workspace")


def build_managed(configuration: str) -> None:
    run(["dotnet", "restore", "Moonlight.sln"], label="Restore managed projects")
    run(
        ["dotnet", "build", "Moonlight.sln", "--no-restore", "--configuration", configuration],
        label=f"Build managed projects ({configuration})",
    )
    run(
        ["dotnet", "test", "Moonlight.sln", "--no-build", "--configuration", configuration],
        label=f"Test managed projects ({configuration})",
    )


def validate_modules() -> None:
    run(
        [sys.executable, "scripts/moonlight.py", "validate"],
        label="Validate module graph",
    )
    run(
        [
            sys.executable,
            "scripts/moonlight.py",
            "export",
            "--out", "build/graph/module-catalog.json",
        ],
        label="Export module catalog",
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build and validate all current Moonlight language targets."
    )
    parser.add_argument(
        "--configuration",
        choices=("debug", "release"),
        default="debug",
        help="Native CMake and managed .NET configuration (default: debug).",
    )
    parser.add_argument(
        "--skip",
        action="append",
        choices=("native", "rust", "managed", "modules"),
        default=[],
        help="Skip a stage; may be supplied more than once.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    skipped = set(args.skip)
    require_tools(
        [
            tool
            for stage, tool in (
                ("native", "cmake"),
                ("native", "ctest"),
                ("rust", "cargo"),
                ("managed", "dotnet"),
            )
            if stage not in skipped
        ]
    )

    if "modules" not in skipped:
        validate_modules()
    if "native" not in skipped:
        build_native(args.configuration)
    if "rust" not in skipped:
        build_rust()
    if "managed" not in skipped:
        build_managed(args.configuration.capitalize())

    print("\nMoonlight build completed successfully.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

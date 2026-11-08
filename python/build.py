#!/usr/bin/env python3
"""Build and validate the complete Moonlight foundation."""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Sequence


ROOT = Path(__file__).resolve().parents[1]


class BuildRunner:
    def __init__(self, *, dry_run: bool = False) -> None:
        self.dry_run = dry_run

    def run(self, command: Sequence[str], *, label: str) -> None:
        command = list(command)
        print(f"\n==> {label}", flush=True)
        print("$ " + " ".join(command), flush=True)
        if self.dry_run:
            return
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


def run(command: Sequence[str], *, label: str, runner: BuildRunner) -> None:
    runner.run(command, label=label)


def require_tools(commands: list[str]) -> None:
    missing = [command for command in commands if shutil.which(command) is None]
    if missing:
        joined = ", ".join(missing)
        raise SystemExit(f"Required tools are missing from PATH: {joined}")


def build_native(configuration: str, *, runner: BuildRunner, jobs: int | None) -> None:
    preset = configuration
    run(
        ["cmake", "--preset", preset],
        label=f"Configure native ({configuration})",
        runner=runner,
    )
    build = ["cmake", "--build", "--preset", preset]
    if jobs is not None:
        build.extend(["--parallel", str(jobs)])
    run(
        build,
        label=f"Build native ({configuration})",
        runner=runner,
    )
    run(
        ["ctest", "--preset", preset, "--output-on-failure"],
        label=f"Test native ({configuration})",
        runner=runner,
    )


def bootstrap_dependencies(*, runner: BuildRunner) -> None:
    run(
        [sys.executable, "python/bootstrap.py"],
        label="Bootstrap third-party dependencies",
        runner=runner,
    )
    if not runner.dry_run:
        os.environ["VCPKG_ROOT"] = str(ROOT / "third_party" / "vcpkg")


def build_workbench(*, runner: BuildRunner, jobs: int | None) -> None:
    run(
        ["cmake", "--preset", "workbench-vcpkg"],
        label="Configure Workbench",
        runner=runner,
    )
    build = [
        "cmake",
        "--build",
        "--preset",
        "workbench-vcpkg",
        "--target",
        "moonlight_workbench",
    ]
    if jobs is not None:
        build.extend(["--parallel", str(jobs)])
    run(build, label="Build Workbench", runner=runner)


def build_rust(*, runner: BuildRunner) -> None:
    run(
        ["cargo", "fmt", "--all", "--", "--check"],
        label="Check Rust formatting",
        runner=runner,
    )
    run(
        ["cargo", "clippy", "--workspace", "--all-targets", "--", "-D", "warnings"],
        label="Lint Rust",
        runner=runner,
    )
    run(
        ["cargo", "check", "--workspace", "--locked"],
        label="Check Rust workspace",
        runner=runner,
    )
    run(
        ["cargo", "test", "--workspace", "--locked"],
        label="Test Rust workspace",
        runner=runner,
    )


def build_managed(configuration: str, *, runner: BuildRunner) -> None:
    run(
        ["dotnet", "restore", "Moonlight.sln"],
        label="Restore managed projects",
        runner=runner,
    )
    run(
        ["dotnet", "build", "Moonlight.sln", "--no-restore", "--configuration", configuration],
        label=f"Build managed projects ({configuration})",
        runner=runner,
    )
    run(
        ["dotnet", "test", "Moonlight.sln", "--no-build", "--configuration", configuration],
        label=f"Test managed projects ({configuration})",
        runner=runner,
    )


def validate_modules(*, runner: BuildRunner) -> None:
    run(
        [sys.executable, "scripts/moonlight.py", "validate"],
        label="Validate module graph",
        runner=runner,
    )
    run(
        [
            sys.executable,
            "scripts/moonlight.py",
            "export",
            "--out", "build/graph/module-catalog.json",
        ],
        label="Export module catalog",
        runner=runner,
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
    parser.add_argument(
        "--workbench",
        action="store_true",
        help="Bootstrap dependencies, then configure and build the Vulkan Workbench.",
    )
    parser.add_argument(
        "--jobs",
        type=int,
        metavar="N",
        help="Limit native build parallelism to N jobs.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print all commands without executing them.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.jobs is not None and args.jobs < 1:
        raise SystemExit("--jobs must be at least 1.")
    skipped = set(args.skip)
    runner = BuildRunner(dry_run=args.dry_run)
    if not args.dry_run:
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
            + (["cmake"] if args.workbench else [])
        )

    if args.workbench:
        bootstrap_dependencies(runner=runner)
    if "modules" not in skipped:
        validate_modules(runner=runner)
    if "native" not in skipped:
        build_native(args.configuration, runner=runner, jobs=args.jobs)
    if "rust" not in skipped:
        build_rust(runner=runner)
    if "managed" not in skipped:
        build_managed(args.configuration.capitalize(), runner=runner)
    if args.workbench:
        build_workbench(runner=runner, jobs=args.jobs)

    suffix = " (dry run)" if args.dry_run else ""
    print(f"\nMoonlight build completed successfully{suffix}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

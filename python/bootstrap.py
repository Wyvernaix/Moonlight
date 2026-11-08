#!/usr/bin/env python3
"""Install Moonlight's reproducible third-party CMake dependencies."""

from __future__ import annotations

import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
VCPKG_ROOT = ROOT / "third_party" / "vcpkg"
VCPKG_REF = os.environ.get("MOONLIGHT_VCPKG_REF", "2025.12.12")


def command_exists(command: str) -> bool:
    return shutil.which(command) is not None


def run(command: list[str], *, cwd: Path | None = None) -> None:
    print("$ " + " ".join(command), flush=True)
    try:
        subprocess.run(command, cwd=cwd or ROOT, check=True)
    except FileNotFoundError as error:
        raise SystemExit(
            f"Required tool '{error.filename}' was not found. "
            "Install Git, CMake, and a C++ compiler first."
        ) from error
    except subprocess.CalledProcessError as error:
        raise SystemExit(
            f"Dependency bootstrap failed with exit code {error.returncode}."
        ) from error


def vcpkg_executable() -> Path:
    name = "vcpkg.exe" if os.name == "nt" else "vcpkg"
    return VCPKG_ROOT / name


def triplet() -> str:
    system = platform.system()
    machine = platform.machine().lower()
    if system == "Windows":
        return "arm64-windows" if machine in {"arm64", "aarch64"} else "x64-windows"
    if system == "Darwin":
        return "arm64-osx" if machine in {"arm64", "aarch64"} else "x64-osx"
    if system == "Linux":
        return "arm64-linux" if machine in {"arm64", "aarch64"} else "x64-linux"
    raise SystemExit(
        f"Unsupported host platform '{system}'. "
        "Moonlight supports Windows, macOS, and Linux."
    )


def check_host_build_tools() -> None:
    if platform.system() not in {"Linux", "Darwin"}:
        return
    missing = missing_host_packages()
    if not missing:
        return
    install_hint = host_package_install_command(missing)
    raise SystemExit(
        "The host is missing build tools required by a transitive dependency: "
        f"{', '.join(missing)}.\nInstall them with:\n  {install_hint}\n"
        "The project bootstrap downloads project dependencies, but cannot "
        "install OS packages without your permission."
    )


def missing_host_packages() -> list[str]:
    missing: list[str] = []
    required_tools = ["autoconf", "autoreconf", "automake", "pkg-config"]
    if platform.system() == "Linux":
        required_tools.append("libtoolize")
    elif not (command_exists("libtoolize") or command_exists("glibtoolize")):
        required_tools.append("libtoolize")
    if any(not command_exists(tool) for tool in required_tools):
        missing.extend(["autoconf", "automake", "libtool", "pkg-config"])
    aclocal = shutil.which("aclocal")
    if aclocal:
        macro_dir = subprocess.run(
            [aclocal, "--print-ac-dir"],
            capture_output=True,
            text=True,
            check=False,
        ).stdout.strip()
        if macro_dir and not (Path(macro_dir) / "ax_pthread.m4").exists():
            missing.append("autoconf-archive")
    return list(dict.fromkeys(missing))


def host_package_install_command(packages: list[str]) -> str:
    """Return a distro-appropriate command without executing package managers."""
    package_names = " ".join(packages)
    if platform.system() == "Darwin":
        return f"brew install {package_names}"
    if platform.system() == "Windows":
        return (
            "Install the missing tools with winget or MSYS2, then rerun bootstrap "
            f"(required: {package_names})"
        )

    candidates = [
        ("pacman", f"sudo pacman -S --needed {package_names}"),
        ("dnf", f"sudo dnf install {package_names}"),
        ("zypper", f"sudo zypper install {package_names}"),
        ("apk", f"sudo apk add {package_names}"),
        ("apt-get", f"sudo apt-get update && sudo apt-get install {package_names}"),
    ]
    for manager, command in candidates:
        if command_exists(manager):
            return command
    return (
        "Install the packages using your distribution's package manager "
        f"(required: {package_names})"
    )


def install_host_tools() -> None:
    """Explicitly install missing host tools using the detected package manager."""
    if platform.system() in {"Windows", "Darwin"}:
        raise SystemExit(
            "Automatic host-tool installation is only supported for Linux package "
            "managers. Install the reported tools with Homebrew, winget, or MSYS2."
        )
    missing = missing_host_packages()
    if not missing:
        print("Host build tools are already installed.")
        return
    manager = next(
        (
            manager
            for manager in ("pacman", "dnf", "zypper", "apk", "apt-get")
            if command_exists(manager)
        ),
        None,
    )
    if manager is None:
        raise SystemExit(host_package_install_command(missing))
    commands = {
        "pacman": ["sudo", "pacman", "-S", "--needed", *missing],
        "dnf": ["sudo", "dnf", "install", *missing],
        "zypper": ["sudo", "zypper", "install", *missing],
        "apk": ["sudo", "apk", "add", *missing],
        "apt-get": ["sudo", "apt-get", "update"],
    }
    if manager == "apt-get":
        run(commands[manager])
        run(["sudo", "apt-get", "install", *missing])
    else:
        run(commands[manager])

def install_vcpkg() -> None:
    if not command_exists("git"):
        raise SystemExit("Git is required to download vcpkg and is not on PATH.")
    if not command_exists("cmake"):
        raise SystemExit("CMake is required to bootstrap vcpkg and is not on PATH.")
    check_host_build_tools()

    if not VCPKG_ROOT.exists():
        VCPKG_ROOT.parent.mkdir(parents=True, exist_ok=True)
        run(
            [
                "git",
                "clone",
                "--depth",
                "1",
                "--branch",
                VCPKG_REF,
                "https://github.com/microsoft/vcpkg.git",
                str(VCPKG_ROOT),
            ]
        )
    elif not (VCPKG_ROOT / ".git").exists():
        raise SystemExit(
            f"{VCPKG_ROOT} exists but is not a vcpkg checkout. "
            "Remove that directory and rerun bootstrap."
        )
    else:
        current_ref = subprocess.run(
            ["git", "-C", str(VCPKG_ROOT), "describe", "--tags", "--exact-match", "HEAD"],
            capture_output=True,
            text=True,
            check=False,
        ).stdout.strip()
        if current_ref != VCPKG_REF:
            run(
                [
                    "git",
                    "-C",
                    str(VCPKG_ROOT),
                    "fetch",
                    "--depth",
                    "1",
                    "origin",
                    f"refs/tags/{VCPKG_REF}:refs/tags/{VCPKG_REF}",
                ]
            )
            run(
                [
                    "git",
                    "-C",
                    str(VCPKG_ROOT),
                    "checkout",
                    "--detach",
                    f"refs/tags/{VCPKG_REF}",
                ]
            )

    executable = vcpkg_executable()
    if not executable.exists():
        bootstrap = "bootstrap-vcpkg.bat" if os.name == "nt" else "./bootstrap-vcpkg.sh"
        run([bootstrap, "-disableMetrics"], cwd=VCPKG_ROOT)

    os.environ["VCPKG_ROOT"] = str(VCPKG_ROOT)
    run(
        [
            str(executable),
            "install",
            "--triplet",
            triplet(),
            "--x-feature=shader-toolchain",
            "--feature-flags=manifests",
        ],
        cwd=ROOT,
    )
    print(f"vcpkg dependencies are ready ({triplet()}).")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Download and install Moonlight's CMake dependencies."
    )
    parser.add_argument(
        "--print-root",
        action="store_true",
        help="Print the repository-local vcpkg path and exit.",
    )
    parser.add_argument(
        "--install-host-tools",
        action="store_true",
        help="Install missing Linux host build tools using the detected package manager.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.print_root:
        print(VCPKG_ROOT)
        return 0
    if args.install_host_tools:
        install_host_tools()
    install_vcpkg()
    return 0


if __name__ == "__main__":
    sys.exit(main())

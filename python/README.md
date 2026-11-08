# Python tooling

Python is used for repository automation, build orchestration, validation, and
project management. It is not a runtime language for engine modules.

## Setup

From the repository root:

```sh
python/get_python.sh
```

On Windows:

```bat
python\get_python.bat
```

The scripts create `.venv/` and install `requirements.txt`. Use `python/pip.sh`
or `python/pip.cmd` to invoke pip from that environment without activating it.

## Platform files

`Platform/` contains CMake installation fragments for packaging Python tooling:

- `Linux/install_files_linux.cmake`
- `Mac/install_files_mac.cmake`
- `Windows/install_files_windows.cmake`

The files are installation-only; they do not run during the normal C++ build
unless explicitly included by a packaging target.
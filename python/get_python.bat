@echo off
setlocal
set "ROOT_DIR=%~dp0.."
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "PYTHON_DIR=%ROOT_DIR%\.venv"

where py >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    set "PYTHON_BIN=py -3"
) else (
    where python >nul 2>nul
    if not %ERRORLEVEL% EQU 0 (
        echo Python 3.11 or newer is required.
        exit /b 1
    )
    set "PYTHON_BIN=python"
)

%PYTHON_BIN% -c "import sys; raise SystemExit(0 if sys.version_info >= (3, 11) else 1)"
if not %ERRORLEVEL% EQU 0 (
    echo Python 3.11 or newer is required.
    exit /b 1
)

%PYTHON_BIN% -m venv "%PYTHON_DIR%"
"%PYTHON_DIR%\Scripts\python.exe" -m pip install --upgrade pip
"%PYTHON_DIR%\Scripts\python.exe" -m pip install -r "%ROOT_DIR%\python\requirements.txt"
echo Python environment ready: %PYTHON_DIR%

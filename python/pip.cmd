@echo off
setlocal
set "ROOT_DIR=%~dp0.."
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"
set "PYTHON=%ROOT_DIR%\.venv\Scripts\python.exe"

if not exist "%PYTHON%" call "%ROOT_DIR%\python\get_python.bat"
if not exist "%PYTHON%" exit /b 1

"%PYTHON%" -m pip %*

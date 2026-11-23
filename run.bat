@echo off
setlocal
cd /d "%~dp0"
echo Building complete port scheduling project...
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\build_windows.ps1"
if errorlevel 1 (
  echo.
  echo BUILD FAILED. Check that GCC is installed and available in PATH.
  pause
  exit /b 1
)
echo.
echo Starting interactive port menu...
main.exe --menu
if errorlevel 1 (
  echo.
  echo Application returned an error.
)
pause

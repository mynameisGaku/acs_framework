@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Sync-ProjectFilters.ps1" %*
set "syncExitCode=%errorlevel%"
pause
exit /b %syncExitCode%

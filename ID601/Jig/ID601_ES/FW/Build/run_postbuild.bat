@echo off
setlocal EnableExtensions DisableDelayedExpansion
set "POSTBUILD_SCRIPT=%~dp0postbuild.py"
if not exist "%POSTBUILD_SCRIPT%" (
    echo [BAT] ERROR: postbuild.py was not found.
    exit /b 1
)
where py >nul 2>&1
if errorlevel 1 goto use_python
py -3 "%POSTBUILD_SCRIPT%"
exit /b %ERRORLEVEL%
:use_python
python "%POSTBUILD_SCRIPT%"
exit /b %ERRORLEVEL%

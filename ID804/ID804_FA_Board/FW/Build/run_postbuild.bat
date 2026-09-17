@echo off
setlocal EnableExtensions DisableDelayedExpansion

set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "PROJECT_ROOT=%%~fI"

set "POSTBUILD_SCRIPT=%SCRIPT_DIR%postbuild.py"
set "VERSION_HEADER=%PROJECT_ROOT%\App\User\App\Inc\version.h"
set "SOURCE_BIN=%PROJECT_ROOT%\App\EWARM\App\Exe\App.bin"
set "RELEASE_DIR=%PROJECT_ROOT%\App\EWARM\App\Exe\Release"

echo.
echo === ID804 Firmware Post-build ===
echo version.h: "%VERSION_HEADER%"
echo Source app.bin: "%SOURCE_BIN%"

if not exist "%POSTBUILD_SCRIPT%" (
    echo ERROR: postbuild.py was not found: "%POSTBUILD_SCRIPT%"
    pause
    exit /b 1
)

if not exist "%VERSION_HEADER%" (
    echo ERROR: version.h was not found: "%VERSION_HEADER%"
    pause
    exit /b 1
)

for /f "tokens=3" %%A in ('findstr /c:"FW_VER_MAJOR" "%VERSION_HEADER%"') do set "FW_VER_MAJOR=%%A"
for /f "tokens=3" %%A in ('findstr /c:"FW_VER_MINOR" "%VERSION_HEADER%"') do set "FW_VER_MINOR=%%A"
for /f "tokens=3" %%A in ('findstr /c:"FW_VER_BUILD" "%VERSION_HEADER%"') do set "FW_VER_BUILD=%%A"
for /f "tokens=3" %%A in ('findstr /c:"FW_GIT_REV" "%VERSION_HEADER%"') do set "FW_GIT_REV=%%~A"

if not defined FW_VER_MAJOR goto :invalid_version_header
if not defined FW_VER_MINOR goto :invalid_version_header
if not defined FW_VER_BUILD goto :invalid_version_header
if not defined FW_GIT_REV goto :invalid_version_header

set "RELEASE_IMAGE=%RELEASE_DIR%\ID804_FA_Board_v%FW_VER_MAJOR%.%FW_VER_MINOR%.%FW_VER_BUILD%_%FW_GIT_REV%.bin"
echo Release image: "%RELEASE_IMAGE%"

git -C "%PROJECT_ROOT%" rev-parse --is-inside-work-tree >nul 2>&1
if errorlevel 1 (
    echo WARNING: Unable to determine the Git working-tree status.
) else (
    set "GIT_DIRTY="
    for /f "delims=" %%A in ('git -C "%PROJECT_ROOT%" status --porcelain') do set "GIT_DIRTY=1"
    if defined GIT_DIRTY (
        echo WARNING: Git working tree is dirty. postbuild.py will refuse to create a release image when FW_GIT_REV contains -dirty.
    ) else (
        echo Git working tree is clean.
    )
)

if not exist "%SOURCE_BIN%" (
    echo ERROR: Source app.bin was not found: "%SOURCE_BIN%"
    exit /b 1
)

where py >nul 2>&1
if not errorlevel 1 (
    py -3 "%POSTBUILD_SCRIPT%"
) else (
    python "%POSTBUILD_SCRIPT%"
)
set "POSTBUILD_EXIT_CODE=%ERRORLEVEL%"

if not "%POSTBUILD_EXIT_CODE%"=="0" (
    echo ERROR: postbuild.py failed with exit code %POSTBUILD_EXIT_CODE%.
    pause
    exit /b %POSTBUILD_EXIT_CODE%
)

if not exist "%RELEASE_IMAGE%" (
    echo ERROR: postbuild.py completed but the release image was not found: "%RELEASE_IMAGE%"
    pause
    exit /b 1
)

echo Release image created: "%RELEASE_IMAGE%"
pause
exit /b 0

:invalid_version_header
echo ERROR: Required firmware version macros were not found in "%VERSION_HEADER%"
pause
exit /b 1

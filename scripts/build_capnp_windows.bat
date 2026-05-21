@echo off
setlocal enabledelayedexpansion

set "GIT_URL=%~1"
set "VERSION_MODE=%~2"
set "GIT_BRANCH=%~3"
set "GIT_TAG=%~4"
set "GIT_COMMIT=%~5"
set "REPO_DIR=%~6"
set "INSTALL_DIR=%~7"

if not defined LIBMAN_ROOT set "LIBMAN_ROOT=%~dp0.."
if "!GIT_URL!"=="" set "GIT_URL=https://github.com/capnproto/capnproto.git"
if "!VERSION_MODE!"=="" set "VERSION_MODE=branch"
if "!GIT_BRANCH!"=="" set "GIT_BRANCH=master"
if "!REPO_DIR!"=="" set "REPO_DIR=!LIBMAN_ROOT!/capnproto"
if "!INSTALL_DIR!"=="" set "INSTALL_DIR=!LIBMAN_ROOT!/capnp-install"

rem Use forward slashes only (\c++ is broken in batch %VAR%\suffix parsing).
set "REPO_DIR=!REPO_DIR:\=/!"
set "INSTALL_DIR=!INSTALL_DIR:\=/!"

set "STAMP_FILE=!INSTALL_DIR!/.built"
set "STATE_FILE=!INSTALL_DIR!/.capnp_revision"
set "CAPNP_EXE=!INSTALL_DIR!/bin/capnp.exe"
set "CAPNP_CPP_BUILD=!REPO_DIR!/c++/build"

if not exist "!INSTALL_DIR!" mkdir "!INSTALL_DIR!"

if exist "!STAMP_FILE!" if exist "!CAPNP_EXE!" if exist "!INSTALL_DIR!/include/capnp/message.h" (
    echo Cap'n Proto already installed at !INSTALL_DIR!
    exit /b 0
)

if not exist "!REPO_DIR!/.git" (
    if exist "!REPO_DIR!" (
        echo ERROR: !REPO_DIR! exists but is not a git clone.
        echo Remove it so Cap'n Proto can be cloned automatically: rmdir /s /q "!REPO_DIR!"
        exit /b 1
    )
    git clone "!GIT_URL!" "!REPO_DIR!"
    if errorlevel 1 exit /b 1
)

cd /d "!REPO_DIR!"
git fetch --tags origin
if errorlevel 1 exit /b 1

if /I "!VERSION_MODE!"=="branch" (
    for /f %%i in ('git rev-parse origin/!GIT_BRANCH!') do set TARGET_REV=%%i
) else if /I "!VERSION_MODE!"=="tag" (
    for /f %%i in ('git rev-parse refs/tags/!GIT_TAG!') do set TARGET_REV=%%i
) else if /I "!VERSION_MODE!"=="commit" (
    for /f %%i in ('git rev-parse !GIT_COMMIT!') do set TARGET_REV=%%i
) else (
    echo Unsupported CAPNP_VERSION_MODE: !VERSION_MODE!
    exit /b 1
)

set CURRENT_REV=
if exist "!STATE_FILE!" (
    set /p CURRENT_REV=<"!STATE_FILE!"
)

if "!CURRENT_REV!"=="!TARGET_REV!" if exist "!STAMP_FILE!" if exist "!INSTALL_DIR!/include/capnp/message.h" (
    echo Cap'n Proto is already up to date: !TARGET_REV!
    exit /b 0
)

git reset --hard !TARGET_REV!
if errorlevel 1 exit /b 1

if not exist "!CAPNP_CPP_BUILD!" mkdir "!CAPNP_CPP_BUILD!"
cd /d "!CAPNP_CPP_BUILD!"

for %%I in ("!INSTALL_DIR!") do set "INSTALL_DIR_NATIVE=%%~fI"
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="!INSTALL_DIR_NATIVE!"
if errorlevel 1 exit /b 1

mingw32-make -j4 capnp kj
if errorlevel 1 exit /b 1

mingw32-make -j4 install
if errorlevel 1 exit /b 1

if not exist "!INSTALL_DIR!/include/capnp/message.h" (
    echo ERROR: capnp headers not found under !INSTALL_DIR!/include/capnp after install.
    echo Check CMAKE_INSTALL_PREFIX and mingw32-make install output.
    exit /b 1
)

echo !TARGET_REV!>"!STATE_FILE!"
echo built>"!STAMP_FILE!"

echo Cap'n Proto installed at revision !TARGET_REV!
exit /b 0

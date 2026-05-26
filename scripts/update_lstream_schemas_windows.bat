@echo off
setlocal enabledelayedexpansion

set "GIT_URL=%~1"
set "VERSION_MODE=%~2"
set "GIT_BRANCH=%~3"
set "GIT_TAG=%~4"
set "GIT_COMMIT=%~5"
set "REPO_DIR=%~6"
set "OUT_DIR=%~7"
set "CAPNP_ROOT=%~8"

if not defined LIBMAN_ROOT set "LIBMAN_ROOT=%~dp0.."
if "!GIT_URL!"=="" set "GIT_URL=https://codeberg.org/klayoutmatthias/lstream.git"
if "!VERSION_MODE!"=="" set "VERSION_MODE=branch"
if "!GIT_BRANCH!"=="" set "GIT_BRANCH=main"
if "!REPO_DIR!"=="" set "REPO_DIR=!LIBMAN_ROOT!/.deps/lstream"
if "!OUT_DIR!"=="" set "OUT_DIR=!LIBMAN_ROOT!/capnp"
if "!CAPNP_ROOT!"=="" set "CAPNP_ROOT=!LIBMAN_ROOT!/capnp-install"

set "REPO_DIR=!REPO_DIR:\=/!"
set "OUT_DIR=!OUT_DIR:\=/!"
set "CAPNP_ROOT=!CAPNP_ROOT:\=/!"

set "STAMP_FILE=!OUT_DIR!/schemas_built_stamp"
set "STATE_FILE=!OUT_DIR!/.lstream_revision"
set "CAPNP_EXE=!CAPNP_ROOT!/bin/capnp.exe"
set "CAPNPC_CXX=!CAPNP_ROOT!/bin/capnpc-c++.exe"

if "!GIT_URL!"=="" (
    echo ERROR: GIT_URL is empty
    exit /b 1
)
if "!REPO_DIR!"=="" (
    echo ERROR: REPO_DIR is empty
    exit /b 1
)
if "!OUT_DIR!"=="" (
    echo ERROR: OUT_DIR is empty
    exit /b 1
)
if not exist "!CAPNP_EXE!" (
    echo ERROR: capnp.exe not found: !CAPNP_EXE!
    exit /b 1
)
if not exist "!CAPNPC_CXX!" (
    echo ERROR: capnpc-c++.exe not found: !CAPNPC_CXX!
    exit /b 1
)

set "PATH=!CAPNP_ROOT!/bin;!PATH!"

if exist "!STAMP_FILE!" if exist "!OUT_DIR!/cell.capnp.cc" (
    echo LStream schemas already present at !OUT_DIR!
    exit /b 0
)

if not exist "!REPO_DIR!/.git" (
    if exist "!REPO_DIR!" (
        echo ERROR: !REPO_DIR! exists but is not a git clone.
        echo Remove it so LStream schemas can be fetched automatically: rmdir /s /q "!REPO_DIR!"
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
    echo ERROR: unsupported VERSION_MODE=!VERSION_MODE!
    exit /b 1
)

set CURRENT_REV=
if exist "!STATE_FILE!" (
    set /p CURRENT_REV=<"!STATE_FILE!"
)

if "!CURRENT_REV!"=="!TARGET_REV!" if exist "!STAMP_FILE!" (
    echo LStream schemas already up to date: !TARGET_REV!
    exit /b 0
)

git reset --hard !TARGET_REV!
if errorlevel 1 exit /b 1

if not exist "!OUT_DIR!" mkdir "!OUT_DIR!"

copy /Y "!REPO_DIR!/capnp/*.capnp" "!OUT_DIR!/"
if errorlevel 1 exit /b 1

cd /d "!OUT_DIR!"

for %%f in (*.capnp) do (
    echo Generating %%f
    "!CAPNP_EXE!" compile -I . -o c++ "%%f"
    if errorlevel 1 exit /b 1
)

for %%f in (*.capnp.c++) do (
    if exist "%%~ff" ren "%%~ff" "%%~nf.cc"
)

echo !TARGET_REV!>"!STATE_FILE!"
echo built>"!STAMP_FILE!"

echo LStream schemas updated successfully.
exit /b 0

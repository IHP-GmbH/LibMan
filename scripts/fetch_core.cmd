@echo off
rem Fetch and build CORE (CommonDB) for qmake builds. Run from repo root.
setlocal EnableExtensions
set "ROOT=%~dp0.."
for %%I in ("%ROOT%") do set "ROOT=%%~fI"
set "CAPNP_ROOT=%ROOT%\capnp-install"
set "CORE_BUILD=%ROOT%\.deps\core-build"
set "STAMP=%CORE_BUILD%\libman_core_built.stamp"

if exist "%STAMP%" exit /b 0

if defined LIBMAN_CORE_SOURCE_DIR (
    if exist "%LIBMAN_CORE_SOURCE_DIR%" (
        set "CORE_SRC=%LIBMAN_CORE_SOURCE_DIR%"
        echo Using local CORE tree: %CORE_SRC%
        goto :configure
    )
)

set "CORE_SRC=%ROOT%\.deps\CommonDB"
if not exist "%CORE_SRC%\.git" (
    if defined LIBMAN_CORE_GIT_TOKEN (
        set "CLONE_URL=https://x-access-token:%LIBMAN_CORE_GIT_TOKEN%@github.com/IHP-GmbH/CommonDB.git"
    ) else if defined GITHUB_TOKEN (
        set "CLONE_URL=https://x-access-token:%GITHUB_TOKEN%@github.com/IHP-GmbH/CommonDB.git"
    ) else (
        set "CLONE_URL=https://github.com/IHP-GmbH/CommonDB.git"
    )
    echo Cloning CORE from GitHub...
    git clone --depth 1 --branch main "%CLONE_URL%" "%CORE_SRC%"
    if errorlevel 1 exit /b 1
)

:configure
if not exist "%CAPNP_ROOT%\include\capnp\message.h" (
    echo ERROR: Cap'n Proto not found in %CAPNP_ROOT%. Run mkcapnp.cmd first.
    exit /b 1
)

cmake -S "%CORE_SRC%" -B "%CORE_BUILD%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCORE_BOOTSTRAP_CAPNP=OFF ^
    -DCAPNP_ROOT="%CAPNP_ROOT%" ^
    -DCORE_BUILD_TESTS=OFF ^
    -DCORE_BUILD_OAS_TESTS=OFF ^
    -DCORE_BUILD_EXAMPLES=OFF ^
    -G "MinGW Makefiles"
if errorlevel 1 exit /b 1

cmake --build "%CORE_BUILD%" --target core core_utils -j
if errorlevel 1 exit /b 1

type nul > "%STAMP%"
echo CORE built in %CORE_BUILD%
exit /b 0

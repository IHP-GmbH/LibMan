@echo off
setlocal enabledelayedexpansion

set GIT_URL=%~1
set VERSION_MODE=%~2
set GIT_BRANCH=%~3
set GIT_TAG=%~4
set GIT_COMMIT=%~5
set REPO_DIR=%~6
set INSTALL_DIR=%~7

set STAMP_FILE=%INSTALL_DIR%\.built
set STATE_FILE=%INSTALL_DIR%\.capnp_revision

if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"

if not exist "%REPO_DIR%\.git" (
    git clone "%GIT_URL%" "%REPO_DIR%"
    if errorlevel 1 exit /b 1
)

cd /d "%REPO_DIR%"
git fetch --tags origin
if errorlevel 1 exit /b 1

if /I "%VERSION_MODE%"=="branch" (
    for /f %%i in ('git rev-parse origin/%GIT_BRANCH%') do set TARGET_REV=%%i
) else if /I "%VERSION_MODE%"=="tag" (
    for /f %%i in ('git rev-parse refs/tags/%GIT_TAG%') do set TARGET_REV=%%i
) else if /I "%VERSION_MODE%"=="commit" (
    for /f %%i in ('git rev-parse %GIT_COMMIT%') do set TARGET_REV=%%i
) else (
    echo Unsupported CAPNP_VERSION_MODE: %VERSION_MODE%
    exit /b 1
)

set CURRENT_REV=
if exist "%STATE_FILE%" (
    set /p CURRENT_REV=<"%STATE_FILE%"
)

if "%CURRENT_REV%"=="%TARGET_REV%" if exist "%STAMP_FILE%" (
    echo Cap'n Proto is already up to date: %TARGET_REV%
    exit /b 0
)

git reset --hard %TARGET_REV%
if errorlevel 1 exit /b 1

if not exist "%REPO_DIR%\c++\build" mkdir "%REPO_DIR%\c++\build"
cd /d "%REPO_DIR%\c++\build"

cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%"
if errorlevel 1 exit /b 1

mingw32-make -j4
if errorlevel 1 exit /b 1

mingw32-make install
if errorlevel 1 exit /b 1

echo %TARGET_REV%>"%STATE_FILE%"
echo built>"%STAMP_FILE%"

echo Cap'n Proto installed at revision %TARGET_REV%
exit /b 0


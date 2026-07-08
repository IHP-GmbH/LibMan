@echo off
rem Exit 0 if CommonDB (CORE) is available for this build, 1 otherwise.
setlocal EnableExtensions

set "LIBMAN_ROOT=%~1"
if "%LIBMAN_ROOT%"=="" exit /b 1

set "CORE_REPO=IHP-GmbH/CommonDB"

if defined LIBMAN_CORE_SOURCE_DIR (
    if exist "%LIBMAN_CORE_SOURCE_DIR%\src\core_paths.h" exit /b 0
)

if exist "%LIBMAN_ROOT%\.deps\CommonDB\src\core_paths.h" exit /b 0

set "AUTH_HEADER="
if defined LIBMAN_CORE_GIT_TOKEN (
    set "AUTH_HEADER=-H Authorization: Bearer %LIBMAN_CORE_GIT_TOKEN%"
) else if defined GITHUB_TOKEN (
    set "AUTH_HEADER=-H Authorization: Bearer %GITHUB_TOKEN%"
)

for /f %%H in ('curl.exe --connect-timeout 5 --max-time 10 -sS -o nul -w "%%{http_code}" %AUTH_HEADER% https://api.github.com/repos/%CORE_REPO% 2^>nul') do set "HTTP_CODE=%%H"
if "%HTTP_CODE%"=="200" exit /b 0

exit /b 1

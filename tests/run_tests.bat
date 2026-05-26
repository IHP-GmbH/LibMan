@echo off
rem Qt Creator: set Run program to tests\run_tests.bat (working directory = repo root or tests/).
setlocal EnableExtensions EnableDelayedExpansion

for %%I in ("%~dp0..") do set "ROOT=%%~fI"
set "TESTS_BUILD=%~dp0build"
set "BIN=tst_libman_gui.exe"

for %%d in ("debug" "release" ".") do (
    if exist "!TESTS_BUILD!\%%~d\!BIN!" (
        call "!ROOT!\scripts\run_tests.bat" "!TESTS_BUILD!\%%~d"
        exit /b !ERRORLEVEL!
    )
)

for /d %%k in ("!TESTS_BUILD!\Desktop_*") do (
    for %%d in ("debug" "release") do (
        if exist "%%k\%%d\!BIN!" (
            call "!ROOT!\scripts\run_tests.bat" "%%k\%%d"
            exit /b !ERRORLEVEL!
        )
    )
)

call "!ROOT!\scripts\run_tests.bat" "!TESTS_BUILD!"
exit /b !ERRORLEVEL!

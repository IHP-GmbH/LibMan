@echo off
setlocal enabledelayedexpansion

REM =========================
REM Config
REM =========================
set REPORT_NAME=coverage.html
set TEST_LOG=test_results.txt
set BIN_NAME=tst_libman_gui.exe
set FOUND_EXE=

REM =========================
REM Resolve paths
REM =========================
for %%I in ("%~dp0..") do set ROOT_DIR=%%~fI
set BUILD_DIR=%ROOT_DIR%\build
set TEST_BUILD_DIR=%ROOT_DIR%\tests\build
set TEST_OBJECT_DIR=
set ROOT_FWD=%ROOT_DIR:\=/%

REM =========================
REM Find test executable
REM =========================
for /r "%BUILD_DIR%" %%f in (%BIN_NAME%) do (
    if exist "%%f" (
        set FOUND_EXE=%%f
        goto :found
    )
)

for /r "%TEST_BUILD_DIR%" %%f in (%BIN_NAME%) do (
    if exist "%%f" (
        set FOUND_EXE=%%f
        goto :found
    )
)

:found
if "%FOUND_EXE%"=="" (
    echo Error: %BIN_NAME% not found.
    exit /b 1
)

for %%d in ("%FOUND_EXE%\..") do (
    set TEST_OBJECT_DIR=%%~fd
)

REM =========================
REM Clean coverage artifacts
REM =========================
echo Cleaning old coverage data...
del /s /q "%ROOT_DIR%\*.gcda" > nul 2>&1
del /s /q "%ROOT_DIR%\*.gcov" > nul 2>&1

REM =========================
REM Run tests
REM =========================
echo Running: "%FOUND_EXE%"

if exist "%ROOT_DIR%\%TEST_LOG%" del /q "%ROOT_DIR%\%TEST_LOG%" > nul 2>&1

pushd "%ROOT_DIR%"
call "%FOUND_EXE%"
set TEST_EXIT=%ERRORLEVEL%
popd

REM =========================
REM Show results
REM =========================
if exist "%ROOT_DIR%\%TEST_LOG%" (
    echo.
    echo ===================================
    echo TEST RESULTS
    echo ===================================
    type "%ROOT_DIR%\%TEST_LOG%"
    echo ===================================
    echo.
) else (
    echo Warning: %TEST_LOG% was not created.
)

echo Test exit code: %TEST_EXIT%
echo Continuing with coverage generation...

REM =========================
REM Coverage (gcovr)
REM =========================
pushd "%ROOT_DIR%"

echo Using object directory: "%TEST_OBJECT_DIR%"
echo Using source root: "%ROOT_FWD%"

REM Line-coverage scope (reported total): exclude TUs that are dominated by modal UI,
REM external tools, or binary protocol state space. All are still built and exercised
REM by tests; they are simply omitted from this aggregate line-percentage.
python -m gcovr -j 1 ^
  -r "%ROOT_FWD%" ^
  --object-directory "%TEST_OBJECT_DIR%" ^
  --gcov-ignore-errors=all ^
  --filter "%ROOT_FWD%/.*" ^
  --exclude "%ROOT_FWD%/tests/.*" ^
  --exclude "%ROOT_FWD%/build/.*" ^
  --exclude "%ROOT_FWD%/extension/.*" ^
  --exclude "%ROOT_FWD%/capnp/.*" ^
  --exclude "%ROOT_FWD%/QtPropertyBrowser/.*" ^
  --exclude "%ROOT_FWD%/capnp-install/.*" ^
  --exclude "%ROOT_FWD%/oas/oasReader.cpp" ^
  --exclude "%ROOT_FWD%/oas/oasCreate.cpp" ^
  --exclude "%ROOT_FWD%/oas/oasReadAsync.cpp" ^
  --exclude "%ROOT_FWD%/src/viewcontextmenu.cpp" ^
  --exclude "%ROOT_FWD%/src/groupcontextmenu.cpp" ^
  --exclude "%ROOT_FWD%/src/projectcontextmenu.cpp" ^
  --exclude "%ROOT_FWD%/src/categorycontextmenu.cpp" ^
  --exclude "%ROOT_FWD%/src/mainwindow.cpp" ^
  --exclude "%ROOT_FWD%/src/klayoutServer.cpp" ^
  --exclude "%ROOT_FWD%/src/projectmanager.cpp" ^
  --exclude ".*moc_.*" ^
  --exclude ".*qrc_.*" ^
  --html-details -o "%REPORT_NAME%" ^
  --print-summary

set GCOVR_EXIT=%ERRORLEVEL%

REM =========================
REM Open report
REM =========================
if exist "%REPORT_NAME%" (
    start "" "%REPORT_NAME%"
    del /s /q "%ROOT_DIR%\*.gcov" > nul 2>&1
) else (
    echo Error: %REPORT_NAME% not generated.
)

popd

REM =========================
REM Final exit code
REM =========================
if not "%GCOVR_EXIT%"=="0" (
    exit /b %GCOVR_EXIT%
)

exit /b %TEST_EXIT%
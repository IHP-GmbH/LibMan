@echo off
setlocal enabledelayedexpansion

REM =========================
REM Config
REM =========================
set REPORT_NAME=coverage.html
set TEST_LOG=test_results.txt
set BIN_NAME=tst_libman_gui.exe
set FOUND_EXE=

REM Optional: run_tests.bat [path\to\build\dir] or set TEST_BUILD_DIR / BUILD_DIR
if not "%~1"=="" (
    if exist "%~1\%BIN_NAME%" (
        set FOUND_EXE=%~1\%BIN_NAME%
        goto :found
    )
    if exist "%~1" (
        set TEST_BUILD_DIR=%~1
    ) else (
        set TEST_BUILD_DIR=%~1
    )
)

REM =========================
REM Resolve paths
REM =========================
for %%I in ("%~dp0..") do set ROOT_DIR=%%~fI
if not defined BUILD_DIR set BUILD_DIR=%ROOT_DIR%\build
if not defined TEST_BUILD_DIR set TEST_BUILD_DIR=%ROOT_DIR%\build-tests
set LEGACY_TEST_BUILD_DIR=%ROOT_DIR%\tests\build
set TEST_OBJECT_DIR=
set ROOT_FWD=%ROOT_DIR:\=/=%

REM =========================
REM Find test executable
REM =========================
if not "%FOUND_EXE%"=="" goto :found

call :find_in_dir "%TEST_BUILD_DIR%"
if not "%FOUND_EXE%"=="" goto :found
call :find_in_dir "%BUILD_DIR%"
if not "%FOUND_EXE%"=="" goto :found
call :find_in_dir "%LEGACY_TEST_BUILD_DIR%"
if not "%FOUND_EXE%"=="" goto :found

REM Qt Creator shadow builds: build/Desktop_Qt_*-Debug/debug/tst_libman_gui.exe
for /d %%d in ("%BUILD_DIR%\*" "%TEST_BUILD_DIR%\*") do (
    call :find_in_dir "%%~d"
    if not "!FOUND_EXE!"=="" goto :found
)

REM Last resort: search repo (skip heavy dependency trees)
for /f "delims=" %%f in ('dir /s /b "%ROOT_DIR%\%BIN_NAME%" 2^>nul') do (
    echo %%f | findstr /i /c:"\capnproto\" /c:"\capnp-install\" /c:"\.deps\" /c:"\capnproto\" >nul
    if errorlevel 1 (
        set FOUND_EXE=%%f
        goto :found
    )
)

:found
if "%FOUND_EXE%"=="" (
    echo Error: %BIN_NAME% not found.
    echo.
    echo Searched under:
    echo   %TEST_BUILD_DIR%
    echo   %BUILD_DIR%  (including Qt Creator shadow subfolders)
    echo   %LEGACY_TEST_BUILD_DIR%
    echo.
    echo Build tests first:
    echo   Qt Creator: open tests/tests.pro, Run qmake, then Build Project
    echo   VS Code:    Ctrl+Shift+B - "Tests: Build Tests"
    echo   CLI:        mkdir build-tests ^& cd build-tests ^& qmake ../tests/tests.pro ^& mingw32-make
    echo.
    echo Or pass the build directory:  scripts\run_tests.bat path\to\dir\with\%BIN_NAME%
    exit /b 1
)

for %%d in ("%FOUND_EXE%\..") do (
    set TEST_OBJECT_DIR=%%~fd
)

echo Using test binary: %FOUND_EXE%

set TESTDATA_SRC=%ROOT_DIR%\tests\data
set LIBMAN_TEST_DATA_DIR=%TESTDATA_SRC%

REM =========================
REM Sync fixtures next to the test binary
REM =========================
if exist "%TESTDATA_SRC%" (
    echo Syncing test fixtures to "%TEST_OBJECT_DIR%\data"...
    if not exist "%TEST_OBJECT_DIR%\data" mkdir "%TEST_OBJECT_DIR%\data"
    xcopy /E /I /Y "%TESTDATA_SRC%\*" "%TEST_OBJECT_DIR%\data\" > nul
) else (
    echo Warning: test data source not found: %TESTDATA_SRC%
)

REM =========================
REM Clean coverage artifacts (object dir only)
REM =========================
echo Cleaning old coverage data in "%TEST_OBJECT_DIR%"...
del /s /q "%TEST_OBJECT_DIR%\*.gcda" > nul 2>&1
del /s /q "%TEST_OBJECT_DIR%\*.gcov" > nul 2>&1

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

if not "%TEST_EXIT%"=="0" (
    echo Tests reported %TEST_EXIT% failure^(s^) - see per-suite [FAIL] lines above.
) else (
    echo All tests passed.
)

echo Generating coverage report...

REM =========================
REM Coverage (gcovr)
REM =========================
pushd "%ROOT_DIR%"

echo Using object directory: "%TEST_OBJECT_DIR%"
echo Using source root: "%ROOT_FWD%"

python -m gcovr -j 1 ^
  -r "%ROOT_FWD%" ^
  --object-directory "%TEST_OBJECT_DIR%" ^
  --gcov-ignore-errors=all ^
  --filter "%ROOT_FWD%/.*" ^
  --exclude "%ROOT_FWD%/tests/.*" ^
  --exclude "%ROOT_FWD%/build/.*" ^
  --exclude "%ROOT_FWD%/build-tests/.*" ^
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

if exist "%REPORT_NAME%" (
    start "" "%REPORT_NAME%"
    del /s /q "%ROOT_DIR%\*.gcov" > nul 2>&1
) else (
    echo Error: %REPORT_NAME% not generated.
)

popd

echo.
echo Summary: test failures=%TEST_EXIT%, gcovr exit=%GCOVR_EXIT%

if not "%GCOVR_EXIT%"=="0" (
    echo Error: gcovr failed.
    exit /b %GCOVR_EXIT%
)

if not "%TEST_EXIT%"=="0" (
    echo Error: one or more tests failed.
    exit /b 1
)

exit /b 0

REM =========================
:find_in_dir
set "_SEARCH_DIR=%~1"
if not exist "%_SEARCH_DIR%" exit /b 0
for /r "%_SEARCH_DIR%" %%f in (%BIN_NAME%) do (
    if exist "%%f" (
        set FOUND_EXE=%%f
        exit /b 0
    )
)
exit /b 0

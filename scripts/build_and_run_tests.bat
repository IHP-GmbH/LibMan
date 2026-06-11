@echo off
setlocal enabledelayedexpansion

for %%I in ("%~dp0..") do set ROOT=%%~fI
set "PATH=C:\Qt\5.15.2\mingw81_64\bin;C:\Qt\Tools\mingw810_64\bin;%PATH%"

taskkill /IM tst_libman_gui.exe /F >nul 2>&1 || exit /b 0

pushd "%ROOT%\tests"

if not exist build\CMakeCache.txt (
    echo Configuring tests with coverage...
    cmake -S . -B build -G "MinGW Makefiles" ^
        -DCMAKE_BUILD_TYPE=Debug ^
        -DLIBMAN_ENABLE_COVERAGE=ON ^
        -DCMAKE_C_COMPILER=C:/Qt/Tools/mingw810_64/bin/gcc.exe ^
        -DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw810_64/bin/g++.exe ^
        -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/mingw810_64/bin/mingw32-make.exe ^
        -DCMAKE_PREFIX_PATH=C:/Qt/5.15.2/mingw81_64 ^
        -DQt5_DIR=C:/Qt/5.15.2/mingw81_64/lib/cmake/Qt5
    if errorlevel 1 (
        popd
        exit /b 1
    )
)

cmake --build build -j4
if errorlevel 1 (
    popd
    exit /b 1
)

popd

call "%ROOT%\scripts\run_tests.bat" "%ROOT%\tests\build"
exit /b %ERRORLEVEL%

@echo off
setlocal
if "%~2"=="" exit /b 0
if not exist "%~1" exit /b 0
copy /Y "%~1" "%~2" >nul 2>&1
exit /b 0

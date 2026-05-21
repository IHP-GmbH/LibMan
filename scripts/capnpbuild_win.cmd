@echo off
rem Wrapper for mingw32-make: pass only LIBMAN_ROOT (forward slashes). Avoids cmd/bash
rem mangling capnproto in long command lines (%%c, \c escapes).
setlocal EnableExtensions
set "ROOT=%~1"
if "%ROOT%"=="" set "ROOT=%~dp0.."
set "ROOT=%ROOT:\=/%"
call "%~dp0build_capnp_windows.bat" ^
  "https://github.com/capnproto/capnproto.git" branch master "" "" ^
  "%ROOT%/capnproto" "%ROOT%/capnp-install"
exit /b %ERRORLEVEL%

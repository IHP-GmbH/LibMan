@echo off
rem Invoked from repo root (CI) or build/ via: cd .. && cmd /c scripts/mkcapnp.cmd
rem Filename avoids "\c" in "\capnp..." when mingw32-make runs recipes under bash.
setlocal EnableExtensions
set "ROOT=%~dp0.."
set "ROOT=%ROOT:\=/%"
call "%~dp0build_capnp_windows.bat" ^
  "https://github.com/capnproto/capnproto.git" branch master "" "" ^
  "%ROOT%/capnproto" "%ROOT%/capnp-install"
exit /b %ERRORLEVEL%

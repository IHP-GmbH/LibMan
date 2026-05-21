@echo off
setlocal EnableExtensions
set "ROOT=%~dp0.."
set "ROOT=%ROOT:\=/%"
call "%~dp0update_lstream_schemas_windows.bat" ^
  "https://codeberg.org/klayoutmatthias/lstream.git" branch main "" "" ^
  "%ROOT%/.deps/lstream" "%ROOT%/capnp" "%ROOT%/capnp-install"
exit /b %ERRORLEVEL%

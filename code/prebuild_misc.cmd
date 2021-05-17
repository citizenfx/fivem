@echo off
setlocal
:: TODO: make mojo wrappers that use this from deps
for %%i in (%~dp0\tools\build\deps\*.whl) do python -m easy_install "%%i"
set PYTHONPATH=%~dp0\tools\idl\deps\
mkdir "%~dp0\tools\idl\deps"
for %%i in (%~dp0\tools\build\deps\*.whl) do python -m easy_install -d "%~dp0\tools\idl\deps" "%%i"
endlocal

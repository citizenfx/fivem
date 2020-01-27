@echo off
setlocal
:: TODO: make mojo wrappers that use this from deps
python -m easy_install jinja2 ply
set PYTHONPATH=code\tools\idl\deps\
mkdir code\tools\idl\deps
python -m easy_install -d code\tools\idl\deps\ ply jinja2
endlocal

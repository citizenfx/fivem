@echo off
setlocal
:: TODO: make mojo wrappers that use this from deps
python -m easy_install jinja2==2.11.1 ply MarkupSafe==1.1.1
set PYTHONPATH=code\tools\idl\deps\
mkdir code\tools\idl\deps
python -m easy_install -d code\tools\idl\deps\ ply jinja2==2.11.1 MarkupSafe==1.1.1
endlocal

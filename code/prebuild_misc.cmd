@echo off
setlocal
:: TODO: make mojo wrappers that use this from deps
python -m easy_install  MarkupSafe==1.1.1 jinja2==2.11.1 ply
set PYTHONPATH=code\tools\idl\deps\
mkdir code\tools\idl\deps
python -m easy_install -d code\tools\idl\deps\ MarkupSafe==1.1.1 ply jinja2==2.11.1
endlocal

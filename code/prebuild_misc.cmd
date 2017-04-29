@echo off
setlocal
set PYTHONPATH=code\tools\idl\deps\
mkdir code\tools\idl\deps
python -m easy_install -d code\tools\idl\deps\ ply
endlocal

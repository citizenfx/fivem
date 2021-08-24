@echo off
py -m pip install Jinja2 MarkupSafe ply

set PYTHONPATH=code\tools\idl\deps\
mkdir code\tools\idl\deps
py -m pip install Jinja2 MarkupSafe ply

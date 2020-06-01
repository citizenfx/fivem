@echo off
set ROOT=%~dp0

powershell -ExecutionPolicy Bypass %ROOT%\run_postbuild.ps1 %*
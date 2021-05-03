@echo off
where /q pwsh

if errorlevel 1 (
    echo You need to have PowerShell 7 installed to run the fxd utility.
    exit /B 0
)

pwsh .\fxd.ps1 %*
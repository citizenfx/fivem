@echo off

where /q cargo

if errorlevel 1 (
    exit /B 0
)

pushd %~dp0\..\..\..\ext\jexl-eval
cargo build --release
popd
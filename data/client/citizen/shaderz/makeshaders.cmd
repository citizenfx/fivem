@echo off

setlocal enableextensions enabledelayedexpansion

set MAKESHADERS_SCRIPT_DIR=%~dp0

for /f %%I in (preloadcfx.list) do (
	if %%~xI==.fx (
		echo ==== %%I ====
		call %MAKESHADERS_SCRIPT_DIR%makeshader.cmd %%I %*
		if errorlevel 1 goto fail
	)
)
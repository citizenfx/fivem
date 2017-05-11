@echo off
call code\prebuild_udis86.cmd
call code\prebuild_misc.cmd

cd %~dp0

call prebuild_natives.cmd
@echo off
pushd vendor\luajit\src
call msvcbuild static

call "%VCINSTALLDIR%\vcvarsall.bat" amd64
call msvcbuild static
popd

@echo off

set find=_find

pushd %~dp0
mkdir ..\data\client\citizen\locales
del ..\data\client\citizen\locales\cfx.pot

%find% client shared components -type f -iname "*.cpp" -o -iname "*.c" -o -iname "*.h" > %temp%\pot.lst
xgettext -f "%temp%\pot.lst" --from-code utf-8 --no-wrap -C -d cfx -s -kgettext -o ../data/client/citizen/locales/cfx.pot
del "%temp%\pot.lst"

popd
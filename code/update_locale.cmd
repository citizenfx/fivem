@echo off

set find=_find

mkdir ..\data\client\citizen\locales
%find% client shared components -type f -iname "*.cpp" -o -iname "*.c" -o -iname "*.h" | xargs xgettext --from-code utf-8 --no-wrap -C -d cfx -s -kgettext -o ../data/client/citizen/locales/cfx.pot 
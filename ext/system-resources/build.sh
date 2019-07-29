#!/bin/sh
set -e

cd ../webadmin/server/
dotnet publish -c Release
cd ../../system-resources/

mkdir -p data/webadmin/wwwroot/
mkdir -p data/webadmin/server/bin/Release/netstandard2.0/publish/
cp -a ../webadmin/__resource.lua data/webadmin/

cp -a ../webadmin/wwwroot/ data/webadmin/
cp -a ../webadmin/server/bin/Release/netstandard2.0/publish/ data/webadmin/server/bin/Release/netstandard2.0/
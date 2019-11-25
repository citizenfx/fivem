#!/bin/sh
set -e

cd ../webadmin/server/
dotnet publish -c Release
cd ../../system-resources/

cd ../monitor/server/
dotnet publish -c Release
cd ../../system-resources/

mkdir -p data/webadmin/wwwroot/
mkdir -p data/webadmin/server/bin/Release/netstandard2.0/publish/
cp -a ../webadmin/fxmanifest.lua data/webadmin/

cp -a ../webadmin/wwwroot/ data/webadmin/
cp -a ../webadmin/server/bin/Release/netstandard2.0/publish/ data/webadmin/server/bin/Release/netstandard2.0/

mkdir -p data/monitor/server/bin/Release/netstandard2.0/publish/
cp -a ../monitor/fxmanifest.lua data/monitor/

cp -a ../monitor/server/bin/Release/netstandard2.0/publish/ data/monitor/server/bin/Release/netstandard2.0/
#!/bin/sh
set -e

apk add --no-cache nodejs npm

cd ../webadmin/server/
dotnet publish -c Release
cd ../../system-resources/

cd ../txAdmin/
npm i
./node_modules/.bin/webpack --config webpack.config.js --progress
cd ../system-resources/

mkdir -p data/webadmin/wwwroot/
mkdir -p data/webadmin/server/bin/Release/netstandard2.0/publish/
cp -a ../webadmin/fxmanifest.lua data/webadmin/

cp -a ../webadmin/wwwroot/ data/webadmin/
cp -a ../webadmin/server/bin/Release/netstandard2.0/publish/ data/webadmin/server/bin/Release/netstandard2.0/

cp -a ../txAdmin/dist data/monitor
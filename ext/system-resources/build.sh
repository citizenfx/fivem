#!/bin/sh
set -e

apk add --no-cache nodejs npm

cd ../txAdmin/
npm ci
./node_modules/.bin/webpack --config webpack.config.js --progress
cd ../system-resources/

cp -a ../txAdmin/dist data/monitor
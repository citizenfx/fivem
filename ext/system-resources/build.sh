#!/bin/sh
set -e

apk add --no-cache nodejs npm
npm install -g npm@7.8.0

mkdir -p data

cd ../txAdmin/
npm ci
npm run build
cd ../system-resources/

cp -a ../txAdmin/dist data/monitor
#!/bin/sh
set -e

apk add --no-cache nodejs npm
npm install -g npm@7.19.1

mkdir -p data

# txAdmin
cd ../txAdmin/
npm ci
npm run build
cd ../system-resources/

cp -a ../txAdmin/dist data/monitor

# chat
cd resources/chat/

npm install yarn@1.22
node_modules/.bin/yarn
NODE_OPTIONS=--openssl-legacy-provider node_modules/.bin/webpack
cd ../../

rm -rf resources/chat/node_modules/
cp -a resources/chat data/chat
rm -rf data/chat/package.json data/chat/yarn.lock
rm -rf data/chat/html/
mkdir data/chat/html/
cp -a resources/chat/html/vendor data/chat/html/vendor

#!/bin/sh
set -e

SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

apk add unzip

ROOT=$(pwd)

unzip $ROOT/out/server.zip -d $ROOT/out/server/

cd $SCRIPTPATH
echo '//registry.npmjs.org/:_authToken=${NPM_TOKEN}' > .npmrc

npm config set git-tag-version false

cp -a $ROOT/out/server/citizen/scripting/v8/index.d.ts $ROOT/out/server/citizen/scripting/v8/natives_server.d.ts .
sed -i 's/natives_universal\.d\.ts/natives_server.d.ts/g' index.d.ts
npm version "2.0.${CI_PIPELINE_ID}-1"
npm publish

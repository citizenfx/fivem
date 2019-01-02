#!/bin/sh
set -e

SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

apk add unzip

PWD=$(pwd)

unzip $PWD/out/server.zip -d $PWD/out/server/

cd $SCRIPTPATH
echo '//registry.npmjs.org/:_authToken=${NPM_TOKEN}' > .npmrc

npm config set git-tag-version false

cp -a $PWD/out/server/citizen/scripting/v8/index.d.ts $PWD/out/server/citizen/scripting/v8/natives_server.d.ts .
npm version "1.0.${CI_PIPELINE_ID}-1"
npm publish

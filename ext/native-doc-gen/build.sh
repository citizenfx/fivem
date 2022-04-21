#!/bin/sh

set -xe

ROOT=$(pwd)
LUA53=lua5.3
NODE=node
YARN=yarn

[ "$OS" == "Windows_NT" ] && LUA53=./lua53
[ "$OS" == "Windows_NT" ] && NODE=$ROOT/node
[ "$OS" == "Windows_NT" ] && YARN="$NODE $ROOT/yarn_cli.js"

[ "$OS" == "Windows_NT" ] && curl -z node.exe -Lo node.exe https://nodejs.org/dist/v18.0.0/win-x64/node.exe && curl -z yarn_cli.js -Lo yarn_cli.js https://github.com/yarnpkg/yarn/releases/download/v1.22.18/yarn-1.22.18.js

# install yarn deps
cd $ROOT/../native-doc-tooling/

[ "$OS" == "Windows_NT" ] && $YARN global add node-gyp
$YARN

cd $ROOT/../natives/

NATIVES_MD_DIR=$ROOT/../native-decls/native_md/ $LUA53 codegen.lua inp/natives_global.lua markdown server rpc

# make out dir
cd $ROOT
mkdir out || true

# enter out dir
cd out

# setup clang and build
[ "$OS" == "Windows_NT" ] && cp -a $ROOT/libclang.dll $PWD/libclang.dll || true
$NODE $ROOT/../native-doc-tooling/index.js $ROOT/../native-decls/

mkdir -p $ROOT/../natives/inp/ || true

NODE_PATH=$ROOT/../native-doc-tooling/node_modules/ $NODE $ROOT/../native-doc-tooling/build-template.js lua CFX > $ROOT/../natives/inp/natives_cfx_new.lua
rm $PWD/libclang.dll || true

# copy outputs
cd $ROOT
cp -a out/natives_test.json natives_cfx.json

# copy new
if [ -e $ROOT/../natives/inp/natives_cfx.lua ]; then
	if ! diff -q $ROOT/../natives/inp/natives_cfx_new.lua $ROOT/../natives/inp/natives_cfx.lua 2>&1 > /dev/null; then
		cp -a $ROOT/../natives/inp/natives_cfx_new.lua $ROOT/../natives/inp/natives_cfx.lua
	fi
else
    cp -a $ROOT/../natives/inp/natives_cfx_new.lua $ROOT/../natives/inp/natives_cfx.lua
fi
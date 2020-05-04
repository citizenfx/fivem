#!/bin/sh

set -xe

ROOT=$(pwd)

# install yarn deps
cd $ROOT/../native-doc-tooling/
yarn

# make out dir
cd $ROOT
mkdir out || true

# enter out dir
cd out

# setup clang and build
[ "x$OS" == "xWindows_NT" ] && cp -a $ROOT/libclang.dll $PWD/libclang.dll || true
node $ROOT/../native-doc-tooling/index.js $ROOT/../native-decls/

mkdir -p $ROOT/../natives/inp/ || true

NODE_PATH=$ROOT/../native-doc-tooling/node_modules/ node $ROOT/../native-doc-tooling/build-template.js lua CFX > $ROOT/../natives/inp/natives_cfx_new.lua
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
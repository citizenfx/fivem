#!/bin/sh

set -xe

ROOT=$(pwd)
LUA53=lua5.3
NODE=node
YARN=yarn

[ "$OS" == "Windows_NT" ] && LUA53=./lua53
[ "$OS" == "Windows_NT" ] && NODE=$ROOT/node
[ "$OS" != "Windows_NT" ] && NODE=/tmp/node/node/bin/node
YARN="$NODE $ROOT/yarn_cli.js --mutex network"

[ "$OS" == "Windows_NT" ] && curl -z node.exe -Lo node.exe https://content.cfx.re/mirrors/vendor/node/v12.22.12/node.exe --http1.1
[ "$OS" != "Windows_NT" ] && mkdir /tmp/node && \
	curl -Lo /tmp/node/node.tar.gz https://content.cfx.re/mirrors/vendor/node/v12.22.12/node-v12.22.12-linux-x64-musl.tar.gz && \
	tar -C /tmp/node -xf /tmp/node/node.tar.gz && \
	mv /tmp/node/node-* /tmp/node/node

# install yarn deps
cd $ROOT/../native-doc-tooling/

$YARN global add node-gyp@9.3.1
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

[ "$OS" != "Windows_NT" ] && rm -rf /tmp/node || true

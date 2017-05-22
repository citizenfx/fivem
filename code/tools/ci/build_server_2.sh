#!/bin/sh

# fail on error
set -e

# add testing repository
echo http://dl-cdn.alpinelinux.org/alpine/edge/testing >> /etc/apk/repositories

# update apk cache
apk --no-cache update
apk --no-cache upgrade

# install runtime dependencies
apk add libc++ curl libssl1.0 libunwind libstdc++

# install compile-time dependencies
apk add --no-cache --virtual .dev-deps libc++-dev curl-dev clang clang-dev build-base linux-headers openssl-dev python2 py2-pip lua5.3 lua5.3-dev

# install ply
pip install ply

# download and build premake
curl -sLo /tmp/premake.zip https://github.com/premake/premake-core/releases/download/v5.0.0-alpha11/premake-5.0.0-alpha11-src.zip

cd /tmp
unzip -q premake.zip
rm premake.zip
cd premake-*

cd build/gmake.unix/
make -j4
cd ../../

mv bin/release/premake5 /usr/local/bin
cd ..

rm -rf premake-*

# download and extract boost
curl -sLo /tmp/boost.tar.bz2 https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.bz2
tar xf boost.tar.bz2
rm boost.tar.bz2

mv boost_* boost

export BOOST_ROOT=/tmp/boost/

# build CitizenFX
cd /src/code

cp -a ../vendor/curl/include/curl/curlbuild.h.dist ../vendor/curl/include/curl/curlbuild.h

premake5 gmake --game=server --cc=clang
cd build/server/linux

export CXXFLAGS="-std=c++1z -stdlib=libc++"

make clean
make clean config=release
make -j4 config=release

cd ../../../

# build an output tree
mkdir -p /opt/cfx-server

cp -a ../data/shared/* /opt/cfx-server
cp -a ../data/server/* /opt/cfx-server
cp -a bin/server/linux/release/FXServer /opt/cfx-server
cp -a bin/server/linux/release/*.so /opt/cfx-server
cp -a bin/server/linux/release/*.json /opt/cfx-server

# strip output files
strip --strip-unneeded /opt/cfx-server/*.so
strip --strip-unneeded /opt/cfx-server/FXServer

cd /src/ext/natives
gcc -O2 -shared -fpic -o cfx.so -I/usr/include/lua5.3/ lua_cfx.c

lua5.3 codegen.lua > /opt/cfx-server/citizen/scripting/lua/natives_server.lua

cd /opt/cfx-server

# clean up
rm -rf /tmp/boost

apk del .dev-deps

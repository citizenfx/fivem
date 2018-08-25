#!/bin/sh

# fail on error
set -e

# add testing repository
echo http://dl-cdn.alpinelinux.org/alpine/edge/testing >> /etc/apk/repositories

# update apk cache
apk --no-cache update
apk --no-cache upgrade

# install runtime dependencies
apk add --no-cache libc++ curl libssl1.0 libunwind libstdc++ zlib c-ares icu-libs

# add fivem repositories
curl -sLo /etc/apk/keys/peachypies@protonmail.ch-5adb3818.rsa.pub https://runtime.fivem.net/client/alpine/peachypies@protonmail.ch-5adb3818.rsa.pub

echo https://runtime.fivem.net/client/alpine/builds >> /etc/apk/repositories
apk --no-cache update

# install fivem v8
apk add v8

# install compile-time dependencies
apk add --no-cache --virtual .dev-deps libc++-dev curl-dev clang clang-dev build-base linux-headers openssl-dev python2 py2-pip lua5.3 lua5.3-dev mono-dev c-ares-dev v8-dev

# install ply
pip install ply

# download and build premake
curl -sLo /tmp/premake.zip https://github.com/premake/premake-core/releases/download/v5.0.0-alpha12/premake-5.0.0-alpha12-src.zip

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

# build natives
cd /src/ext/natives
gcc -O2 -shared -fpic -o cfx.so -I/usr/include/lua5.3/ lua_cfx.c

mkdir -p /opt/cfx-server/citizen/scripting/lua/
mkdir -p /opt/cfx-server/citizen/scripting/v8/

lua5.3 codegen.lua natives_stash/gta_universal.lua lua server > /opt/cfx-server/citizen/scripting/lua/natives_server.lua
lua5.3 codegen.lua natives_stash/gta_universal.lua js server > /opt/cfx-server/citizen/scripting/v8/natives_server.js
lua5.3 codegen.lua natives_stash/gta_universal.lua dts server > /opt/cfx-server/citizen/scripting/v8/natives_server.d.ts


cat > /src/code/client/clrcore/NativesServer.cs << EOF
#if IS_FXSERVER
namespace CitizenFX.Core.Native
{
EOF

lua5.3 codegen.lua natives_stash/gta_universal.lua enum server >> /src/code/client/clrcore/NativesServer.cs
lua5.3 codegen.lua natives_stash/gta_universal.lua cs server >> /src/code/client/clrcore/NativesServer.cs

cat >> /src/code/client/clrcore/NativesServer.cs << EOF
}
#endif
EOF

lua5.3 codegen.lua natives_stash/gta_universal.lua rpc server > /opt/cfx-server/citizen/scripting/rpc_natives.json

# build CitizenFX
cd /src/code

premake5 gmake --game=server --cc=clang --dotnet=msnet
cd build/server/linux

export CXXFLAGS="-std=c++1z -stdlib=libc++"

if [ "$CI" = true ] && [ "$TRAVIS" = true ]; then
	echo '#pragma once' > /src/code/shared/cfx_version.h
	echo '#define GIT_DESCRIPTION "'$TRAVIS_BRANCH' '$TRAVIS_BUILD_NUMBER' linux"' >> /src/code/shared/cfx_version.h
fi

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
cp tools/ci/run.sh /opt/cfx-server
chmod +x /opt/cfx-server/run.sh

mkdir -p /opt/cfx-server/citizen/clr2/cfg/mono/4.5/
mkdir -p /opt/cfx-server/citizen/clr2/lib/mono/4.5/

cp -a /etc/mono/4.5/machine.config /opt/cfx-server/citizen/clr2/cfg/mono/4.5/machine.config

cp -a bin/server/linux/release/citizen/ /opt/cfx-server
cp -a ../data/client/citizen/clr2/lib/mono/4.5/MsgPack.dll /opt/cfx-server/citizen/clr2/lib/mono/4.5/

cp -a /usr/lib/mono/4.5/Facades/ /opt/cfx-server/citizen/clr2/lib/mono/4.5/Facades/

cp -a /usr/lib/libMonoPosixHelper.so /tmp/libMonoPosixHelper.so

for dll in Microsoft.CSharp.dll Mono.CSharp.dll Mono.Posix.dll Mono.Security.dll mscorlib.dll \
	System.Configuration.dll System.Core.dll System.Data.dll System.dll System.EnterpriseServices.dll System.ServiceModel.dll System.Web.dll \
	System.Net.dll System.Net.Http.dll System.Numerics.Vectors.dll System.Runtime.Serialization.dll \
	System.ServiceModel.Internals.dll System.Transactions.dll System.Xml.dll System.Xml.Linq.dll System.ComponentModel.DataAnnotations.dll \
	System.Management.dll; do
	cp /usr/lib/mono/4.5/$dll /opt/cfx-server/citizen/clr2/lib/mono/4.5/
done

# strip output files
strip --strip-unneeded /opt/cfx-server/*.so
strip --strip-unneeded /opt/cfx-server/FXServer

cd /opt/cfx-server

# clean up
rm -rf /tmp/boost

apk del .dev-deps

apk del curl
apk add --no-cache curl=7.59.0-r1 libcurl=7.59.0-r1

mv /tmp/libMonoPosixHelper.so /usr/lib/libMonoPosixHelper.so

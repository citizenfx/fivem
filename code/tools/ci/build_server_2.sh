#!/bin/sh

# fail on error
set -e

# set the number of job slots
JOB_SLOTS=${JOB_SLOTS:-24}

# upgrade to edge
echo http://dl-cdn.alpinelinux.org/alpine/edge/main > /etc/apk/repositories
echo http://dl-cdn.alpinelinux.org/alpine/edge/community >> /etc/apk/repositories
echo http://dl-cdn.alpinelinux.org/alpine/edge/testing >> /etc/apk/repositories

# update apk cache
apk --no-cache update
apk --no-cache upgrade

# add curl so we can curl the key
# also add ca-certificates so we don't lose it when removing curl
apk add --no-cache curl ca-certificates

# add fivem repositories
curl --http1.1 -sLo /etc/apk/keys/peachypies@protonmail.ch-5adb3818.rsa.pub https://runtime.fivem.net/client/alpine/peachypies@protonmail.ch-5adb3818.rsa.pub

echo https://runtime.fivem.net/client/alpine/builds >> /etc/apk/repositories
echo https://runtime.fivem.net/client/alpine/main >> /etc/apk/repositories
echo https://runtime.fivem.net/client/alpine/testing >> /etc/apk/repositories
echo https://runtime.fivem.net/client/alpine/community >> /etc/apk/repositories
apk --no-cache update

# uninstall old curl
apk del curl

# install runtime dependencies
apk add --no-cache curl=7.63.0-r99 libssl1.1 libunwind libstdc++ zlib c-ares icu-libs v8

# install compile-time dependencies
apk add --no-cache --virtual .dev-deps curl-dev=7.63.0-r99 clang clang-dev build-base linux-headers openssl-dev python2 py2-pip lua5.3 lua5.3-dev mono-reference-assemblies=5.16.1.0-r9990 mono-dev=5.16.1.0-r9990 libmono=5.16.1.0-r9990 mono-corlib=5.16.1.0-r9990 mono=5.16.1.0-r9990 mono-reference-assemblies-4.x=5.16.1.0-r9990 mono-reference-assemblies-facades=5.16.1.0-r9990 mono-csc=5.16.1.0-r9990 mono-runtime=5.16.1.0-r9990 c-ares-dev v8-dev

# install ply
pip install ply

# download and build premake
curl --http1.1 -sLo /tmp/premake.zip https://github.com/premake/premake-core/releases/download/v5.0.0-alpha13/premake-5.0.0-alpha13-src.zip

cd /tmp
unzip -q premake.zip
rm premake.zip
cd premake-*

cd build/gmake.unix/
make -j${JOB_SLOTS}
cd ../../

mv bin/release/premake5 /usr/local/bin
cd ..

rm -rf premake-*

# download and extract boost
curl --http1.1 -sLo /tmp/boost.tar.bz2 https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.bz2
tar xf boost.tar.bz2
rm boost.tar.bz2

mv boost_* boost

export BOOST_ROOT=/tmp/boost/

# build natives
cd /src/ext/natives

mkdir -p out
curl --http1.1 -sLo out/natives_global.lua http://runtime.fivem.net/doc/natives.lua

gcc -O2 -shared -fpic -o cfx.so -I/usr/include/lua5.3/ lua_cfx.c

mkdir -p /opt/cfx-server/citizen/scripting/lua/
mkdir -p /opt/cfx-server/citizen/scripting/v8/

lua5.3 codegen.lua out/natives_global.lua native_lua server > /src/code/components/citizen-scripting-lua/include/NativesServer.h
lua5.3 codegen.lua out/natives_global.lua lua server > /opt/cfx-server/citizen/scripting/lua/natives_server.lua
lua5.3 codegen.lua out/natives_global.lua js server > /opt/cfx-server/citizen/scripting/v8/natives_server.js
lua5.3 codegen.lua out/natives_global.lua dts server > /opt/cfx-server/citizen/scripting/v8/natives_server.d.ts


cat > /src/code/client/clrcore/NativesServer.cs << EOF
#if IS_FXSERVER
using ContextType = CitizenFX.Core.fxScriptContext;

namespace CitizenFX.Core.Native
{
EOF

lua5.3 codegen.lua out/natives_global.lua enum server >> /src/code/client/clrcore/NativesServer.cs
lua5.3 codegen.lua out/natives_global.lua cs server >> /src/code/client/clrcore/NativesServer.cs

cat >> /src/code/client/clrcore/NativesServer.cs << EOF
}
#endif
EOF

lua5.3 codegen.lua out/natives_global.lua rpc server > /opt/cfx-server/citizen/scripting/rpc_natives.json

# build CitizenFX
cd /src/code

premake5 gmake2 --game=server --cc=clang --dotnet=msnet
cd build/server/linux

export CXXFLAGS="-std=c++17 -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR -Wno-deprecated-declarations -Wno-invalid-offsetof"

if [ ! -z "$CI_BRANCH" ] && [ ! -z "$CI_BUILD_NUMBER" ]; then
	echo '#pragma once' > /src/code/shared/cfx_version.h
	echo '#define GIT_DESCRIPTION "'$CI_BRANCH' '$CI_BUILD_NUMBER' linux"' >> /src/code/shared/cfx_version.h
	echo '#define GIT_TAG "'$CI_BUILD_NUMBER'"' >> /src/code/shared/cfx_version.h
fi

make clean
make clean config=release
make -j${JOB_SLOTS} config=release

cd ../../../

# build an output tree
mkdir -p /opt/cfx-server

cp -a ../data/shared/* /opt/cfx-server
cp -a ../data/server/* /opt/cfx-server
cp -a ../data/server_linux/* /opt/cfx-server
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
cp -a /usr/lib/libmono-btls-shared.so /tmp/libmono-btls-shared.so

for dll in I18N.CJK.dll I18N.MidEast.dll I18N.Other.dll I18N.Rare.dll I18N.West.dll I18N.dll Microsoft.CSharp.dll Mono.CSharp.dll Mono.Posix.dll Mono.Security.dll System.Collections.Immutable.dll System.ComponentModel.DataAnnotations.dll System.Configuration.dll System.Core.dll System.Data.dll System.Drawing.dll System.EnterpriseServices.dll System.IO.Compression.FileSystem.dll System.IO.Compression.dll System.Management.dll System.Net.Http.WebRequest.dll System.Net.Http.dll System.Net.dll System.Numerics.Vectors.dll System.Numerics.dll System.Reflection.Metadata.dll System.Runtime.InteropServices.RuntimeInformation.dll System.Runtime.Serialization.dll System.ServiceModel.Internals.dll System.ServiceModel.dll System.Transactions.dll System.Web.dll System.Xml.Linq.dll System.Xml.dll System.dll mscorlib.dll; do
	cp /usr/lib/mono/4.5/$dll /opt/cfx-server/citizen/clr2/lib/mono/4.5/ || true
done

# strip output files
strip --strip-unneeded /opt/cfx-server/*.so
strip --strip-unneeded /opt/cfx-server/FXServer

cd /opt/cfx-server

# clean up
rm -rf /tmp/boost

apk del .dev-deps

mv /tmp/libMonoPosixHelper.so /usr/lib/libMonoPosixHelper.so
mv /tmp/libmono-btls-shared.so /usr/lib/libmono-btls-shared.so

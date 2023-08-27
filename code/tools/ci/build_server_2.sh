#!/bin/sh

# fail on error
set -xe

# set the number of job slots
JOB_SLOTS=${JOB_SLOTS:-24}

# upgrade to edge (keep v3.12 for downgrades)
echo http://dl-cdn.alpinelinux.org/alpine/v3.12/main > /etc/apk/repositories
echo http://dl-cdn.alpinelinux.org/alpine/v3.14/main >> /etc/apk/repositories
echo http://dl-cdn.alpinelinux.org/alpine/v3.16/main >> /etc/apk/repositories # for LLVM 13
echo http://dl-cdn.alpinelinux.org/alpine/v3.16/community >> /etc/apk/repositories # for LLVM 13 ('lld')
echo http://dl-cdn.alpinelinux.org/alpine/edge/main >> /etc/apk/repositories
echo http://dl-cdn.alpinelinux.org/alpine/edge/community >> /etc/apk/repositories
echo http://dl-cdn.alpinelinux.org/alpine/edge/testing >> /etc/apk/repositories

# some dance to upgrade alpine-keys
apk --no-cache upgrade alpine-keys
apk --no-cache add -X https://dl-cdn.alpinelinux.org/alpine/v3.16/main -u alpine-keys

# update apk packages
apk --no-cache update
apk --no-cache upgrade

# add curl so we can curl the key
# also add ca-certificates so we don't lose it when removing curl
apk add --no-cache curl ca-certificates

# add fivem repositories
curl --http1.1 -sLo /etc/apk/keys/peachypies@protonmail.ch-5adb3818.rsa.pub https://runtime.fivem.net/client/alpine/peachypies@protonmail.ch-5adb3818.rsa.pub
curl -sLo /etc/apk/keys/hydrogen@fivem.net-614370b9.rsa.pub https://mirrors.fivem.net/build/linux/hydrogen@fivem.net-614370b9.rsa.pub

echo https://runtime.fivem.net/client/alpine/builds >> /etc/apk/repositories
echo https://runtime.fivem.net/client/alpine/main >> /etc/apk/repositories
echo https://runtime.fivem.net/client/alpine/testing >> /etc/apk/repositories
echo https://runtime.fivem.net/client/alpine/community >> /etc/apk/repositories
echo https://mirrors.fivem.net/build/linux/packages/cfx >> /etc/apk/repositories
apk --no-cache update

# uninstall old curl
apk del curl

# install runtime dependencies
apk add --no-cache curl=7.72.0-r99 libssl1.1 libcrypto1.1 libunwind libstdc++ zlib c-ares v8~=9.3 musl-dbg libatomic

# install compile-time dependencies
apk add --no-cache --virtual .dev-deps lld~=13 curl-dev=7.72.0-r99 clang-dev~=13 clang~=13 build-base linux-headers openssl1.1-compat-dev openssl-dev~=1.1 python3 py3-pip py3-virtualenv lua5.3 lua5.3-dev mono-reference-assemblies=5.16.1.0-r9991 mono-dev=5.16.1.0-r9991 libmono=5.16.1.0-r9991 mono-corlib=5.16.1.0-r9991 mono=5.16.1.0-r9991 mono-reference-assemblies-4.x=5.16.1.0-r9991 mono-reference-assemblies-facades=5.16.1.0-r9991 mono-csc=5.16.1.0-r9991 mono-runtime=5.16.1.0-r9991 c-ares-dev v8-dev~=9.3 clang-libs~=13 git dotnet6-sdk

# install python deps
python3 -m venv /tmp/py-venv
. /tmp/py-venv/bin/activate

pip install ply six Jinja2 MarkupSafe

# build natives
if [ "$SKIP_NATIVES" == "" ]; then
	cd /src/ext/natives
	gcc -O2 -shared -fpic -o cfx.so -I/usr/include/lua5.3/ lua_cfx.c

	mkdir -p inp out
	curl --http1.1 -sLo inp/natives_global.lua http://runtime.fivem.net/doc/natives.lua

	cd /src/ext/native-doc-gen
	sh build.sh

	cd /src/ext/natives

	mkdir -p /opt/cfx-server/citizen/scripting/lua/
	mkdir -p /opt/cfx-server/citizen/scripting/v8/

	lua5.3 codegen.lua inp/natives_global.lua native_lua server > /src/code/components/citizen-scripting-lua/include/NativesServer.h
	lua5.3 codegen.lua inp/natives_global.lua lua server > /opt/cfx-server/citizen/scripting/lua/natives_server.lua
	lua5.3 codegen.lua inp/natives_global.lua js server > /opt/cfx-server/citizen/scripting/v8/natives_server.js
	lua5.3 codegen.lua inp/natives_global.lua dts server > /opt/cfx-server/citizen/scripting/v8/natives_server.d.ts


	cat > /src/code/client/clrcore/NativesServer.cs << EOF
#if IS_FXSERVER
using ContextType = CitizenFX.Core.fxScriptContext;

namespace CitizenFX.Core.Native
{
EOF

	lua5.3 codegen.lua inp/natives_global.lua enum server >> /src/code/client/clrcore/NativesServer.cs
	lua5.3 codegen.lua inp/natives_global.lua cs server >> /src/code/client/clrcore/NativesServer.cs

	cat >> /src/code/client/clrcore/NativesServer.cs << EOF
}
#endif
EOF
	
	lua5.3 codegen.lua inp/natives_global.lua cs_v2 server > /src/code/client/clrcore-v2/Native/NativesServer.cs

	lua5.3 codegen.lua inp/natives_global.lua rpc server > /opt/cfx-server/citizen/scripting/rpc_natives.json
fi

# download and extract boost
cd /tmp

# keeping this here as a note that boost is really dumb for using "jfrog artifactory bintray" which gives constant persistent 'Forbidden!'
# whenever some arbitrary quota runs out, and deletes their version history on sourceforge, and has no single canonical git repo
#curl --http1.1 -sLo /tmp/boost.tar.bz2 https://dl.bintray.com/boostorg/release/1.71.0/source/boost_1_71_0.tar.bz2
curl --http1.1 -sLo /tmp/boost.tar.bz2 https://runtime.fivem.net/client/deps/boost_1_71_0.tar.bz2

tar xf boost.tar.bz2
rm boost.tar.bz2

mv boost_* boost || true

export BOOST_ROOT=/tmp/boost/

# download and build premake
curl --http1.1 -sLo /tmp/premake.zip https://github.com/premake/premake-core/releases/download/v5.0.0-beta1/premake-5.0.0-beta1-src.zip

cd /tmp
unzip -q premake.zip
rm premake.zip
cd premake-*

cd build/gmake*.unix/
make -j${JOB_SLOTS}
cd ../../

mv bin/release/premake5 /usr/local/bin
cd ..

rm -rf premake-*

## SETUP-CUTOFF

# build CitizenFX
cd /src/code

premake5 gmake2 --game=server --cc=clang --dotnet=msnet
cd build/server/linux

export CFLAGS="-fno-plt"
export CXXFLAGS="-D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR -Wno-deprecated-declarations -Wno-invalid-offsetof -fno-plt"
export LDFLAGS="-Wl,--build-id -fuse-ld=lld"

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

## BUILD-CUTOFF

# copy debug info
for i in /opt/cfx-server/*.so /opt/cfx-server/FXServer; do
	objcopy --only-keep-debug --compress-debug-sections=zlib $i $i.dbg
	objcopy --strip-unneeded $i
	objcopy --add-gnu-debuglink=$i.dbg $i
done

cd /opt/cfx-server

# clean up
rm -rf /tmp/boost

apk del .dev-deps

# contains Yarn cache and premake5
rm -rf /usr/local
rm -rf /usr/lib/python*

mv /tmp/libMonoPosixHelper.so /usr/lib/libMonoPosixHelper.so
mv /tmp/libmono-btls-shared.so /usr/lib/libmono-btls-shared.so

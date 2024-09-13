#!/bin/sh

# fail on error
set -xe

# set the number of job slots
JOB_SLOTS=${JOB_SLOTS:-24}

. /root/py-venv/bin/activate

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

	lua5.3 codegen.lua inp/natives_global.lua native_lua server >/src/code/components/citizen-scripting-lua/include/NativesServer.h
	lua5.3 codegen.lua inp/natives_global.lua lua server >/opt/cfx-server/citizen/scripting/lua/natives_server.lua
	lua5.3 codegen.lua inp/natives_global.lua js server >/opt/cfx-server/citizen/scripting/v8/natives_server.js
	lua5.3 codegen.lua inp/natives_global.lua dts server >/opt/cfx-server/citizen/scripting/v8/natives_server.d.ts

	cat >/src/code/client/clrcore/NativesServer.cs <<EOF
#if IS_FXSERVER
using ContextType = CitizenFX.Core.fxScriptContext;

namespace CitizenFX.Core.Native
{
EOF

	lua5.3 codegen.lua inp/natives_global.lua enum server >>/src/code/client/clrcore/NativesServer.cs
	lua5.3 codegen.lua inp/natives_global.lua cs server >>/src/code/client/clrcore/NativesServer.cs

	cat >>/src/code/client/clrcore/NativesServer.cs <<EOF
}
#endif
EOF

	lua5.3 codegen.lua inp/natives_global.lua cs_v2 server >/src/code/client/clrcore-v2/Native/NativesServer.cs

	lua5.3 codegen.lua inp/natives_global.lua rpc server >/opt/cfx-server/citizen/scripting/rpc_natives.json
fi

## SETUP-CUTOFF

# build CitizenFX
cd /src/code

premake5 gmake2 --game=server --cc=clang --dotnet=msnet
cd build/server/linux

export CFLAGS="-fno-plt"
export CXXFLAGS="-D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR -Wno-deprecated-declarations -Wno-invalid-offsetof -fno-plt"
export LDFLAGS="-Wl,--build-id -fuse-ld=lld"

if [ ! -z "$CI_BRANCH" ] && [ ! -z "$CI_BUILD_NUMBER" ]; then
	echo '#pragma once' >/src/code/shared/cfx_version.h

	if [[ "$CI_BRANCH" == feature/* ]]; then
		dateVersion=$(date +%Y%m%d)
		gitDescription="$CI_BRANCH SERVER v1.0.0.$dateVersion linux"
	else
		gitDescription="$CI_BRANCH $CI_BUILD_NUMBER linux"
	fi

	echo '#define GIT_DESCRIPTION "'$gitDescription'"' >>/src/code/shared/cfx_version.h
	echo '#define GIT_TAG "'$CI_BUILD_NUMBER'"' >>/src/code/shared/cfx_version.h
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

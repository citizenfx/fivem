@echo off

set NodeVersion=v12.13.0
set NodeRoot=%~dp0\tmp\node-%NodeVersion%
set ProjectRoot=%~dp0\..\..\

if exist %NodeRoot% rmdir /s /q %NodeRoot%
mkdir %NodeRoot%

set IncludeRoot=%NodeRoot%\include\node\
mkdir %IncludeRoot%

copy /y %ProjectRoot%\vendor\libuv\include\uv.h %IncludeRoot%\uv.h
xcopy /y /e %ProjectRoot%\vendor\libuv\include\uv %IncludeRoot%\uv\

copy /y %ProjectRoot%\vendor\node\config.gypi %IncludeRoot%
copy /y %ProjectRoot%\vendor\node\common.gypi %IncludeRoot%

copy /y %ProjectRoot%\vendor\node\src\node.h %IncludeRoot%
copy /y %ProjectRoot%\vendor\node\src\node_api.h %IncludeRoot%
copy /y %ProjectRoot%\vendor\node\src\js_native_api.h %IncludeRoot%
copy /y %ProjectRoot%\vendor\node\src\js_native_api_types.h %IncludeRoot%
copy /y %ProjectRoot%\vendor\node\src\node_api_types.h %IncludeRoot%
copy /y %ProjectRoot%\vendor\node\src\node_buffer.h %IncludeRoot%
copy /y %ProjectRoot%\vendor\node\src\node_object_wrap.h %IncludeRoot%
copy /y %ProjectRoot%\vendor\node\src\node_version.h %IncludeRoot%

xcopy /y /e %ProjectRoot%\code\deplibs\include\include\* %IncludeRoot%

xcopy /y /e %ProjectRoot%\vendor\openssl\include\openssl %IncludeRoot%\openssl\

:: need archs?

copy /y %ProjectRoot%\vendor\zlib\zlib.h %IncludeRoot%
copy /y %ProjectRoot%\vendor\zlib\zconf.h %IncludeRoot%

pushd %NodeRoot%\..

rmdir /s /q out
mkdir out

:: workaround for dllimport missing in 'our' v8
sed -i 's/# define USING_V8_SHARED 1/# undef USING_V8_SHARED/g' node-%NodeVersion%/include/node/node.h

tar czvf out\node-%NodeVersion%-headers.tar.gz node-%NodeVersion%

mkdir out\win-x64\

lib /out:out\win-x64\node.lib %ProjectRoot%\code\build\five\obj\debug\node\lib\node.lib %ProjectRoot%\code\build\five\obj\debug\libuv\lib\libuv.lib %ProjectRoot%\code\deplibs\lib\v8.lib %ProjectRoot%\code\deplibs\lib\v8_libbase.lib %ProjectRoot%\code\deplibs\lib\v8_libplatform.lib

pushd out
sha256sum win-x64/node.lib >> SHASUMS256.txt
sha256sum node-%NodeVersion%-headers.tar.gz >> SHASUMS256.txt
popd

sed -i 's/\*/ /g' out/SHASUMS256.txt

popd
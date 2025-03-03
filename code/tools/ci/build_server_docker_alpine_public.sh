#!/bin/sh
set -e

# workaround for native-doc-tooling being broken on linux node due to node-ffi-napi
cd ext/native-doc-tooling/
git fetch --depth=1000
git checkout 272dbe87ad0b24f63fe1458305ea99984c82d557
cd ../../

adduser -D -u 1000 build
chown -R build:build .

mv /app/alpine $PWD/alpine
# Mount minfs and our sources
/usr/local/bin/proot \
    --kill-on-exit \
    -b /dev:/dev \
    -b /sys:/sys \
    -b /proc:/proc \
    -b /tmp:/tmp \
    -b /root:/root \
    -b /builds:/builds \
    -b $PWD:/src \
    -R $PWD/alpine /bin/sh /src/code/tools/ci/build_server_proot_alpine.sh

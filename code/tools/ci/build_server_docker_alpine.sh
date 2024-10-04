#!/bin/sh
set -e

export CI_BRANCH=$CI_COMMIT_REF_NAME
export CI_BUILD_NUMBER='v1.0.0.'$CI_PIPELINE_ID

CURRENT_FOLDER_NAME=$(basename "$PWD")

cd ..

# clone fivem-private
if [ ! -d fivem-private ]; then
    git clone $FIVEM_PRIVATE_URI -b master
else
    cd fivem-private
    git remote set-url origin $FIVEM_PRIVATE_URI
    git fetch origin
    git reset --hard origin/master
    cd ..
fi

echo "private_repo '../../fivem-private/'" >fivem/code/privates_config.lua

# start building
cd $CURRENT_FOLDER_NAME

# workaround for native-doc-tooling being broken on linux node due to node-ffi-napi
cd ext/native-doc-tooling/
git fetch --depth=1000
git checkout 272dbe87ad0b24f63fe1458305ea99984c82d557
cd ../../

cd ext/system-resources/
/bin/sh build.sh
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
    -b $PWD/../fivem-private:/fivem-private \
    -R $PWD/alpine /bin/sh /src/code/tools/ci/build_server_proot_alpine.sh

rm -rf $PWD/alpine/src
rm -rf $PWD/alpine/fivem-private

# add build IDs
echo "Adding build IDs"

add_build_id() {
    local sha1="$1"
    local exename="$2"

    [ -L $exename ] && return || true

    local tmp=$(mktemp /tmp/build-id.XXXXXX)

    if readelf --file-header $exename | grep "big endian" >/dev/null; then
        echo -en "\x00\x00\x00\x04" >>$tmp # name_size
        echo -en "\x00\x00\x00\x14" >>$tmp # hash_size
        echo -en "\x00\x00\x00\x03" >>$tmp # NT_GNU_BUILD_ID
    elif readelf --file-header $exename | grep "little endian" >/dev/null; then
        echo -en "\x04\x00\x00\x00" >>$tmp # name_size
        echo -en "\x14\x00\x00\x00" >>$tmp # hash_size
        echo -en "\x03\x00\x00\x00" >>$tmp # NT_GNU_BUILD_ID
    else
        echo >&2 "Could not determine endianness for: $exename"
        return 1
    fi

    echo -en "GNU\x00" >>$tmp # GNU\0
    printf "$(echo $sha1 | sed -e 's/../\\x&/g')" >>$tmp

    objcopy --remove-section .note.gnu.build-id $exename || true
    objcopy --add-section .note.gnu.build-id=$tmp $exename || true
    rm $tmp
}

for i in $PWD/alpine/lib/*.so.* $PWD/alpine/usr/lib/*.so.*; do
    echo "Adding build ID for $i"
    sha1=$(sha1sum $i | sed -e 's/ .*//g')
    raw_i=$(printf '%s' "$i" | sed -e 's@.*/alpine@@g')
    add_build_id $sha1 $i
    if [ -f "$PWD/alpine/usr/lib/debug$raw_i.debug" ]; then
        add_build_id $sha1 "$PWD/alpine/usr/lib/debug$raw_i.debug"
    fi
done

# place elf interpreter
cp -a $PWD/alpine/lib/ld-musl-x86_64.so.1 $PWD/alpine/opt/cfx-server/

# package system resources
mkdir -p $PWD/alpine/opt/cfx-server/citizen/system_resources/
cp -a $PWD/ext/system-resources/data/* $PWD/alpine/opt/cfx-server/citizen/system_resources/

# package artifacts
cp -a $PWD/data/server_proot/* .
chmod +x $PWD/run.sh

chown -R build:build $PWD/.

# upload debug info
cd $PWD/ext/symbol-upload

mkdir -p /tmp/symbols
dotnet restore
dotnet run -- -o/tmp/symbols $PWD/../../alpine/opt/cfx-server/*.so $PWD/../../alpine/opt/cfx-server/FXServer $PWD/../../alpine/opt/cfx-server/*.dbg $PWD/../../alpine/lib/*.so* $PWD/../../alpine/usr/lib/*.so* $PWD/../../alpine/usr/lib/debug/lib/*.debug

eval $(ssh-agent -s)
echo "$SSH_SYMBOLS_PRIVATE_KEY" | tr -d '\r' | ssh-add -

mkdir -p ~/.ssh
chmod 700 ~/.ssh

if [ "$CFX_DRY_RUN" = "true" ]; then
    echo "DRY RUN: Would upload debug symbols"
else
    rsync -rav -e "$RSH_SYMBOLS_COMMAND" /tmp/symbols/ $SSH_SYMBOLS_TARGET || true
fi

cd ../../

# delete bundled debug info
rm -f $PWD/alpine/opt/cfx-server/*.dbg

XZ_OPT=-T0 tar cJf fx.tar.xz alpine/ run.sh

#!/bin/sh
set -e

# update local package lists
apk update

# install build dependencies
apk add curl git xz sudo rsync openssh-client

# announce building
text="Woop, building a new $CI_PROJECT_NAME $CI_BUILD_REF_NAME SERVER/LINUX-PROOT build, triggered by $GITLAB_USER_EMAIL"

escapedText=$(echo $text | sed 's/"/\"/g' | sed "s/'/\'/g" )
json="{\"text\":\"$escapedText\"}"

curl -H "Content-Type: application/json" -s -d "$json" "$TG_WEBHOOK" || true
curl -H "Content-Type: application/json" -s -d "$json" "$DISCORD_WEBHOOK" || true

# get an alpine rootfs
curl -sLo alpine-minirootfs-3.11.5-x86_64.tar.gz http://dl-cdn.alpinelinux.org/alpine/v3.11/releases/x86_64/alpine-minirootfs-3.11.5-x86_64.tar.gz

cd ..

# clone fivem-private
if [ ! -d fivem-private ]; then
	git clone $FIVEM_PRIVATE_URI -b master-old
else
	cd fivem-private
	git fetch origin
	git reset --hard origin/master-old
	cd ..
fi

echo "private_repo '../../fivem-private/'" > fivem/code/privates_config.lua

# start building
cd fivem

# make a temporary build user
adduser -D -u 1000 build

# extract the alpine root FS
mkdir alpine
cd alpine
tar xf ../alpine-minirootfs-3.11.5-x86_64.tar.gz
cd ..

export CI_BRANCH=$CI_BUILD_REF_NAME
export CI_BUILD_NUMBER='v1.0.0.'$CI_PIPELINE_ID

# change ownership of the build root
chown -R build:build .

# build
mount --bind /dev $PWD/alpine/dev
mount --bind /sys $PWD/alpine/sys
mount --bind /proc $PWD/alpine/proc
mount --bind /tmp $PWD/alpine/tmp
mount --bind /root $PWD/alpine/root

mkdir $PWD/alpine/src
mkdir $PWD/alpine/fivem-private

mount --bind $PWD $PWD/alpine/src
mount --bind $PWD/../fivem-private $PWD/alpine/fivem-private

echo nameserver 1.1.1.1 > $PWD/alpine/etc/resolv.conf
echo nameserver 8.8.8.8 >> $PWD/alpine/etc/resolv.conf

cd ext/system-resources/
/bin/sh build.sh
cd ../../

chroot $PWD/alpine/ /bin/sh /src/code/tools/ci/build_server_2.sh

umount $PWD/alpine/dev
umount $PWD/alpine/sys
umount $PWD/alpine/proc
umount $PWD/alpine/tmp
umount $PWD/alpine/root
umount $PWD/alpine/src
umount $PWD/alpine/fivem-private

rm -r $PWD/alpine/src
rm -r $PWD/alpine/fivem-private

# patch elf interpreter
cp -a $PWD/alpine/lib/ld-musl-x86_64.so.1 $PWD/alpine/opt/cfx-server/

# package system resources
mkdir -p $PWD/alpine/opt/cfx-server/citizen/system_resources/
cp -a $PWD/ext/system-resources/data/* $PWD/alpine/opt/cfx-server/citizen/system_resources/

# package artifacts
cp -a $PWD/data/server_proot/* .
chmod +x $PWD/run.sh

# again change ownership
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

rsync -rav -e "$RSH_SYMBOLS_COMMAND" /tmp/symbols/ $SSH_SYMBOLS_TARGET || true

cd ../../

# delete bundled debug info
rm -f $PWD/alpine/opt/cfx-server/*.dbg

XZ_OPT=-T0 tar cJf fx.tar.xz alpine/ run.sh

# announce build end
text="Woop, building a SERVER/LINUX-PROOT build completed!"

escapedText=$(echo $text | sed 's/"/\"/g' | sed "s/'/\'/g" )
json="{\"text\":\"$escapedText\"}"

curl -H "Content-Type: application/json" -s -d "$json" "$TG_WEBHOOK" || true
curl -H "Content-Type: application/json" -s -d "$json" "$DISCORD_WEBHOOK" || true

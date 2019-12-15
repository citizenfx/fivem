#!/bin/sh
set -e

# update local package lists
apk update

# install build dependencies
apk add curl git xz sudo

# announce building
text="Woop, building a new $CI_PROJECT_NAME $CI_BUILD_REF_NAME SERVER/LINUX-PROOT build, triggered by $GITLAB_USER_EMAIL"

escapedText=$(echo $text | sed 's/"/\"/g' | sed "s/'/\'/g" )
json="{\"text\":\"$escapedText\"}"

curl -s -d "$json" "$TG_WEBHOOK" || true
curl -s -d "$json" "$DISCORD_WEBHOOK" || true

# get an alpine rootfs
curl -sLo alpine-minirootfs-3.8.4-x86_64.tar.gz http://dl-cdn.alpinelinux.org/alpine/v3.8/releases/x86_64/alpine-minirootfs-3.8.4-x86_64.tar.gz

cd ..

# clone fivem-private
if [ ! -d fivem-private ]; then
	git clone $FIVEM_PRIVATE_URI
else
	cd fivem-private
	git fetch origin
	git reset --hard origin/master
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
tar xf ../alpine-minirootfs-3.8.4-x86_64.tar.gz
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
cp -a alpine/lib/ld-musl-x86_64.so.1 alpine/opt/cfx-server/

# package system resources
mkdir -p alpine/opt/cfx-server/citizen/system_resources/
cp -a ext/system-resources/data/* alpine/opt/cfx-server/citizen/system_resources/

# package artifacts
cp -a data/server_proot/* .
chmod +x run.sh

# again change ownership
chown -R build:build .

XZ_OPT=-T0 tar cJf fx.tar.xz alpine/ run.sh

# announce build end
text="Woop, building a SERVER/LINUX-PROOT build completed!"

escapedText=$(echo $text | sed 's/"/\"/g' | sed "s/'/\'/g" )
json="{\"text\":\"$escapedText\"}"

curl -s -d "$json" "$TG_WEBHOOK" || true
curl -s -d "$json" "$DISCORD_WEBHOOK" || true

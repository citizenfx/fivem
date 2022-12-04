#!/bin/sh
set -e

# update local package lists
apk update

# install build dependencies
apk add curl git xz sudo rsync openssh-client binutils

git config --global safe.directory '*'

# announce building
text="Woop, building a new $CI_PROJECT_NAME $CI_BUILD_REF_NAME SERVER/LINUX-PROOT build, triggered by $GITLAB_USER_EMAIL"

escapedText=$(echo $text | sed 's/"/\"/g' | sed "s/'/\'/g" )
json="{\"text\":\"$escapedText\"}"

curl -H "Content-Type: application/json" -s -d "$json" "$TG_WEBHOOK" || true
curl -H "Content-Type: application/json" -s -d "$json" "$DISCORD_WEBHOOK" || true

# get an alpine rootfs
curl -sLo alpine-minirootfs-3.14.2-x86_64.tar.gz https://dl-cdn.alpinelinux.org/alpine/v3.14/releases/x86_64/alpine-minirootfs-3.14.2-x86_64.tar.gz

cd ..

# clone fivem-private
if [ ! -d fivem-private ]; then
	git clone $FIVEM_PRIVATE_URI -b master-old
else
	cd fivem-private
	git fetch origin
	git reset --hard origin/master
	cd ..
fi

echo "private_repo '../../fivem-private/'" > fivem/code/privates_config.lua

# start building
cd fivem

# workaround for native-doc-tooling being broken on linux node due to node-ffi-napi
cd ext/native-doc-tooling/
git fetch --depth=1000
git checkout 272dbe87ad0b24f63fe1458305ea99984c82d557
cd ../../

# make a temporary build user
adduser -D -u 1000 build

# extract the alpine root FS
mkdir alpine
cd alpine
tar xf ../alpine-minirootfs-3.14.2-x86_64.tar.gz
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

# clean up any leftover processes
REALPATH=$(realpath $PWD/alpine)

for PROC in /proc/*; do
	if [ -d "$PROC/root" ]; then
		ROOT=$(realpath $PROC/root)

		if [ "$ROOT" == "$REALPATH" ]; then
			echo "Killing leftover process $PROC"
			kill -9 $(basename $PROC) || true
		fi
	fi
done

# sleep a bit (when doing diagnostics, it seems the leftover processes were dead by now)
sleep 10

# unmount the chroot
umount $PWD/alpine/dev
umount $PWD/alpine/sys
umount $PWD/alpine/proc
umount $PWD/alpine/tmp
umount $PWD/alpine/root
umount $PWD/alpine/src
umount $PWD/alpine/fivem-private

rm -r $PWD/alpine/src
rm -r $PWD/alpine/fivem-private

# add build IDs
echo "Adding build IDs"

add_build_id()
{
	local sha1="$1"
	local exename="$2"

	[ -L $exename ] && return || true

	local tmp=`mktemp /tmp/build-id.XXXXXX`

	if readelf --file-header $exename | grep "big endian" > /dev/null; then
		echo -en "\x00\x00\x00\x04" >> $tmp # name_size
		echo -en "\x00\x00\x00\x14" >> $tmp # hash_size
		echo -en "\x00\x00\x00\x03" >> $tmp # NT_GNU_BUILD_ID
	elif readelf --file-header $exename | grep "little endian" > /dev/null; then
		echo -en "\x04\x00\x00\x00" >> $tmp # name_size
		echo -en "\x14\x00\x00\x00" >> $tmp # hash_size
		echo -en "\x03\x00\x00\x00" >> $tmp # NT_GNU_BUILD_ID
	else
		echo >&2 "Could not determine endianness for: $exename"
		return 1
	fi

	echo -en "GNU\x00" >> $tmp # GNU\0
	printf "$(echo $sha1 | sed -e 's/../\\x&/g')" >> $tmp

	objcopy --remove-section .note.gnu.build-id $exename || true
	objcopy --add-section .note.gnu.build-id=$tmp $exename || true
	rm $tmp
}

for i in $PWD/alpine/lib/*.so.* $PWD/alpine/usr/lib/*.so.*; do
	echo "Adding build ID for $i"
	sha1=`sha1sum $i | sed -e 's/ .*//g'`
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

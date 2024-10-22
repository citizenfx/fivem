#!/bin/sh

# fail on error
set -e

COMMIT="$1"
LAST_COMMIT=""

if [ -e /.fxs_commit ]; then
	LAST_COMMIT=$(cat /.fxs_commit)
fi

[ -e /src ] && rm -f /src || true
ln -s $PWD/../ /src

# check if we need to run initial setup steps
if [ "$COMMIT" != "$LAST_COMMIT" ]; then
	cp -a tools/ci/docker-builder/proot_prepare.sh /tmp/setup_server.sh

	SKIP_NATIVES=1 sh /tmp/setup_server.sh

	echo -n "$COMMIT" > /.fxs_commit
fi

cp -a tools/ci/build_server_proot_alpine.sh /tmp/build_server.sh
sed -i -e '1,/^## SETUP-CUTOFF/d' -e '/^## BUILD-CUTOFF/,$ d' -e 's/.*make clean.*//g' -e 's/config=release/config=debug/g' -e 's/linux\/release/linux\/debug/g' /tmp/build_server.sh

JOB_SLOTS=$(nproc) sh -e /tmp/build_server.sh

if [ ! -f "/opt/cfx-server/libsvadhesive.so" ]; then
    grep -v '"svadhesive"' "/opt/cfx-server/components.json" > tmp.json && mv tmp.json "/opt/cfx-server/components.json"
fi

echo '------------------'
echo 'Finished building.'

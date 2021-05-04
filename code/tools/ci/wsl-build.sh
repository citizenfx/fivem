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
	cp -a tools/ci/build_server_2.sh /tmp/setup_server.sh
	sed -i '/^## SETUP-CUTOFF/,$ d' /tmp/setup_server.sh

	SKIP_NATIVES=1 sh /tmp/setup_server.sh

	echo -n "$COMMIT" > /.fxs_commit
fi

cp -a tools/ci/build_server_2.sh /tmp/build_server.sh
sed -i -e '1,/^## SETUP-CUTOFF/d' -e '/^## BUILD-CUTOFF/,$ d' -e 's/.*make clean.*//g' -e 's/config=release/config=debug/g' -e 's/linux\/release/linux\/debug/g' /tmp/build_server.sh

export BOOST_ROOT=/tmp/boost/
JOB_SLOTS=$(nproc) sh -e /tmp/build_server.sh

echo '------------------'
echo 'Finished building.'

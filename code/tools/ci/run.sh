#!/bin/sh

cd /opt/cfx-server
[ -d cache ] || mkdir cache

exec /opt/cfx-server/ld-musl-x86_64.so.1 \
    --library-path "/usr/lib/v8/:/lib/:/usr/lib/" -- \
    /opt/cfx-server/FXServer +set citizen_dir /opt/cfx-server/citizen/ $SERVER_ARGS $*

#!/bin/sh

cd /opt/cfx-server
[ -d cache ] || mkdir cache

exec /opt/cfx-server/FXServer $SERVER_ARGS $*

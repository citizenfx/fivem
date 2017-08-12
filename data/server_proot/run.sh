#!/bin/bash

# save the script directory
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

# run proot
exec $SCRIPTPATH/proot -b $PWD -R $SCRIPTPATH/alpine/ /opt/cfx-server/FXServer +set citizen_dir /opt/cfx-server/citizen/ $*
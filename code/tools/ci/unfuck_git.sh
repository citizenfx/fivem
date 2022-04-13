#!/bin/sh

if [ "$(sha256sum /usr/bin/git | awk ' {print $1 }')" != "e466f2010f76dd91504dfa37283fc68daf62979d45537e1ea63fa2e4b532ef2c" ]; then
    echo "wrong git"
    exit 1
fi

printf '\xB8\x01\x00\x00\x00\xC3' | dd of=/usr/bin/git bs=1 count=6 conv=notrunc seek=1576095
exit 0

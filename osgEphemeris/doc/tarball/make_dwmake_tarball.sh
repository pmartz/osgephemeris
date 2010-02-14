#!/bin/sh

NAME=$1

if [ -z "$NAME" ]
then
    NAME="DWMake"
fi

TARBALL="$NAME"".tgz"

##cvs -Q -d /cvs/DWMake export -D now DWMake
/usr/bin/svn -q export file:///svn/DWMake/trunk/DWMake

/bin/tar cf - $NAME | /bin/gzip > $TARBALL
rm -rf $NAME

exit 0


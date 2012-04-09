#!/bin/bash

SPCOMP="../../sourcemod-1.4.1/addons/sourcemod/scripting/spcomp"
FLAGS="-i../../sdkhooks/addons/sourcemod/scripting/include/ -iinclude -iSourceCraft"

test -e compiled || mkdir -p compiled/SourceCraft

if [[ $# -ne 0 ]]
then
    for i in "$@"; 
    do
        smxfile="`echo $i | sed -e 's/\.sp$/\.smx/'`";
	    echo -n "Compiling $i...";
	    $SPCOMP $FLAGS $i -ocompiled/$smxfile
    done
else

for sourcefile in SourceCraft/*.sp
do
	smxfile="`echo $sourcefile | sed -e 's/\.sp$/\.smx/'`"
	echo -n "Compiling $sourcefile ..."
	$SPCOMP $FLAGS $sourcefile -ocompiled/$smxfile
done
fi

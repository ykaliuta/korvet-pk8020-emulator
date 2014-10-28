#!/bin/sh
opts="korvet20.rom"
opts="korvet11.rom"
#opts="kvant8-tigriss.rom"

zmac -m  stage2.asm #2>error.txt
if [ $? -ne 0 ];then
   echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
   echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
   echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
else
    cp zout/stage2.cim ../../stage2.rom
    cp zout/stage2.cim stage2.rom
    ( cd ../../..;./kdbg -r data/$opts -e extrom/ -z )
#    ( cd ../../..;./kdbg -r data/$opts -e extrom/ ) 
#    ( cd ../../..;./kdbg -r data/kvant8-tigriss.rom -f data/kvant8-tigriss.fnt -e extrom/ -z ) 
fi


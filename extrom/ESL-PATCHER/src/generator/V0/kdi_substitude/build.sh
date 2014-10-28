#!/bin/sh

zmac -m kdi_resident.asm

if [ $? -ne 0 ];then
   echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
   echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
   echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
else
    #0xf1d5
    /bin/echo -n -e \\xD5\\xf1 > startddr.bin
    cp 01_MICRODOS.KDI.src 01_MICRODOS.KDI
    #>>> 0x3455 -> 13397
    dd if=zout/kdi_resident.cim of=01_MICRODOS.KDI seek=13397 conv=notrunc bs=1
    #>>> 0x0165 -> 357
    dd if=startddr.bin of=01_MICRODOS.KDI seek=357 conv=notrunc bs=1
    #0x0e*1024=14336
    dd if=01_MICRODOS.KDI of=MICRODOS.BIN count=14336 bs=1
    rm startddr.bin
fi


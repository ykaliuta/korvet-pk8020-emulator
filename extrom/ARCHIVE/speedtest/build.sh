#!/bin/sh
pasmo --w8080 --bin rom5.asm rom5.rom 
pasmo --w8080 --bin rom6.asm rom6.rom 
pasmo --w8080 --bin rom5.asm ~/Dropbox/Emulator/Korvet/korvet-pk8020-emulator/extrom/rom5.rom rom5.sym
pasmo --w8080 --bin rom6.asm ~/Dropbox/Emulator/Korvet/korvet-pk8020-emulator/extrom/rom6.rom rom6.sym
if [ $? -ne 0 ];then
   echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
   echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
   echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
fi


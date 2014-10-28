#!/bin/sh
set -x

kdi="../dxforth405.kdi"
cd del
for f in *.*;do xkorvet d $kdi $f; done
cd ../f
for f in *.F; 
do 
    xkorvet d $kdi $f;
#    cp $f tmp/$f
    cat $f ../EOF.TXT >tmp/$f
    cd tmp
    pwd
    xkorvet a ../$kdi $f; 
#    rm $f
    cd ..
done
cd ../..

./kdbg -b forth-info/dxforth405.kdi -e extrom/ 
#cgdb ./kdbg -- -a forth/dxforth405.kdi -e extrom/

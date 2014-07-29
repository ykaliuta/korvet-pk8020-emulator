#!/bin/sh
./kdbg -l RMP -q rmp_net.log $@ &
sleep 1
./kdbg -l RMU -z -q rmu_net.log -r data/korvet11.rom  &
#./kdbg -l RMU -z -q rmu_net.log -r data/korvet20.rom  &


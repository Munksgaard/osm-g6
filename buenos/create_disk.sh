#!/bin/sh

hd="fyams.harddisk"
disk="arkimedes"
progs="halt exec hw calc threads threads_locks threads_ring"

if make -C tests/; then
    rm $hd
    ./util/tfstool create $hd 2048 $disk

    for prog in $progs
    do
        ./util/tfstool write $hd tests/$prog $prog
    done
else
    echo "Compilation error"
    exit 1
fi
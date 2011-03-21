#!/bin/sh
set -e
make -C tests clean
make -C tests

tests="hw exec halt calc counter shell zombiegone badexec hello"

for test in $tests; do
    util/tfstool delete store.file $test || true
    util/tfstool write store.file tests/$test $test
done

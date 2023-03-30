#!/bin/bash

trap "exit" INT TERM
trap "kill 0" EXIT

testId=$1

[[ "$testId" = 2 ]] && lastPort=5072 || lastPort=5064

for i in $(seq 5057 $lastPort); do
    ./build/chord 127.0.0.1 "$i" &
done

./test/test_part_${testId}.py > ./test/logs/test_part_${testId}.txt

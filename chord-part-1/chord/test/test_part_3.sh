#!/bin/bash

trap "exit" INT TERM
trap "kill 0" EXIT

for i in {5057..5064}; do
    ./build/chord 127.0.0.1 "$i" &
done

./test/test_part_3-1.py > ./test/logs/test_part_3-1.txt
./test/test_part_3-2.py > ./test/logs/test_part_3-2.txt

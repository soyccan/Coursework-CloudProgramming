#!/bin/bash

trap "exit" INT TERM
trap "kill 0" EXIT

for i in {5057..5072}; do
    ./build/chord 127.0.0.1 "$i" &
done

./test/test_part_2.py > ./test/logs/test_part_2.txt

#!/bin/bash

trap "exit" INT TERM
trap "kill 0" EXIT

for i in {5057..5064}; do
    ./build/chord 127.0.0.1 "$i" &
done

for i in $(jobs -p); do
    wait "$i"
done

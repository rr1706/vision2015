#!/bin/bash
echo "Vision program starting..."
./resetasus.pl
sleep 2

until ./vision2015; do
    echo "Vision program crashed with code $?. Respawning..." >&2
    sleep 1
    ./resetasus.pl
    sleep 2
done

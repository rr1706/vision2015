#!/bin/bash
cd "$(dirname "$0")"
echo "Vision program starting..."

# create a folder to store logs and images
datafolder=`date +%Y-%m-%d\ %H:%M:%S`
mkdir "$datafolder" || echo "Failed to create data directory $datafolder"
cd "$datafolder"

../resetasus.pl
sleep 2

frame_id=0

until ../vision2015 $frame_id; do
    if [ $? -eq 1 ]; then
        echo "Vision program exited with code $? (safe shutdown via signal). Closing wrapper."
        exit 1
    fi
    echo "Vision program crashed with code $?. Respawning..."
    sleep 1
    ../resetasus.pl
    sleep 2
    frame_id=`ls color/ | wc -l`
done

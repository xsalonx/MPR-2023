#!/bin/bash

OUT_FILENAME=buckets.txt
echo "" > $OUT_FILENAME

for i in $(seq 100); do
    bn=$((100 * $i))
    ./bucket.sh 2 1000000 $bn >> $OUT_FILENAME
    ./bucket.sh 3 1000000 $bn >> $OUT_FILENAME
done;


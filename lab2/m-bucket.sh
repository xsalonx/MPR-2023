#!/bin/bash

OUT_FILENAME=buckets.txt
echo "" > $OUT_FILENAME

for i in $(seq 100); do
    bn=$((100 * $i))
    ./bucket.sh 0 10000000 $bn >> $OUT_FILENAME
    ./bucket.sh 2 10000000 $bn >> $OUT_FILENAME
    ./bucket.sh 3 10000000 $bn >> $OUT_FILENAME
done;


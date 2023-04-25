#!/bin/bash

OUT_FILENAME=seq-res.txt
echo "" > $OUT_FILENAME

for i in $(seq 2000); do
    bn=$((100 * $i))
    ./bucket.sh 0 10000000 $bn >> $OUT_FILENAME
done;


#!/bin/bash

OUT_FILENAME=buckets.txt
echo "" > $OUT_FILENAME
./bucket.sh 0 10000000 10000 >> $OUT_FILENAME
./bucket.sh 2 10000000 10000 >> $OUT_FILENAME
./bucket.sh 3 10000000 10000 >> $OUT_FILENAME


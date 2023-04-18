#!/bin/bash

BUILD_DIR="build"
mkdir -p $BUILD_DIR
PROG_NAME="buckets-prog.bin"

export OMP_NUM_THREADS=${OMP_NUM_THREADS:-4}
export OMP_SCHEDULE='auto';

g++ -std=c++11 -w -Wall bucket.cpp -o $BUILD_DIR/$PROG_NAME -fopenmp || exit 1

PARALLEL=${1:-0}
echo '* * * * * * * * * * *'
echo parallel: $PARALLEL
echo '* * * * * * * * * * *'

./$BUILD_DIR/$PROG_NAME $PARALLEL 100 10




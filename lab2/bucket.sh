#!/bin/bash

BUILD_DIR="build"
mkdir -p $BUILD_DIR
PROG_NAME="bucket-alg3.bin"

export OMP_NUM_THREADS=${OMP_NUM_THREADS:-4}
export OMP_SCHEDULE='auto';

g++ -std=c++11 -w -Wall alg2.cpp -o $BUILD_DIR/$PROG_NAME -fopenmp || exit 1

PARALLEL=${1:-0}
./$BUILD_DIR/$PROG_NAME $PARALLEL 10000000 100000




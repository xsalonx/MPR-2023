#!/bin/bash

BUILD_DIR="build"
mkdir -p $BUILD_DIR

ALGO=$1
case $ALGO in
    0)
        ALGO=3
        PARALLEL=0;
    ;;
    2)
        PARALLEL=1;
    ;;
    3)
        PARALLEL=1;
    ;;
    *)
        echo "incorrect algo number"
        exit 1
    ;;
esac

ARR_SIZE=${2:-1000000}
BUCKETS_NO=${3:-1000}

PROG_NAME="bucket_alg$ALGO.bin"
SRC_NAME="bucket_alg$ALGO.cpp"

export OMP_NUM_THREADS=${OMP_NUM_THREADS:-14}
export OMP_SCHEDULE='auto';

g++ -std=c++11 -w -Wall $SRC_NAME -o $BUILD_DIR/$PROG_NAME -fopenmp || exit 1


./$BUILD_DIR/$PROG_NAME $PARALLEL $ARR_SIZE $BUCKETS_NO




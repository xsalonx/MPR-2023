#!/bin/bash

# export LC_ALL=C

BUILD_DIR="build"
mkdir -p $BUILD_DIR
PROG_NAME="prog.bin"

g++ -Wall main.c -o $BUILD_DIR/$PROG_NAME -fopenmp

SUBMEASURES_NO=${1:-10}
OUTFILE="openmp-measures.csv"

MAX_NSIZE='1000000' #00'
START_SIZE='100000'
STEP='100000'

cd $BUILD_DIR
echo -n "size"
for at in $(seq $SUBMEASURES_NO); do echo -n ";at$at"; done
echo
for (( s=$START_SIZE; s <= $MAX_NSIZE; s+=$STEP)); do
    echo -n "$s"
    for i in $(seq $SUBMEASURES_NO); do
        N_TIME=$(./$PROG_NAME 0 $MAX_NSIZE)
        P_TIME=$(./$PROG_NAME 1 $MAX_NSIZE)
        # echo -n ";$N_TIME|$P_TIME"
        R=$(echo "scale=2; $N_TIME / $P_TIME "| bc -l)
        echo -n "; $R"
    done
    echo ""
done

cd -
# echo N=$N_TIME
# echo P=$P_TIME
# R=$(echo "scale=20; $N_TIME / $P_TIME"| bc -l)
# echo R=$R
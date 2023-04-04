#!/bin/bash

# export LC_ALL=C

BUILD_DIR="build"
mkdir -p $BUILD_DIR
PROG_NAME="prog.bin"

SUBMEASURES_NO=${1:-10}
export OMP_NUM_THREADS=${OMP_NUM_THREADS:-4}

MAX_NSIZE='1000000' #00'
START_SIZE='100000'
STEP='100000'

rm build/*.csv

g++ -Wall main.c -o $BUILD_DIR/$PROG_NAME -fopenmp




cd $BUILD_DIR
env | grep OMP_ > meta-measures.env

get_sch_fn() {
    echo "openmp-measures_$*.csv"
}


mk_meas() {
    PARALLEL=$1
    out=$(get_sch_fn) # $OMP_SCHEDULE)
    SCH_T=$OMP_SCHEDULE
    if [[ $OMP_SCHEDULE == 'seq' ]]; then
        OMP_SCHEDULE="auto"
    fi

    echo -n "$s" >> $out
    for i in $(seq $SUBMEASURES_NO); do
        N_TIME=$(./$PROG_NAME $PARALLEL $s)
        echo -n ";$N_TIME" >> $out
    done
    echo -n ";$SCH_T" >> $out
    echo ";$CHUNK_SIZE_DIV" >> $out
}

out=$(get_sch_fn)
echo -n 'arr_size' >> "$out"
for i in $(seq 1 $SUBMEASURES_NO); do echo -n ";at$i" >> "$out"; done
echo -n ";schedule_type" >> "$out";
echo -n ";chunk_size_div" >> "$out";
echo "" >> "$out"

for (( s=$START_SIZE; s <= $MAX_NSIZE; s+=$STEP )); do
# sequential
    export OMP_SCHEDULE='seq'
    mk_meas 0
# parallel auto
    export OMP_SCHEDULE='auto'
    mk_meas 1
# parallel with chunks
    for sch_t in 'static' 'dynamic' 'guided'; do
        for div in 1 $(seq 2 2 8); do
            chunk_size=$(($s / $OMP_NUM_THREADS / $div))
            export OMP_SCHEDULE="$sch_t,$chunk_size"
            export CHUNK_SIZE_DIV=$(($OMP_NUM_THREADS * $div))
            mk_meas 1
        done
    done
done

cd -

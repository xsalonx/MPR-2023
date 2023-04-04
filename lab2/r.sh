#!/bin/bash

BUILD_DIR="build"
mkdir -p $BUILD_DIR
PROG_NAME="prog.bin"

export OMP_NUM_THREADS=${OMP_NUM_THREADS:-4}
export OMP_SCHEDULE='NONE';

MAX_NSIZE='1000000' #00'
START_SIZE='100000'
STEP='100000'




while [[ $# -gt 0 ]]; do
    case $1 in 
        --num_threads_div_proportional)
            lOMP_DIV_MULTIPLIER=$OMP_NUM_THREADS
            OUT_FILE_SUFIX='-num-thread-div-proportional'
            shift;
        ;;
        --sub_m)
            lOMP_SUBMEASURES_NO=$2
            shift 2;
        ;;
        -m)
            lOMP_DIV_MIN=$2
            shift 2;
        ;;
        -s)
            lOMP_DIV_STEP=$2
            shift 2;
        ;;
        -S)
            lOMP_DIV_MAX=$2
            shift 2;
        ;;
        --out)
            OUT_FILE_BASE_NAME=$2
            shift 2;
        ;;
        *)
            echo "incorrect arg: $1"
            exit 254
        ;;
    esac
done

export lOMP_DIV_MULTIPLIER=${lOMP_DIV_MULTIPLIER:-1}
export lOMP_SUBMEASURES_NO=${lOMP_SUBMEASURES_NO:-10}

export lOMP_DIV_MIN=${lOMP_DIV_MIN:-1}
export lOMP_DIV_STEP=${lOMP_DIV_STEP:-1}
export lOMP_DIV_MAX=${lOMP_DIV_MAX:-3}

OUT_FILE_BASE_NAME=${OUT_FILE_BASE_NAME:-openmp-measures}$OUT_FILE_SUFIX

# rm build/*.csv
g++ -Wall main.c -o $BUILD_DIR/$PROG_NAME -fopenmp




cd $BUILD_DIR
env_file="$OUT_FILE_BASE_NAME.env"
env | grep OMP_ > $env_file
cat $env_file

get_sch_fn() {
    echo "$OUT_FILE_BASE_NAME$1.csv"
}


mk_meas() {
    PARALLEL=$1
    out=$(get_sch_fn) # $OMP_SCHEDULE)
    SCH_T=$OMP_SCHEDULE
    if [[ $OMP_SCHEDULE == 'seq' ]]; then
        OMP_SCHEDULE="auto"
    fi

    echo -n "$s" >> $out
    for i in $(seq $lOMP_SUBMEASURES_NO); do
        N_TIME=$(./$PROG_NAME $PARALLEL $s)
        echo -n ";$N_TIME" >> $out
    done
    echo -n ";$sch_t" >> $out
    echo -n ";$SCH_T" >> $out
    echo -n ";$chunk_size" >> $out
    echo ";$CHUNK_SIZE_DIV" >> $out
}

out=$(get_sch_fn)
rm -f $out
echo -n 'arr_size' >> "$out"
for i in $(seq 1 $lOMP_SUBMEASURES_NO); do echo -n ";at$i" >> "$out"; done
echo -n ";schedule_type" >> "$out";
echo -n ";OMP_SCHEDULE" >> "$out";
echo -n ";chunk_size" >> "$out";
echo -n ";chunk_size_div" >> "$out";
echo "" >> "$out"

for (( s=$START_SIZE; s <= $MAX_NSIZE; s+=$STEP )); do
# sequential
    chunk_size='0'
    CHUNK_SIZE_DIV='0'
    OMP_SCHEDULE='seq'
    sch_t='seq'
    mk_meas 0
# parallel auto
    chunk_size='0'
    CHUNK_SIZE_DIV='0'
    OMP_SCHEDULE='auto'
    sch_t='auto'
    mk_meas 1
# parallel with chunks

    # chunks size = 1
    for sch_t in 'static' 'dynamic' 'guided'; do
        chunk_size=1
        OMP_SCHEDULE="$sch_t,$chunk_size"
        CHUNK_SIZE_DIV=0
        mk_meas 1
    done

    # chunk_size = s / num_threads / div 
    for sch_t in 'static' 'dynamic' 'guided'; do
        for div in $(seq $lOMP_DIV_MIN $lOMP_DIV_STEP $lOMP_DIV_MAX); do
            chunk_size=$(($s / $lOMP_DIV_MULTIPLIER / $div))
            OMP_SCHEDULE="$sch_t,$chunk_size"
            CHUNK_SIZE_DIV=$(($lOMP_DIV_MULTIPLIER * $div))
            mk_meas 1
        done
    done
done

cd -

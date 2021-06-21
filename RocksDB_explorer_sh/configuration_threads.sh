#!/bin/bash

DEV_PATH=$1
RESULT_PATH="../result_txt/"
VALUE_SIZE=512
NUM=5000000
BENCHMARK=fillrandom
THREAD=$2

print > $RESULT_PATH/background_thread_result.txt
echo ========== Background thread num = $THREAD =========== >> $RESULT_PATH/background_thread_result.txt 
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_background_jobs=$THREAD >> $RESULT_PATH/background_thread_result.txt


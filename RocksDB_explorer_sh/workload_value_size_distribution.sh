#!/bin/bash

DEV_PATH=$1
RESULT_PATH="../result_txt/"
KEY_SIZE=16
NUM=5000000
BENCHMARK=$2
DIST=$3
MAX=$4
MIN=$5

print > $RESULT_PATH/value_distribution_result.txt
echo ========== key size = $KEY_SIZE, value size = $VALUE_SIZE =========== >> $RESULT_PATH/value_distribution_result.txt 
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --key_size=$KEY_SIZE --value_size_distribution_type=$DIST --value_size_min=$MIN --value_size_max=$MAX --num=$NUM >> $RESULT_PATH/value_distribution_result.txt


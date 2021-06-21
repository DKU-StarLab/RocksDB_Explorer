#!/bin/bash

DEV_PATH=$1
RESULT_PATH="../result_txt/"
VALUE_SIZE=512
NUM=5000000
BENCHMARK=fillrandom

print > $RESULT_PATH/policy_wal_result.txt
echo ========== Operation WAL on =========== >> $RESULT_PATH/policy_wal_result.txt 
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --disable_wal=false >> $RESULT_PATH/policy_wal_result.txt

echo >> $RESULT_PATH/policy_wal_result.txt
echo ========== Operation WAL off =========== >> $RESULT_PATH/policy_wal_result.txt
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --disable_wal=true >> $RESULT_PATH/policy_wal_result.txt

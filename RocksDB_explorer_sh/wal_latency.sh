#!/bin/bash

DEV_PATH=$1
RESULT_PATH="../result_txt/"
VALUE_SIZE=512
NUM=5000000
BENCHMARK=fillrandom

print > $RESULT_PATH/wal_on_latency_result.txt
echo ========== Operation WAL on for Latency =========== >> $RESULT_PATH/wal_on_latency_result.txt 
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --histogram --value_size=$VALUE_SIZE --num=$NUM --disable_wal=false >> $RESULT_PATH/wal_on_latency_result.txt

print > $RESULT_PATH/wal_off_latency_result.txt
echo ========== Operation WAL off for Latency =========== >> $RESULT_PATH/wal_off_latency_result.txt
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --histogram --value_size=$VALUE_SIZE --num=$NUM --disable_wal=true >> $RESULT_PATH/wal_off_latency_result.txt

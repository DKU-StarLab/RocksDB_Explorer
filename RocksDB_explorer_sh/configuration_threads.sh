#!/bin/bash

DEV_PATH="/home/choigunhee/hojin/mnt"
RESULT_PATH="/home/choigunhee/hojin/RocksDB_Explorer/result_txt/"
VALUE_SIZE=512
NUM=5000000
BENCHMARK=fillrandom

print > $RESULT_PATH/background_thread_result.txt
echo ========== Background thread num = 1 =========== >> $RESULT_PATH/background_thread_result.txt 
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_background_jobs=1 >> $RESULT_PATH/background_thread_result.txt

echo >> $RESULT_PATH/background_thread_result.txt
echo ========== Background thread num = 2 =========== >> $RESULT_PATH/background_thread_result.txt
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_background_jobs=2 >> $RESULT_PATH/background_thread_result.txt

echo >> $RESULT_PATH/background_thread_result.txt
echo ========== Background thread num = 4 =========== >> $RESULT_PATH/background_thread_result.txt
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_background_jobs=4 >> $RESULT_PATH/background_thread_result.txt

echo >> $RESULT_PATH/background_thread_result.txt
echo ========== Background thread num = 8 =========== >> $RESULT_PATH/background_thread_result.txt
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_background_jobs=8 >> $RESULT_PATH/background_thread_result.txt

echo >> $RESULT_PATH/background_thread_result.txt
echo ========== Background thread num = 16 =========== >> $RESULT_PATH/background_thread_result.txt
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_background_jobs=16 >> $RESULT_PATH/background_thread_result.txt


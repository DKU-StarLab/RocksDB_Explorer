#!/bin/bash

DEV_PATH="/home/choigunhee/hojin/mnt"
RESULT_PATH="/home/choigunhee/hojin/RocksDB_Explorer/result_txt/"
VALUE_SIZE=512
NUM=5000000
BENCHMARK=fillrandom
MEM_SIZE=67108864

print > $RESULT_PATH/memtable_result.txt
echo ========== memtable num = 1 =========== >> $RESULT_PATH/memtable_result.txt 
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_write_buffer_number=1 --write_buffer_size=$MEM_SIZE >> $RESULT_PATH/memtable_result.txt

echo >> $RESULT_PATH/memtable_result.txt
echo ========== memtable num = 2 =========== >> $RESULT_PATH/memtable_result.txt                  
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_write_buffer_number=2 --write_buffer_size=$MEM_SIZE >> $RESULT_PATH/memtable_result.txt

echo >> $RESULT_PATH/memtable_result.txt
echo ========== memtable num = 4 =========== >> $RESULT_PATH/memtable_result.txt                  
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_write_buffer_number=4 --write_buffer_size=$MEM_SIZE >> $RESULT_PATH/memtable_result.txt

echo >> $RESULT_PATH/memtable_result.txt
echo ========== memtable num = 8 =========== >> $RESULT_PATH/memtable_result.txt                  
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_write_buffer_number=8 --write_buffer_size=$MEM_SIZE >> $RESULT_PATH/memtable_result.txt

echo >> $RESULT_PATH/memtable_result.txt
echo ========== memtable num = 16 =========== >> $RESULT_PATH/memtable_result.txt                  
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_write_buffer_number=16 --write_buffer_size=$MEM_SIZE >> $RESULT_PATH/memtable_result.txt


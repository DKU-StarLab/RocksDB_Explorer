#!/bin/bash

DEV_PATH=$1
RESULT_PATH="../result_txt/"
VALUE_SIZE=512
NUM=5000000
BENCHMARK=fillrandom
MEM_NUM=$2
let MEM_SIZE=$3*1024*1024

print > $RESULT_PATH/memtable_result.txt
echo ========== memtable num = $MEM_NUM, memtable size = $MEM_SIZE =========== >> $RESULT_PATH/memtable_result.txt 
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --max_write_buffer_number=$MEM_NUM --write_buffer_size=$MEM_SIZE >> $RESULT_PATH/memtable_result.txt


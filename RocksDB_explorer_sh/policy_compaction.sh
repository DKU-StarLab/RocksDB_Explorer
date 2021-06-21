#!/bin/bash

DEV_PATH=$1
RESULT_PATH="../result_txt/"
VALUE_SIZE=512
NUM=5000000
BENCHMARK=fillrandom,levelstats

print > $RESULT_PATH/policy_compaction_result.txt
echo ========== Operation Compaction on =========== >> $RESULT_PATH/policy_compaction_result.txt 
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --disable_auto_compactions=false --level0_file_num_compaction_trigger=-1 >> $RESULT_PATH/policy_compaction_result.txt

echo >> $RESULT_PATH/policy_compaction_result.txt
echo ========== Operation Compaction off =========== >> $RESULT_PATH/policy_compaction_result.txt
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=$BENCHMARK --value_size=$VALUE_SIZE --num=$NUM --disable_auto_compactions=true --level0_file_num_compaction_trigger=-1 >> $RESULT_PATH/policy_compaction_result.txt

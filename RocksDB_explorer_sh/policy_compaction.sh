#!/bin/bash

DEV_PATH="/home/choigunhee/hojin/mnt"
RESULT_PATH="/home/choigunhee/hojin/rocksdb/result_txt/"

print > $RESULT_PATH/policy_compaction_on_result.txt
echo ========== Operation Compaction on =========== >> $RESULT_PATH/policy_compaction_on_result.txt 
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=fillrandom,levelstats --value_size=512 --num=5000000 --disable_auto_compactions=false --level0_file_num_compaction_trigger=-1 >> $RESULT_PATH/policy_compaction_on_result.txt

print > $RESULT_PATH/policy_compaction_off_result.txt
echo ========== Operation Compaction off =========== >> $RESULT_PATH/policy_compaction_off_result.txt
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=fillrandom,levelstats --value_size=512 --num=5000000 --disable_auto_compactions=true --level0_file_num_compaction_trigger=-1 >> $RESULT_PATH/policy_compaction_off_result.txt

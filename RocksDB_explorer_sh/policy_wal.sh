#!/bin/bash

DEV_PATH="/home/choigunhee/hojin/mnt"
RESULT_PATH="/home/choigunhee/hojin/rocksdb/result_txt/"

print > $RESULT_PATH/policy_wal_on_result.txt
echo ========== Operation WAL on =========== >> $RESULT_PATH/policy_wal_on_result.txt 
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=fillrandom --value_size=512 --num=5000000 --disable_wal=false >> $RESULT_PATH/policy_wal_on_result.txt

print > $RESULT_PATH/policy_wal_off_result.txt
echo ========== Operation WAL off =========== >> $RESULT_PATH/policy_wal_off_result.txt
../db_bench --db=$DEV_PATH --use_direct_io_for_flush_and_compaction=true --benchmarks=fillrandom --value_size=512 --num=5000000 --disable_wal=true >> $RESULT_PATH/policy_wal_off_result.txt

# RocksDB Explorer 

## RocksDB Festival : Quantitative Approaches by DKU StarLab

This is a place for stduying RocksDB (Facebook) by Dankook University
- Writer : Hojin Shin
- Laboratory : Embedded System Lab. in Dankook University
- 2021 DKU RocksDB Festival Lecture Information [link](https://github.com/DKU-StarLab/RocksDB_Festival)

## How to use This Github

### 1. Setting Environment for experiments

#### Follow the instructions below 

        git clone https://github.com/DKU-StarLab/RocksDB_Explorer.git
        
        cd RocksDB_Explorer
        
        make db_bench (It will take some time)

You need to edit your DEV_PATH before starting experiments.

### 2. Policy-related experiments

2.1 What if we turn off the WAL (Write-Ahead-Log)

* Throughput

         ./policy_wal.sh [YOUR_DEV_PATH]
         
         Ex) ./policy_wal.sh ../mnt

* Latency
        
        ./wal_latency.sh [YOUR_DEV_PATH]
        
        Ex) ./wal_latency.sh ../mnt
        
        cd ../python_parser
        
        python3 latency_parser.py [RESULT_TXT] [RESULT_CSV]
        
        Ex) python3 latency_parser.py ../result_txt/wal_on_latency_result.txt ../parsing_csv/wal_on_latecny.csv
        
2.2 What if we turn off the Compaction

* Throughput

        ./policy_compaction.sh [YOUR_DEV_PATH]
        
        Ex) ./policy_compaction.sh ../mnt

* Latency

        ./compaction_latency.sh [YOUR_DEV_PATH]
        
        Ex) ./compaction_latency.sh ../mnt
        
        cd ../python_parser
        
        python3 latency_parser.py [RESULT_TXT] [RESULT_CSV]
        
        Ex) python3 latency_parser.py ../result_txt/compaction_on_latency_result.txt ../parsing_csv/compaction_on_latecny.csv

2.2 Explore files that are used and created during compaction

        Not yet, it will be update soon
        
2.3 Details explanation about options

* WAL (Write-Ahead-Log)
        
        - disalbe_wal = true/false (Control WAL operation)

* Compaction

        - disable_auto_compactions = true/false (Control automatic compactions)
        - level0_file_num_compaction_trigger = integer (Number of files to trigger level-0 compaction. A value < 0 means that level-0 compaction will not be triggered by number of files at all)

### 3. Configuration-related experiments

3.1 What if we change the number of background thread

        ./configuration_thread.sh [YOUR_DEV_PATH] [NUM_THREAD]
        
        Ex) ./configuration_thread.sh ../mnt 16

3.2 What if we change the memtable setting

        ./configuration_memtable.sh [YOUR_DEV_PATH] [MEM_NUM] [MEM_SIZE]
        
        Ex) ./configuration_memtable.sh ../mnt 2 64
        
        MEM_SIZE is in MB, 64 means 64MB

3.3 Details explanation about options

        - max_background_jobs = integer (Maximum number of concurrent background jobs, compactions and flushes)
        - max_write_buffer_number = integer (Maximum number of memtable)
        - write_buffer_size = integer (byte, Memtable size)

### 4. Workload-related experiments

4.1 + 2 What if the key-value size and the key pattern are changed

        ./workload_key_value_size.sh [YOUR_DEV_PATH] [KEY_SIZE] [VALUE_SIZE] [BENCHMARK]
        
        Ex) ./workload_key_value_size.sh ../mnt 16 100 fillrandom
        
        Value distribution is fixed in this configure.

4.3 What if the value size distribution is changed

        ./workload_value_size_distribution.sh [YOUR_DEV_PATH] [BENCHMARK] [DISTRIBUTION_TYPE] [VALUE_MAX_SIZE] [VALUE_MIN_SIZE]
        
        Ex) ./workload_value_size_distribution.sh ../mnt fillrandom uniform 1024 100

4.4 Details explanation about options

        - key_size = integer (Control key size)
        - value_size = integer (Control value size)
        - value_size_distribution = fixed/uniform/normal
        - value_size_min, value_size_manx = integer (Control value max, min size)

### 5. Source-level Analysis

5.1 Source code level exploration

5.1.1 Write Operation Flow

        If you want to see write operation flow, change value 0 to 1 in options/cf_options.h - DB_WRITE_FLOW variable
        
        Then, make db_bench
        
        While do db_bench, you can see write operation flow (Functions) - What functions are you working with

5.1.2 Compaction Operation Flow

        If you want to see compaction operation flow, change value 0 to 1 in options/cf_options.h - DB_LVL_COMPACTION_FLOW or DB_UNI_COMPACTION_FLOW variable
        
        DB_LVL_COMPACTION_FLOW = Leveled compaction / DB_UNI_COMPACTION_FLOW = Universal compaction
        
        db_bench option = compation_style (0 = Leveled, 1 = Universal, 2 = FIFO)
        
        Then, make db_bench
        
        While do db_bench, you can see compaction(LVL, UNI) operation flow (Functions) - What functions are you working with

5.1.3 Read Operation Flow

        If you want to see Read(ReadRandom only) operation flow, change value 0 to 1 in options/cf_options.h - DB_READ_FLOW variable

        Then, make db_bench

        While do db_bench, you can see read(readrandom) operation flow (Functions) - What functions are you working with

#### Note: Not all functions executed are output. Notice that only the important functions are printed.

## Note: Our experimental environment

#### Author Experiment Environment

    Operatring System : Ubuntu 18.04
  
    CPU : 8 * Intel(R) Core(TM) i7-6700 CPU @ 3.40Hz
  
    RAM : 8GB
  
    SSD : Samsung EVO 860 Pro 1TB
  
    RocksDB ver. 6.14 -> 6.21

If you have any problem to run our explorer, please leave your comment in the Issue section

## License

RocksDB is dual-licensed under both the GPLv2 (found in the COPYING file in the root directory) and Apache 2.0 License (found in the LICENSE.Apache file in the root directory).  You may select, at your option, one of the above-listed licenses.

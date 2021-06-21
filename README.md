# RocksDB Explorer 

## RocksDB Festival : Quantitative Approaches by DKU StarLab

This is a place for stduying RocksDB (Facebook) by Dankook University
- Writer : Hojin Shin
- Laboratory : Embedded System Lab. in Dankook University
- 2021 DKU RocksDB Festival Lecture Information [link]()

## How to use This Github

### 1. Setting Environment for experiments

#### Follow the instructions below 

        git clone https://github.com/DKU-StarLab/RocksDB_Explorer.git
        
        cd RocksDB_Explorer
        
        make db_bench (It will take some time)

You need to edit your DEV_PATH before starting experiments.

### 2. Policy-related experiments

2.1 What if we turn off the WAL (Write-Ahead-Log)

2.2 What if we turn off the Compaction

2.2 Explore files that are used and created during compaction

### 3. Configuration-related experiments

3.1 What if we change the number of background thread

3.2 What if we change the memtable setting

3.3 Details explanation about options

### 4. Workload-related experiments

4.1 What if the key-value size is changed

4.2 What if the key pattern is changed

4.3 What if the value size distribution is changed

4.4 Details explanation about options

### 5. Source-level Analysis

5.1 Source code level exploration

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

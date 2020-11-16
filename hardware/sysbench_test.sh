#!/bin/sh

mkdir sysbench_test
cd sysbench_test

#1 Benchmark CPU
#--------------------------------------------------------------#
sysbench cpu --cpu-max-prime=20000 run > result_cpu
#--------------------------------------------------------------#


#2 Benchmark File IO
#--------------------------------------------------------------#
sysbench fileio --file-total-size=15G prepare
sysbench fileio --file-total-size=15G --file-test-mode=rndrw run > result_fileIO
sysbench fileio --file-total-size=15G cleanup
#--------------------------------------------------------------#


#3 Benchmark Memory
#--------------------------------------------------------------#
sysbench memory run > result_mem
#--------------------------------------------------------------#


#4 Benchmark threads
#--------------------------------------------------------------#
sysbench threads run > result_threads
#--------------------------------------------------------------#


#5 Benchmark mutex
#--------------------------------------------------------------#
sysbench mutex run > result_mutex
#--------------------------------------------------------------#

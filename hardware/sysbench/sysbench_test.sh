#!/bin/bash

# init workspace
#--------------------------------------------------------------#
# 取得Shell所在目錄
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LOGDIR=${DIR}/benchmark_`hostname`

if [ ! -d ${LOGDIR} ]; then 
    mkdir -p ${LOGDIR}
fi

ResultCPU=${LOGDIR}/result_cpu
ResultFIO=${LOGDIR}/result_fileIO
ResultMEM=${LOGDIR}/result_mem
ResultTHD=${LOGDIR}/result_thread
ResultMTX=${LOGDIR}/result_mutex

function Fn_RunCmd()
{
    local cmd=$1
    echo "---------------------------------------------"
    echo ${cmd}
    echo "---------------------------------------------"
    ${cmd}
}

#--------------------------------------------------------------#

echo "1 Benchmark CPU "
#1 Benchmark CPU
#--------------------------------------------------------------#
Fn_RunCmd "sysbench cpu --cpu-max-prime=20000 run" > ${ResultCPU}
#--------------------------------------------------------------#

echo "2 Benchmark File IO"
#2 Benchmark File IO
#--------------------------------------------------------------#
Fn_RunCmd "sysbench fileio --file-total-size=15G prepare" > ${ResultFIO}
Fn_RunCmd "sysbench fileio --file-total-size=15G --file-test-mode=rndrw run" >> ${ResultFIO}
Fn_RunCmd "sysbench fileio --file-total-size=15G cleanup" >> ${ResultFIO}
#--------------------------------------------------------------#


echo "3 Benchmark Memory"
#3 Benchmark Memory
#--------------------------------------------------------------#
Fn_RunCmd "sysbench memory run" > ${ResultMEM}
Fn_RunCmd "sysbench memory run --num-threads=5" >> ${ResultMEM}
#--------------------------------------------------------------#


echo "4 Benchmark threads"
#4 Benchmark threads
#--------------------------------------------------------------#
Fn_RunCmd "sysbench threads run " > ${ResultTHD}
#--------------------------------------------------------------#


echo "5 Benchmark mutex"
#5 Benchmark mutex
#--------------------------------------------------------------#
Fn_RunCmd "sysbench mutex run --mutex-locks=1000000 " > ${ResultMTX}
#--------------------------------------------------------------#

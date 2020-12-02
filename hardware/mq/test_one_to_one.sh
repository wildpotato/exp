#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LOGDIR=${DIR}/benchmark_`hostname`/one_to_one
if [ ! -d ${LOGDIR} ]; then
    mkdir -p ${LOGDIR}
fi

exe_cnt=10000
pq_msg_size=1024     # posix queue message size
pq_max_num=10        # posix queue max number of messages on queue

sysNonBlockSendTime=sysNonBlockSendTime.out
sysNonBlockRecvTime=sysNonBlockRecvTime.out
sysNonBlockResult=sysNonBlockResult.out
sysBlockSendTime=sysBlockSendTime.out
sysBlockRecvTime=sysBlockRecvTime.out
sysBlockResult=sysBlockResult.out
posNonBlockSendTime=posNonBlockSendTime.out
posNonBlockRecvTime=posNonBlockRecvTime.out
posNonBlockResult=posNonBlockResult.out
posBlockSendTime=posBlockSendTime.out
posBlockRecvTime=posBlockRecvTime.out
posBlockResult=posBlockResult.out

function run_cmd_fore_ground()
{
    local cmd=$1
    echo "---------------------------------------------"
    echo ${cmd}
    ${cmd}
    echo "---------------------------------------------"
}

function run_cmd_back_ground()
{
    local cmd=$1
    echo "---------------------------------------------"
    echo ${cmd}
    ${cmd} &
    echo "---------------------------------------------"
}

function run_sys_block()
{
    run_cmd_back_ground "./sysV -e ${exe_cnt} -m serv -t recv -o ${sysBlockRecvTime} -b"
    run_cmd_fore_ground "./sysV -e ${exe_cnt} -m cli -t send -o ${sysBlockSendTime}"
    run_cmd_fore_ground "./compute -e ${exe_cnt} -s ${sysBlockSendTime} -r ${sysBlockRecvTime} -o ${sysBlockResult}"
}

function run_sys_non_block()
{
    run_cmd_back_ground "./sysV -e ${exe_cnt} -m serv -t recv -o ${sysNonBlockRecvTime}"
    run_cmd_fore_ground "./sysV -e ${exe_cnt} -m cli -t send -o ${sysNonBlockSendTime}"
    run_cmd_fore_ground "./compute -e ${exe_cnt} -s ${sysNonBlockSendTime} -r ${sysNonBlockRecvTime} -o ${sysNonBlockResult}"
}

function run_pos_block()
{
    run_cmd_back_ground "./posix -e ${exe_cnt} -m serv -t recv -s ${pq_msg_size} -n ${pq_max_num} -o ${posBlockRecvTime} -b"
    run_cmd_fore_ground "./posix -e ${exe_cnt} -m cli -t send -s ${pq_msg_size} -n ${pq_max_num} -o ${posBlockSendTime}"
    run_cmd_fore_ground "./compute -e ${exe_cnt} -s ${posBlockSendTime} -r ${posBlockRecvTime} -o ${posBlockResult}"
}

function run_pos_non_block()
{
    run_cmd_back_ground "./posix -e ${exe_cnt} -m serv -t recv -s ${pq_msg_size} -n ${pq_max_num} -o ${posNonBlockRecvTime}"
    run_cmd_fore_ground "./posix -e ${exe_cnt} -m cli -t send -s ${pq_msg_size} -n ${pq_max_num} -o ${posNonBlockSendTime}"
    run_cmd_fore_ground "./compute -e ${exe_cnt} -s ${posNonBlockSendTime} -r ${posNonBlockRecvTime} -o ${posNonBlockResult}"
}

function run_setup()
{
    cp compute ${LOGDIR}
    cp sysV ${LOGDIR}
    cp posix ${LOGDIR}
    cd ${LOGDIR}
}

run_setup
run_sys_block
#run_sys_non_block
#run_pos_block
#run_pos_non_block
cd ${DIR}

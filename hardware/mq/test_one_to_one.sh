#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LOGDIR=${DIR}/benchmark_`hostname`/one_to_one
if [ ! -d ${LOGDIR} ]; then
    mkdir -p ${LOGDIR}
fi

exe_cnt=10

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

function run_sys_serv_nonblock()
{
    run_cmd "./sysV -e ${exe_cnt} -m serv -t recv -o ${sysNonBlockRecvTime}"
}

function run_sys_cli_nonblock()
{
    run_cmd "./sysV -e ${exe_cnt} -m cli -t send -o ${sysSendTime}"
}

function run_sys_serv_block()
{
    run_cmd_back_ground "./sysV -e ${exe_cnt} -m serv -t recv -o ${sysBlockRecvTime} -b"
}

function run_sys_cli_block()
{
    run_cmd_fore_ground "./sysV -e ${exe_cnt} -m cli -t send -o ${sysBlockSendTime}"
}

#TODO  run_sys_serv_nonblock
#TODO  run_sys_cli_nonblock
cp sysV ${LOGDIR}
cd ${LOGDIR}
run_sys_serv_block
run_sys_cli_block
cd ${DIR}

#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LOGDIR=${DIR}/benchmark_`hostname`
if [ ! -d ${LOGDIR} ]; then
    mkdir -p ${LOGDIR}
fi

exe_cnt=10

sysNonBlockSendTime=${LOGDIR}/sysNonBlockSendTime.out
sysNonBlockRecvTime=${LOGDIR}/sysNonBlockRecvTime.out
sysNonBlockResult=${LOGDIR}/sysNonBlockResult.out
sysBlockSendTime=${LOGDIR}/sysBlockSendTime.out
sysBlockRecvTime=${LOGDIR}/sysBlockRecvTime.out
sysBlockResult=${LOGDIR}/sysBlockResult.out
posNonBlockSendTime=${LOGDIR}/posNonBlockSendTime.out
posNonBlockRecvTime=${LOGDIR}/posNonBlockRecvTime.out
posNonBlockResult=${LOGDIR}/posNonBlockResult.out
posBlockSendTime=${LOGDIR}/posBlockSendTime.out
posBlockRecvTime=${LOGDIR}/posBlockRecvTime.out
posBlockResult=${LOGDIR}/posBlockResult.out

function run_cmd()
{
    local cmd=$1
    echo "---------------------------------------------"
    echo ${cmd}
    echo "---------------------------------------------"
    ${cmd}

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
    run_cmd "./sysV -e ${exe_cnt} -m serv -t recv -o ${sysBlockRecvTime} -b"
}

function run_sys_cli_block()
{
    run_cmd "./sysV -e ${exe_cnt} -m cli -t send -o ${sysBlockSendTime}"
}

#TODO  run_sys_serv_nonblock
#TODO  run_sys_cli_nonblock
run_sys_serv_block
run_sys_cli_block

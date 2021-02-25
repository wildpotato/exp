#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
#LOGDIR=${DIR}/benchmark_`hostname`/avg

exe_cnt=1000000
num_of_rounds=11
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

coreCnt=`nproc --all`
TASKSET0="taskset -c 0"
TASKSET1="taskset -c 0"
if [ ${coreCnt} -gt 1 ];then
    TASKSET1="taskset -c 1"
fi

function run_cmd_fore_ground()
{
    local cmd=$1
    echo "---------------------------------------------"
    echo ${cmd}
    ${cmd}
    echo "---------------------------------------------"
    sleep 1
}

function run_cmd_back_ground()
{
    local cmd=$1
    echo "---------------------------------------------"
    echo ${cmd}
    ${cmd} > logfile 2>&1 &
    echo "---------------------------------------------"
    sleep 1
}

function run_sys_block()
{
    if [ $# -lt 3 ];then
        echo "argumeent not enougth."
        return
    fi
    local cnt=$1
    local type=$2
    local round=$3

    if [ ${type} == "avg" ]; then
        run_cmd_back_ground "${TASKSET0} ./sysV -e ${cnt} -m serv -t avg -o ${sysBlockRecvTime} -b -d ${round}"
        run_cmd_fore_ground "${TASKSET1} ./sysV -e ${cnt} -m cli -t avg -o ${sysBlockSendTime} -b -d ${round}"
        type="allinone"
    else
        run_cmd_back_ground "${TASKSET0} ./sysV -e ${cnt} -m serv -t recv -o ${sysBlockRecvTime} -b -d ${round}"
        run_cmd_fore_ground "${TASKSET1} ./sysV -e ${cnt} -m cli -t send -o ${sysBlockSendTime} -b -d ${round}"
        type="one2one"
    fi
    run_cmd_fore_ground "./compute -e ${cnt} -s ${sysBlockSendTime} -r ${sysBlockRecvTime} -o ${sysBlockResult} -m ${type} -d ${round}"
    cat ${sysBlockResult}
}

function run_sys_non_block()
{
    if [ $# -lt 3 ];then
        echo "argumeent not enougth."
        return
    fi
    local cnt=$1
    local type=$2
    local round=$3

    if [ ${type} == "avg" ]; then
        run_cmd_back_ground "${TASKSET0} ./sysV -e ${cnt} -m serv -t avg -o ${sysNonBlockRecvTime} -d ${round}"
        run_cmd_fore_ground "${TASKSET1} ./sysV -e ${cnt} -m cli -t avg -o ${sysNonBlockSendTime} -d  ${round}"
        type="allinone"
    else
        run_cmd_back_ground "${TASKSET0} ./sysV -e ${cnt} -m serv -t recv -o ${sysNonBlockRecvTime} -d ${round}"
        run_cmd_fore_ground "${TASKSET1} ./sysV -e ${cnt} -m cli -t send -o ${sysNonBlockSendTime} -d ${round}"
        type="one2one"
    fi
    run_cmd_fore_ground "./compute -e ${cnt} -s ${sysNonBlockSendTime} -r ${sysNonBlockRecvTime} -o ${sysNonBlockResult} -m ${type} -d ${round}"
    cat ${sysNonBlockResult}
}

function run_pos_block()
{
    if [ $# -lt 2 ];then
        echo "argumeent not enougth."
        return
    fi
    local cnt=$1
    local type=$2

    if [ ${type} == "avg" ]; then
        run_cmd_back_ground "${TASKSET0} ./posix -e ${cnt} -m serv -t avg -s ${pq_msg_size} -n ${pq_max_num} -o ${posBlockRecvTime} -b"
        run_cmd_fore_ground "${TASKSET1} ./posix -e ${cnt} -m cli -t avg -s ${pq_msg_size} -n ${pq_max_num} -o ${posBlockSendTime} -b"
    else
        run_cmd_back_ground "${TASKSET0} ./posix -e ${cnt} -m serv -t recv -s ${pq_msg_size} -n ${pq_max_num} -o ${posBlockRecvTime} -b"
        run_cmd_fore_ground "${TASKSET1} ./posix -e ${cnt} -m cli -t send -s ${pq_msg_size} -n ${pq_max_num} -o ${posBlockSendTime} -b"
    fi
    run_cmd_fore_ground "./compute -e ${cnt} -s ${posBlockSendTime} -r ${posBlockRecvTime} -o ${posBlockResult} -m ${type}"
    cat ${posBlockResult}
}

function run_pos_non_block()
{
    if [ $# -lt 2 ];then
        echo "argumeent not enougth."
        return
    fi
    local cnt=$1
    local type=$2

    if [ ${type} == "avg" ]; then
        run_cmd_back_ground "${TASKSET0} ./posix -e ${cnt} -m serv -t avg -s ${pq_msg_size} -n ${pq_max_num} -o ${posNonBlockRecvTime}"
        run_cmd_fore_ground "${TASKSET1} ./posix -e ${cnt} -m cli -t avg -s ${pq_msg_size} -n ${pq_max_num} -o ${posNonBlockSendTime}"
    else
        run_cmd_back_ground "${TASKSET0} ./posix -e ${cnt} -m serv -t recv -s ${pq_msg_size} -n ${pq_max_num} -o ${posNonBlockRecvTime}"
        run_cmd_fore_ground "${TASKSET1} ./posix -e ${cnt} -m cli -t send -s ${pq_msg_size} -n ${pq_max_num} -o ${posNonBlockSendTime}"
    fi
    run_cmd_fore_ground "./compute -e ${cnt} -s ${posNonBlockSendTime} -r ${posNonBlockRecvTime} -o ${posNonBlockResult} -m ${type}"
    cat ${posNonBlockResult}
}

function run_setup()
{
    if [ $# -lt 1 ];then
        echo "argumeent not enougth."
        return
    fi

    local logFolder=${DIR}/benchmark_`hostname`/$1
    if [ ! -d ${logFolder} ]; then
        mkdir -p ${logFolder}
    fi

    cp compute ${logFolder}
    cp sysV ${logFolder}
    cp posix ${logFolder}
    cd ${logFolder}

    echo $logFolder
}

# ----------------------------------------------------------------------
# get parameter from command
# ----------------------------------------------------------------------
TestCase=""
TestType=""
function Usage() {
    echo -e "\nUsage:\n  ./`basename $0` [arguments] \n"
    echo -e "-case : ipcq_block      (System V queue blocking mode)"
    echo -e "      : ipcq            (System V queue non-blocking mode)"
    echo -e "      : posq_block      (POSIX queue blocking mode)"
    echo -e "      : posq            (POSIX queue non-blocking mode)\n"
    echo -e "-type : avg/detail      ('avg' records time before and after BATCH send/receive"
    echo -e "                         'detail' records time before and after EVERY send/receive)\n"
    echo -e "-cnt  : execution count (number of send/receive calls during each round)\n"
    echo -e "-r    : rounds          (each round will execute cnt times of send/receive)\n"
    echo -e "Example usage:\n ./`basename $0` -case \"ipcq\" -type \"detail\" -cnt 1000 -r 10\n"
    echo -e "Explanation: the above command runs system V queue bench test for 10 rounds where each round"
    echo -e "             consists of 1000 send and receive pair and the timing is the average time elapsed"
    echo -e "             from send to corresponding receive is done; note that the first round is not taken"
    echo -e "             into account during the computation of the result\n"
}

until [ -z "$1" ]
do
    case $1 in
        -case )
            shift
            TestCase=$1
            ;;
        -cnt )
            shift
            exe_cnt=$1
            ;;
        -type )
            shift
            if [ $1 == "detail" ]; then
                TestType="one2one"
            else
                TestType="avg"
            fi
            ;;
        -r )
            shift
            num_of_rounds=$1
            ;;
        -h)
            Usage
            exit
            ;;
    esac
    shift
done

if [ -z "${TestCase}" ] || [ -z "${TestType}" ]; then
    Usage
    exit
fi

# ----------------------------------------------------------------------
# main
# ----------------------------------------------------------------------
run_setup ${TestType}

case ${TestCase} in
    ipcq)
        run_sys_non_block ${exe_cnt} ${TestType} ${num_of_rounds}
        ;;
    ipcq_block)
        run_sys_block ${exe_cnt} ${TestType} ${num_of_rounds}
        ;;
    posq)
        run_pos_non_block ${exe_cnt} ${TestType} ${num_of_rounds}
        ;;
    posq_block)
        run_pos_block ${exe_cnt} ${TestType} ${num_of_rounds}
        ;;
    *)
        Usage
        ;;
esac

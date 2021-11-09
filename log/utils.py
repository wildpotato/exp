#!/usr/bin/python3

"""
The purpose of this script is to measure the average latency between the receipt of a message
at gateway provided by OTC and that at the server program. We select UC07 (order update) to be
the de facto target but it may be easily swapped. The program takes in two log files, one that
contains all log info from gateway and the other is the server log. Start time and end time may
be defined so the measurement is taken according to the desired time period.
"""

import re
import argparse
import pprint

_TIMESTAMP_RE = '[0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]{6}'

def createParser():
    """ add argument parser """
    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--app_log", help="[input] application log file")
    parser.add_argument("-g", "--gw_log", help="[input] gateway log file")
    parser.add_argument("-v", "--verbosity_level", default=0, help="verbosity level: 0 is OFF (any positive integer is ON)")
    return parser

def hasTimestamp(string):
    """ returns True if string contains timestamp in format of HH:MM:SS:mmmmmm, False otherwise """
    if re.search('^%s' %_TIMESTAMP_RE , string):
        return True
    return False

def getTimestampAsInt(string):
    """ returns timestamp as int """
    ret = None
    obj = re.search('^%s' %_TIMESTAMP_RE, string)
    if obj:
        ret = obj.group(0).replace(':', '')
        ret = ret.replace('.', '')
    if ret is None:
        raise TypeError("ret=%s, string=%s" %(ret, string))
    return int(ret)

def getTimestampFromString(string):
    """ returns timestamp from string """
    obj = re.search('^%s' %_TIMESTAMP_RE, string)
    if obj:
        return obj.group(0)
    return None

def getTimestampAsStr(bigInt):
    """ returns timestamp as HH:MM:SS.mmmmmm from 11 or 12 digit int """
    src = str(bigInt)
    if len(src) == 11:
        src= '0' + src
    ret = str()
    ret += src[0]
    ret += src[1]
    ret += ':'
    ret += src[2]
    ret += src[3]
    ret += ':'
    ret += src[4]
    ret += src[5]
    ret += '.'
    for i in range(len(src)-6, len(src)):
        ret += src[i]
    return ret

def timeIsGreaterThan(timeA, timeB):
    return getTimestampAsInt(timeA) > getTimestampAsInt(timeB)

def isUC07(string):
    """ returns True if string contains '35=UC07', False otherwise """
    if re.search('UC07', string):
        return True
    return False

def getOrderSeq(string):
    """ returns the 7-digit order sequence, None otherwise """
    obj = re.search('81013=[0-9]{7}', string)
    return obj.group(0)[6:] if obj else None

def isTimestampWithinRange(string, startTime, endTime):
    """ returns true if string containing timestamp is between startTime and endTime """
    msgTime = getTimestampFromString(string)
    if timeIsGreaterThan(startTime, msgTime) and timeIsGreaterThan(msgTime, endTime):
        return True
    return False

class CalculateLatency:
    def __init__(self, appLog, gwLog, msgType="UC07"):
        self._app_log = appLog
        self._gw_log = gwLog
        self._msg_type = msgType
        self._start_time = "09:00:00.000000"
        self._end_time = "15:00:00.000000"
        self._order_seq_seen = set()
        self._gw_msg_timestamp = dict()
        self._app_msg_timestamp = dict()
        self._gw_over_time = False
        self._app_over_time = False
        self._num_of_msgs = 0
        self._records = []
        self._latency = 0.0
        self._tolerated_delay = 50

    def ParseGwLog(self):
        with open(self._gw_log, "rb") as gw_fp:
            while not self._gw_over_time:
                msg = gw_fp.readline()
                try:
                    msg = msg.decode("utf-8")
                except:
                    continue
                if len(msg) < 15:
                    continue
                if hasTimestamp(msg):
                    curr_time = getTimestampFromString(msg)
                    if timeIsGreaterThan(curr_time, self._end_time):
                        self._gw_over_time = True
                        print("reach over time in GW log")
                        break
                    if timeIsGreaterThan(self._start_time, curr_time):
                        continue
                    if isUC07(msg) and getOrderSeq(msg) not in self._order_seq_seen:
                        ordSeq = getOrderSeq(msg)
                        self._order_seq_seen.add(ordSeq)
                        self._gw_msg_timestamp[ordSeq] = getTimestampAsInt(msg)

    def ParseAppLog(self):
        with open(self._app_log, "rb") as app_fp:
            while not self._app_over_time:
                msg = app_fp.readline()
                try:
                    msg = msg.decode("utf-8")
                except:
                    continue
                if len(msg) < 15:
                    continue
                if hasTimestamp(msg):
                    curr_time = getTimestampFromString(msg)
                    if timeIsGreaterThan(curr_time, self._end_time):
                        self._app_over_time = True
                        print("reach over time in APP log")
                        break
                    if timeIsGreaterThan(self._start_time, curr_time):
                        continue
                    if isUC07(msg) and getOrderSeq(msg) in self._order_seq_seen:
                        ordSeq = getOrderSeq(msg)
                        self._app_msg_timestamp[ordSeq] = getTimestampAsInt(msg)
                        self._order_seq_seen.remove(ordSeq)

    def ComputeDelayedEndTime(self):
        endTime = getTimestampAsInt(self._end_time) + 50
        self._end_time = getTimestampAsStr(endTime)

    def ComputeLatency(self):
        print("total msgs = ", len(self._gw_msg_timestamp))
        self._num_of_msgs = len(self._gw_msg_timestamp)
        assert(len(self._gw_msg_timestamp) == len(self._app_msg_timestamp))
        self._records = [(k_gw, self._app_msg_timestamp[k_gw], v_gw) for k_gw, v_gw in self._gw_msg_timestamp.items()]
        for record in self._records:
            self._latency += record[1] - record[2]
        self._latency /= self._num_of_msgs
        print("start time = %s, end time = %s" %(self._start_time, self._end_time))
        print("Average latency = %f micro seconds." %self._latency)

def runTest():
    test_str = ["09:00:00.123456 [MSG] market open",
                "15:00:00.000001 [MSG] market close",
                "Recv 35=UC07^A81013=1234567",
                "Send 35=UC07^A81013=1234567",
                "[MSG] A35=UC07",
                ]
    for string in test_str:
        if isUC07(string):
            print(getOrderSeq(string))
        if hasTimestamp(string):
            print(getTimestampAsInt(string))
    print(getTimestampAsStr(100000123456))

def main():
    args = createParser().parse_args()
    calc = CalculateLatency(args.app_log, args.gw_log)
    calc.ParseGwLog()
    calc.ComputeDelayedEndTime()
    calc.ParseAppLog()
    calc.ComputeLatency()

if __name__ == "__main__":
    main()

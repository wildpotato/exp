#!/usr/bin/pyton3

import argparse
import pprint

class LogParser:
    """
    Simple parser that parses input log file based on the user defined criteria and
    stores the conetents that satisfied said criteria into the output file which is
    also specified by the user.

    Assumptions:
    #1. Log file may contain non-ascii characters (i.e., Chinese characters)
    #2. New lines start with timestamp
    #3. Timestamp is in HH:MM:SS format

    Example usage:
    $ python3 logparse.py -i YT_EmgSvr.log -o output.log -d 3 -s 08:37:37

    Comment:
    Running the above command will parse all log lines within 3 seconds starting at
    time 08:37:37 (so all logs recorded in 08:37:37, 08:37:38, 08:37:39 will be
    copied to output file for further analysis)

    """

    def __init__(self, args):
        self.valid_timestamp = []
        self.out_lines = []
        self.input = args.input
        self.output = args.output
        self.start_time = args.start_timestamp
        self.duration = args.duration
        self.verbose = args.verbosity_level

    def __str__(self):
        return f"Log(in={self.input}, out={self.output}, start={self.start_time}, duration={self.duration})"

    def getValidTimestamps(self):
        curr_time = self.start_time
        d = int(self.duration)
        if d >= 0:
            self.valid_timestamp.append(curr_time)
        while d > 0:
            next_second = self.getNextSecond(curr_time)
            self.valid_timestamp.append(next_second)
            curr_time = next_second
            d -= 1
        if self.verbose > 0:
            pprint.pprint(self.valid_timestamp)

    def parseLines(self):
        is_right_time = False
        with open(self.input, "rb") as in_fp:
            lines = in_fp.readlines()
            for line in lines:
                time = line[0:8].decode("utf-8")
                if self.__class__.isValidTime(time):
                    #print(time)
                    if time in self.valid_timestamp:
                        is_right_time = True
                        self.out_lines.append(line)
                    else:
                        is_right_time = False
                else:
                    if is_right_time:
                        self.out_lines.append(line)
        in_fp.close()

    def writeOutFile(self):
        with open(self.output, "wb") as out_fp:
            for out_line in self.out_lines:
                out_fp.write(out_line)
        out_fp.close()

    @classmethod
    def isValidTime(cls, token):
        return token[0:1].isdigit() and token[3:4].isdigit() and token[6:7].isdigit() and \
                token[2] == ":" and token[5] == ":"

    @classmethod
    def getNextSecond(cls, curr_time):
        token = curr_time.split(':')
        hour = int(token[0])
        minute = int(token[1])
        second = int(token[2])
        if second + 1 == 60:
            second = 0
            minute += 1
            if minute == 60:
                minute = 0
                hour += 1
            else:
                minute += 1
        else:
            second += 1
        return cls.assembleTimestamp(hour, minute, second)

    @classmethod
    def assembleTimestamp(cls, hour, minute, second):
        count = 0
        ret = ""
        for num in [hour, minute, second]:
            if num == 0:
                t = "00"
            elif num > 0 and num < 10:
                t = "0" + str(num)
            else:
                t = str(num)
            ret += t

            if count < 2:
                ret += ":"
            count += 1
        return ret

    @staticmethod
    def createParser():
        """ add argument parser """
        parser = argparse.ArgumentParser()
        parser.add_argument("-i", "--input", help="input file name")
        parser.add_argument("-s", "--start_timestamp", help="starting time")
        parser.add_argument("-o", "--output", help="output file name")
        parser.add_argument("-d", "--duration", type=int, help="duration after starting time")
        parser.add_argument("-v", "--verbosity_level", default=0, help="verbosity level: 0 is off")
        return parser


def main():
    args = LogParser.createParser().parse_args()
    l = LogParser(args)
    #print(l.__str__())
    #print(l.__repr__())
    l.getValidTimestamps()
    l.parseLines()
    l.writeOutFile()

if __name__ == "__main__":
    main()

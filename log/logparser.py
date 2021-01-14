#!/usr/bin/pyton3

import argparse
import pprint

class LogParser:
    """
    Simple parser that parses input log file based on the user defined criteria and
    stores the conetents that satisfy said criteria into the output file which is
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
        self.cache_line_out = 100
        self.timestamp_len = 8
        self.total_out_line = 0
        self.is_right_time = False
        self.is_over_time = False
        self.first_write = True
        self.valid_timestamp = []
        self.out_lines = []
        self.input = args.input
        self.output = args.output
        self.start_time = args.start_timestamp
        self.duration = args.duration
        self.verbose = int(args.verbosity_level) if args.verbosity_level is not None else -1

    def __str__(self):
        return f"Log(in={self.input}, out={self.output}, start={self.start_time}, duration={self.duration})"

    def getValidTimestamps(self):
        """ computes all valid seconds in timestamp """
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

    def isValidLine(self, line):
        """ a valid line is a line of log that originated during desired time """
        # the case when a line is too short
        if len(str(line)) < self.timestamp_len:
            return self.is_right_time
        try:
            time = line.decode("utf-8")[0:self.timestamp_len]
        except:
            return self.is_right_time
        if self.verbose > 1:
            print(time)
            print("total line = ", self.total_out_line)
        # line is long enough, check if it starts with valid timestamp
        if self.__class__.isValidTime(time):
            # bingo
            if time in self.valid_timestamp:
                self.is_right_time = True
                return True
            # not right time
            if self.is_right_time:
                self.is_right_time = False
                self.is_over_time = True
            return False
        # line does not start with valid timestamp
        return self.is_right_time

    def parseLines(self):
        with open(self.input, "rb") as in_fp:
            while (not self.is_over_time):
                line = in_fp.readline()
                if self.verbose > 2:
                    print(line)
                if self.isValidLine(line):
                    self.total_out_line += 1
                    self.out_lines.append(line)
                if len(self.out_lines) == self.cache_line_out:
                    self.writeOutFile()
                    self.first_write = False
                    self.out_lines = []
            self.writeOutFile()
        in_fp.close

    def writeOutFile(self):
        if self.first_write:
            options = "wb"
        else:
            options = "ba+"
        with open(self.output, options) as out_fp:
            out_fp.writelines(self.out_lines)
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
        parser.add_argument("-s", "--start_timestamp", help="starting time must be in HH:MM:SS format")
        parser.add_argument("-o", "--output", help="output file name")
        parser.add_argument("-d", "--duration", type=int, help="duration after starting time (in seconds)")
        parser.add_argument("-v", "--verbosity_level", default=0, help="verbosity level: 0 is OFF (any positive integer is ON)")
        return parser


def main():
    args = LogParser.createParser().parse_args()
    l = LogParser(args)
    #print(l.__str__())
    #print(l.__repr__())
    l.getValidTimestamps()
    l.parseLines()

if __name__ == "__main__":
    main()

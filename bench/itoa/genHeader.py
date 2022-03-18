import re

_HEADER_NAME = "tbl_itoa.h"
_MIN_INT = 0
_MAX_INT = 9999
TAB = "    "

class GenerateHeader():
    def __init__(self):
        pass

    def genHeader(self):
        with open(_HEADER_NAME, 'w+',  encoding='utf-8') as self.f:
            self._addPragmaOnce()
            self._addIncludeHeaders()
            self._addStartInitTable()
            self._addInsertInitTable()
            self._addEndInitTable()

    def _addPragmaOnce(self):
        self.f.write("#pragma once\n\n")

    def _addIncludeHeaders(self):
        self.f.write("#include <unordered_map>\n")

    def _addStartInitTable(self):
        self.f.write("\n")
        self.f.write("std::unordered_map<unsigned int, char *> m_itoa = \n")
        self.f.write("{\n")

    def _addInsertInitTable(self):
        for i in range(_MIN_INT, _MAX_INT, 1):
            self.f.write(TAB + '{ ' + str(i) + ' , ' + '(char *) "' + str(i) + '"},\n')

    def _addEndInitTable(self):
        self.f.write("};")

def main():
    instance = GenerateHeader()
    instance.genHeader()
    print("Header file %s has been auto generated" %(_HEADER_NAME))

if __name__ == "__main__":
    main()

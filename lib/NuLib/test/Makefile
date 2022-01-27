#
# @(#)Makefile
#
# Modification History:
#

# Makefile for Linux platform

DIR = $(shell pwd)
UNAME = $(shell uname)

TMPDIR = $(DIR)/tmp

ifeq ($(UNAME), AIX)
  CC = xlc
  CCFLAG = -g -O3
else
  CC = gcc 
#  CCFLAG = -Wall -g -O3 -flto -fwhole-program
  CCFLAG = -Wall -g 
endif

#CCFLAG = -Wall -g -fopenmp
INCLUDEDIR = -I. -I../include
LIBDIR = -L../bin
LIBS = -lNuFixUtil -lNuUtil -lpthread -lrt #-ltcmalloc
COMPILE = $(CC) $(INCLUDEDIR) $(CCFLAG) $(LIBDIR)

.PHONY: all clean init string collection parser common fixutil hangerq  time num reader fix alloc mmap rb async ipcq

#all: init string collection parser log seq cbmgr common fixutil mq hangerq time num reader fix alloc mmap bind memory bind cq pq async ipcq rb
all: init string collection parser log seq cbmgr common fixutil mq hangerq time num reader fix alloc mmap bind memory bind cq pq async ipcq rb
#all: init async rb ipcq
	@echo "---- end of build ----"	

init:
	@if [ ! -d $(TMPDIR) ]; \
	then \
		mkdir -p $(TMPDIR); \
	fi

%: %.c
	@echo "---- Compile $< ----"
	$(COMPILE) -o $(TMPDIR)/$@ $< $(LIBS)
	@echo .

#%.o: %.c
#	@echo "---- Compile $< ----"
#	$(COMPILE) -o $(TMPDIR)/$@ -c $<
#	@echo .

clean:
	@echo "---- clean all object file ----"
	cd $(TMPDIR); rm -f *

string: NuCStrTest NuCStrTest1 NuStrTest1
collection: NuHashTest NuTrieTest
parser: NuIniTest NuFixTest NuFixTest2
fix:NuFixTest3
log: NuLogTest NuALogTest
seq: NuSeqNoTest
ifeq ($(UNAME), AIX)
cbmgr: NuCBMgrTest
common:
else
cbmgr: NuCBMgrTest NuCBMgrTest1
common : NuCommonTest NuTuneTest NuListTest
endif
fixutil : NuFixMsgTest1 NuFixMsgTest2
hangerq : NuHQTest
time : NuTimeTest
num : NuNumTest1
reader : NuFixReaderTest1
alloc : NuAllocTest
mmap : NuMmapTest
mq : NuMQTest NuMQTest2
bind : NuBindTest
memory : NuMemTest1
cq : NuCQTest1
pq : NuPQTest1 NuPQTest2
rb : NuRingBufferTest1 NuRingBufferTest2
async : NuAsyncTest1
ipcq : NuIPCQTest1

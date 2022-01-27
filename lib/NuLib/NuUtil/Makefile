#
# @(#)Makefile
#
# Modification History:
#

# Makefile for Linux platform

TARGET = libNuUtil.a
SOURCE =  NuCommon.o \
		  NuBCD.o \
		  NuBlocking.o \
		  NuCondVar.o \
		  NuTime.o \
		  NuTimer.o \
		  NuUtil.o \
          NuFile.o \
          NuFileStream.o \
          NuStr.o \
          NuFStr.o \
          NuStream.o \
          NuLog.o \
          NuFLog.o \
		  NuAsyncLog.o \
		  NuAsync.o \
		  NuHangerQ.o \
		  NuCircularQ.o \
		  NuRingBuffer.o \
		  NuHash.o \
		  NuHashMap.o \
		  NuTrie.o \
		  NuSeqNo.o \
		  NuBuffer.o \
		  NuAlloc.o \
		  NuIni.o \
          NuIPCQ.o \
		  NuPosixQ.o \
          NuMMapStream.o \
          NuMMap.o \
		  NuMMapQ.o \
          NuCStr.o \
          NuNum.o \
		  NuDaemon.o \
		  NuProcess.o \
          NuCBMgr.o \
		  NuDLibMgr.o \
		  NuLock.o \
		  NuMutex.o \
		  NuSpin.o \
		  NuThread.o \
		  NuThreadPool.o \
		  NuInvoker.o \
		  NuVariable.o \
		  NuMemory.o \
		  NuFix.o \
		  NuFixReader.o \
		  NuTuneTools.o \
		  base64.o

DIR = $(shell pwd)
UNAME = $(shell uname)

TMPDIR = $(DIR)/tmp/static
PICTMPDIR = $(DIR)/tmp/dynamic
BINDIR = ../bin
HEADERDIR = ../include

ifeq ($(UNAME), AIX)
  CC = xlc
  CCFLAG = -g -O3
else
  CC = gcc
  CCFLAG = -Wall -g -std=c11 -O3 -finline-limit=3600 -D_GNU_SOURCE
endif

# turn on openmp 
#CCFLAG = -Wall -g -O3 -fopenmp

PICFLAG =-fpic -ldl
INCLUDEDIR = -I. 
LIBDIR = 
LIBS = -lpthread -lm 
COMPILE = $(CC) $(INCLUDEDIR) $(CCFLAG) $(LIBS)

AR = ar
AFLAG = ruv

.PHONY: all clean

all: init $(TMPDIR)/$(TARGET) $(PICTMPDIR)/$(TARGET) 
	@echo "---- end of build ----"	

init:
	@if [ ! -d $(TMPDIR) ]; \
	then \
		mkdir -p $(TMPDIR); \
	fi
	@if [ ! -d $(PICTMPDIR) ]; \
	then \
		mkdir -p  $(PICTMPDIR); \
	fi

%/$(TARGET): $(SOURCE)
	@echo "---- build $@ library ----"
	cd $*; $(AR) $(AFLAG) $@ $^ 1> /dev/null
	@ranlib $@ 
	@echo .

%.o: %.c
	@echo "---- Compile $< ----"
	$(COMPILE) -o $(TMPDIR)/$@ -c $<
	$(COMPILE) $(PICFLAG) -o $(PICTMPDIR)/$@ -c $<
	@echo .

clean:
	@echo "---- clean all object file ----"
	cd $(TMPDIR); rm -f *.o $(TARGET)
	cd $(PICTMPDIR); rm -f *.o $(TARGET)

install: all
	cd $(DIR)
	@echo "Install executables ... $(BINDIR)" ; \
	echo "Install libraries ... $(TARGET)" ; \
	echo "cp $(TMPDIR)/$(TARGET) $(BINDIR)/"; \
	cp $(TMPDIR)/$(TARGET) $(BINDIR)/. ; \
	cp $(PICTMPDIR)/$(TARGET) $(BINDIR)/PIC ; \
	echo "Install headers ... " ; \
	echo "cp *.h $(HEADERDIR)"; \
	cp *.h $(HEADERDIR)


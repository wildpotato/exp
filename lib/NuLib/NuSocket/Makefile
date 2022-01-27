#
# @(#)Makefile
#
# Modification History:
#

# Makefile for Linux platform

TARGET = libNuSocket.a
SOURCE = NuSocket.o \
		 NuSocketType_Pipe.o \
		 NuSocketType_Inet.o \
		 NuSocketType_Unix.o

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
  CCFLAG = -Wall -g -std=c11 -O3 -D_GNU_SOURCE
endif

PICFLAG =-fpic -ldl
INCLUDEDIR = -I. -I../include
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


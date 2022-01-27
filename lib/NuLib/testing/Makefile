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
  CCFLAG = -Wall -g -O3
endif

#CCFLAG = -Wall -g -fopenmp
INCLUDEDIR = -I. -I../include -I../../ThirdPartyLib/include
LIBDIR = -L../bin -L../../ThirdPartyLib/lib
LIBS = -lNuFixUtil -lNuUtil -lcmockery -lpthread
COMPILE = $(CC) $(INCLUDEDIR) $(CCFLAG) $(LIBDIR)

.PHONY: all clean init test

all: init test
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

clean:
	@echo "---- clean all object file ----"
	cd $(TMPDIR); rm -f *

test: testing

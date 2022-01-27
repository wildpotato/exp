#
# @(#)Makefile
#
# Modification History:
#

# Makefile for Linux platform
TMPDIR =../tmp
CC = gcc

CCFLAG = -Wall -g -O3 -DDEBUG -DUDEBUG 

INCLUDEDIR = -I. -I../ -I../../include
LIBDIR = -L../../bin
LIBS = -lpthread -lm -lNuDBM -lNuUtil -lrt
COMPILE = $(CC) $(INCLUDEDIR) $(CCFLAG) 
AS = as
ASFLAG =
ASSEMBLE = $(AS) $(ASFLAG)
LINKER = gcc
#LFLAG = -g -O3
LFLAG = -g -O
LINK = $(LINKER) $(LFLAG) 
AR = ar
AFLAG = ruv

all: 
	@echo "Target of make should be specified."	

%: %.c
	@echo "---- build Testing program $(TMPDIR)/$@ ----"
	$(COMPILE) -o $(TMPDIR)/$@ $< $(LIBDIR) $(LIBS)
	@touch $(TMPDIR)/$@
	@echo .


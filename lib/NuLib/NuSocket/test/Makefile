#
# @(#)Makefile
#
# Modification History:
#

# Makefile for Linux platform
TMPDIR =../tmp
CC = gcc

CCFLAG = -Wall -g -O3 -DDEBUG -DUDEBUG 

INCLUDEDIR = -I. -I.. -I../../include
LIBDIR = -L../tmp/static -L../../bin
LIBS = -lpthread -lm -lNuSocket -lNuUtil -lrt
COMPILE = $(CC) $(INCLUDEDIR) $(CCFLAG) 
AS = as
ASFLAG =
ASSEMBLE = $(AS) $(ASFLAG)
LINKER = gcc
LFLAG = -g -O3
LINK = $(LINKER) $(LFLAG) 
AR = ar
AFLAG = ruv

OBJS = tSocket tSocketInvoker tSocket_Pingpong tInetStrmSvr tInetStrmClnt tUnixStrmSvr tUnixStrmClnt

all: $(OBJS)
	@echo "---- end of build ----"	

tSocket: tSocket.o
	@echo "---- build $(TMPDIR)/$@ ----"
	$(LINK) -o $(TMPDIR)/$@ $(TMPDIR)/$^ $(LIBDIR) $(LIBS) $(INCLUDEDIR)
	@touch $(TMPDIR)/$@
	@echo .

tSocket_Pingpong: tSocket_Pingpong.o
	@echo "---- build $(TMPDIR)/$@ ----"
	$(LINK) -o $(TMPDIR)/$@ $(TMPDIR)/$^ $(LIBDIR) $(LIBS) $(INCLUDEDIR)
	@touch $(TMPDIR)/$@
	@echo .

tSocketInvoker: tSocketInvoker.o
	@echo "---- build $(TMPDIR)/$@ ----"
	$(LINK) -o $(TMPDIR)/$@ $(TMPDIR)/$^ $(LIBDIR) $(LIBS) $(INCLUDEDIR)
	@touch $(TMPDIR)/$@
	@echo .

tInetStrmSvr: tInetStrmSvr.o
	@echo "---- build $(TMPDIR)/$@ ----"
	$(LINK) -o $(TMPDIR)/$@ $(TMPDIR)/$^ $(LIBDIR) $(LIBS) $(INCLUDEDIR)
	@touch $(TMPDIR)/$@
	@echo .

tInetStrmClnt: tInetStrmClnt.o
	@echo "---- build $(TMPDIR)/$@ ----"
	$(LINK) -o $(TMPDIR)/$@ $(TMPDIR)/$^ $(LIBDIR) $(LIBS) $(INCLUDEDIR)
	@touch $(TMPDIR)/$@
	@echo .

tUnixStrmSvr: tUnixStrmSvr.o
	@echo "---- build $(TMPDIR)/$@ ----"
	$(LINK) -o $(TMPDIR)/$@ $(TMPDIR)/$^ $(LIBDIR) $(LIBS) $(INCLUDEDIR)
	@touch $(TMPDIR)/$@
	@echo .

tUnixStrmClnt: tUnixStrmClnt.o
	@echo "---- build $(TMPDIR)/$@ ----"
	$(LINK) -o $(TMPDIR)/$@ $(TMPDIR)/$^ $(LIBDIR) $(LIBS) $(INCLUDEDIR)
	@touch $(TMPDIR)/$@
	@echo .

.c.o:
	@echo "---- Compile $< ----"
	$(COMPILE) -o $(TMPDIR)/$@ -c $< $(INCLUDEDIR)
	@echo .

.s.o:
	@echo "---- Assemble $< ----"
	$(ASSEMBLE) $<
	@echo .
	
clean:
	@echo "---- clean all object file ----"
	rm -f $(TMPDIR)/tSocket


SUBDIRS = NuUtil NuFixUtil NuSocket NuDBM

MKFILE = Makefile
ifeq ($(shell uname), AIX)
	MK = gmake
else
	MK = make
endif

all: install

init:
	@if [ ! -d bin ]; \
	then \
		mkdir -p bin/PIC; \
	fi
	@if [ ! -d include ]; \
	then \
		mkdir -p include; \
	fi

install: init
	@for i in $(SUBDIRS) ; \
		do cd $$i ; $(MK) -f $(MKFILE) install ; cd .. ;  done

clean:
	@for i in $(SUBDIRS) ; \
		do cd $$i ; $(MK) -f $(MKFILE) clean ; cd .. ;  done
	@rm -f bin/*.a bin/PIC/*.a include/*.h



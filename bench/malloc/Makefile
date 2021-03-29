EXECUTABLE = bench_malloc
CXX = g++
LIB=-L$(shell pwd)/lib
$(info $$LIB is [${LIB}])
#INC=-I../mimalloc/include -I../gperftools/src/gperftools -I../matplotlib-cpp
INC=-I./include
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:%.cpp=%.o)
CXXFLAGS = -std=c++17 -Wno-conversion -Wall

#############################################################################
#  This section is for building the `matplotlib-cpp` which uses Python
#############################################################################
# Default to using system's default version of python
PYTHON_BIN     ?= python3
PYTHON_CONFIG  := $(PYTHON_BIN)-config
PYTHON_INCLUDE ?= $(shell $(PYTHON_CONFIG) --includes)
EXTRA_FLAGS    := $(PYTHON_INCLUDE)
# NOTE: Since python3.8, the correct invocation is `python3-config --libs --embed`.
# So of course the proper way to get python libs for embedding now is to
# invoke that, check if it crashes, and fall back to just `--libs` if it does.
LDFLAGS        += $(shell if $(PYTHON_CONFIG) --ldflags --embed >/dev/null; then $(PYTHON_CONFIG) --ldflags --embed; else $(PYTHON_CONFIG) --ldflags; fi)

# Either finds numpy or set -DWITHOUT_NUMPY
EXTRA_FLAGS     += $(shell $(PYTHON_BIN) $(CURDIR)/numpy_flags.py)
WITHOUT_NUMPY   := $(findstring $(EXTRA_FLAGS), WITHOUT_NUMPY)
#############################################################################

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
ifeq ($(EXECUTABLE), executable)
	@echo Edit EXECUTABLE variable in Makefile
	@echo Using default a.out.
	$(CXX) $(CXXFLAGS) $(OBJECTS)
else
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LIB) $(LDFLAGS) -lmimalloc -ltcmalloc -o $(EXECUTABLE)
endif

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(LDFLAGS) $(INC) -c $*.cpp

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) a.out
	rm -Rf *.dSYM

.PHONY: all release degug profile static clean alltests help

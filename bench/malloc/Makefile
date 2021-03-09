EXECUTABLE = bench_malloc
CXX = g++
LIB=-L$(shell pwd)/lib
$(info $$LIB is [${LIB}])
INC=-I../mimalloc/include -I../gperftools/src/gperftools
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:%.cpp=%.o)
CXXFLAGS = -std=c++17 -Wall -O3

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
ifeq ($(EXECUTABLE), executable)
	@echo Edit EXECUTABLE variable in Makefile
	@echo Using default a.out.
	$(CXX) $(CXXFLAGS) $(OBJECTS)
else
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LIB) -lmimalloc -ltcmalloc -o $(EXECUTABLE)
endif

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c $*.cpp

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) a.out
	rm -Rf *.dSYM

.PHONY: all release degug profile static clean alltests help

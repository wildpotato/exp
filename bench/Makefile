EXECUTABLE = bench
CXX = g++
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:%.cpp=%.o)
CXXFLAGS = -std=c++11 -Wconversion -Wall -Werror -Wextra -pedantic

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
ifeq ($(EXECUTABLE), executable)
	@echo Edit EXECUTABLE variable in Makefile
	@echo Using default a.out.
	$(CXX) $(CXXFLAGS) $(OBJECTS)
else
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXECUTABLE)
endif

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	rm -Rf *.dSYM

.PHONY: all release degug profile static clean alltests help

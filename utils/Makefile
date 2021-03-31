EXECUTABLE = main
CXX = g++
INC=-I./
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:%.cpp=%.o)
CXXFLAGS = -std=c++17 -Wall


all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
ifeq ($(EXECUTABLE), executable)
	@echo Edit EXECUTABLE varaible in Makefile
	@echo Using default a.out.
	$(CXX) $(CXXFLAGS) $(OBJECTS)
else
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXECUTABLE)
endif
.PHONY: all

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c $*.cpp

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) a.out
.PHONY: clean

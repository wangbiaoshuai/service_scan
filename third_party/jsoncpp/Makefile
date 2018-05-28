all: main

LIBS = -Wl,-rpath "./lib" \
	   -L./lib -ljson

INCLUDE = -I./include

CXX = g++

CXXFLAGS = -Wall -O2 $(LIBS) $(INCLUDE) 

main: main.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ 

.PHONY: clean

clean:
	$(RM) *.o main

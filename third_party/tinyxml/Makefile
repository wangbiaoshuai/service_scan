all: libtinyxml.so 
	#test1

CXX = g++

INCLUDE = -I./include

CXXFLAGS += -Wall -O2 -fPIC -Wl,--as-needed ${INCLUDE}

OBJ = tinyxml.o tinyxmlerror.o tinyxmlparser.o tinystr.o

libtinyxml.so: ${OBJ}
	${CXX} ${CXXFLAGS} -shared -o ./lib/$@ $^

tinyxml.o: tinyxml.cpp
	${CXX} ${CXXFLAGS} -c tinyxml.cpp

tinyxmlerror.o: tinyxmlerror.cpp
	${CXX} ${CXXFLAGS} -c $^

tinyxmlparser.o: tinyxmlparser.cpp
	${CXX} ${CXXFLAGS} -c $^

tinystr.o: tinystr.cpp
	${CXX} ${CXXFLAGS} -c $^

#test1: test1.cc
#	${CXX} -I./include -Wl,-rpath "./lib" -L./lib -ltinyxml -std=c++11 ${CXXFLAGS} -o $@ $^

.PHONY: clean

clean: 
	${RM} *.o lib/*

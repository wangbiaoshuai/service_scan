all: test_parse_config

CXX = g++

INCLUDE = -I./include \
		  -I../src

LIBS = -L./lib -lgtest

CXXFLAGS = -Wall -O2 -Wall -pthread -std=c++11 ${INCLUDE} ${LIBS}

test_parse_config: src/test_parse_config.cc ../src/parse_configure.cc
	${CXX} -o $@ $^ ${CXXFLAGS}

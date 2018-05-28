all: test2 test3

LIBS = -Wl,-rpath "./lib" \
       -L./lib -llog4cplus

INCLUDE = -I./include

CXXFLAGS += -Wall -Wl,--as-needed ${INCLUDE} 

test1: test1.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ ${LIBS} 

test2: test2.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ ${LIBS}

test3: test3.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ ${LIBS}

.PHONY: clean

clean:
	$(RM) test1 test2

FLAGS = -g -m64 -fPIC -Wall  -DDEBUG_TRACE
STCLIB= ./libcurl.a  ./libssl.a  ./libcrypto.a ./libz.a 
DYNLIB= -lrt -ldl 
CFLAGS = $(FLAGS) 
CXX = g++
PROJECT = libMCurl.so 
OPT_LEVEL = -O2 -shared
OBJECTS = MCurl.o

$(PROJECT): $(OBJECTS)
	$(CXX) $(OPT_LEVEL) -o  $@  $^ $(STCLIB) $(DYNLIB)

MCurl.o: MCurl.cpp
	$(CXX) -c $^ $(CFLAGS)

.PHONY:clean
clean:
	rm -f $(PROJECT) *.o

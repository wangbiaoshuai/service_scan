all: libthread_pool.so

libthread_pool.so: thread_pool.cc
	g++ -fPIC -shared -o $@ $^ -lpthread -D_REENTRANT

test: test.cc
	g++ -o main test.cc -L./ -lthread_pool -Wl,-R,"./"

.PHONY: clean

clean:
	${RM} libthread_pool.so

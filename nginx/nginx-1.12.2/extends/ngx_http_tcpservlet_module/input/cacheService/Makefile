SDK_DIR = ../..
THRIFT_DIR = ${SDK_DIR}/thrift-0.9.1

INCLUDE = -I${THRIFT_DIR}/include

LIB = -L${THRIFT_DIR}/lib -lthrift-0.9.1

CXXFLAGS = -O2 -m64 $(INCLUDE) #-fPIC

OBJ = ./src/bean_types.o ./src/bean_constants.o ./src/CacheService.o ./src/cacheService_constants.o ./src/cacheService_types.o

all: libcache_service_export.a

libvrv_service_export.so: $(OBJ)
	$(CXX) -shared $(CXXFLAGS) -o ./lib/$@ $^
	cp ./lib/$@ ../../lib

libcache_service_export.a : ${OBJ}
	ar -cru ./lib/$@ $^

src/bean_types.o: src/bean_types.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/bean_constants.o: src/bean_constants.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/cacheService_types.o: src/cacheService_types.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/cacheService_constants.o: src/cacheService_constants.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/CacheService.o: src/CacheService.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	$(RM) src/*.o lib/libcache_service_export.a

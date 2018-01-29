SDK_DIR = ../..
THRIFT_DIR = ${SDK_DIR}/thrift-0.9.1

INCLUDE = -I${THRIFT_DIR}/include

LIB = -L${THRIFT_DIR}/lib -lthrift-0.9.1

CXXFLAGS = -O2 -m64 $(INCLUDE) #-fPIC

OBJ = ./src/vrv_types.o ./src/vrv_constants.o ./src/VRVService.o

all: libvrv_service_export.a

libvrv_service_export.so: $(OBJ)
	$(CXX) -shared $(CXXFLAGS) -o ./lib/$@ $^
	cp ./lib/$@ ../../lib

libvrv_service_export.a : ${OBJ}
	ar -cru ./lib/$@ $^

src/vrv_types.o: src/vrv_types.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/vrv_constants.o: src/vrv_constants.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/VRVService.o: src/VRVService.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	$(RM) src/*.o lib/libvrv_service_export.a

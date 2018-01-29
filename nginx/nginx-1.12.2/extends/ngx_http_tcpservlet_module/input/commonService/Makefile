SDK_DIR = ../..
THRIFT_DIR = ${SDK_DIR}/thrift-0.9.1

INCLUDE = -I${THRIFT_DIR}/include

LIB = -L${THRIFT_DIR}/lib -lthrift-0.9.1

CXXFLAGS = -O2 -m64 $(INCLUDE) #-fPIC

OBJ = ./src/commonService_types.o ./src/commonService_constants.o ./src/CommonService.o

all: libcommon_service_export.a

libcommon_service_export.so: $(OBJ)
	$(CXX) -shared $(CXXFLAGS) -o ./lib/$@ $^
	cp ./lib/$@ ../../lib

libcommon_service_export.a: ${OBJ}
	ar -cru ./lib/$@ $^

src/commonService_types.o: src/commonService_types.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/commonService_constants.o: src/commonService_constants.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

src/CommonService.o: src/CommonService.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

.PHONY: clean

clean:
	$(RM) src/*.o lib/libvrv_service_export.a

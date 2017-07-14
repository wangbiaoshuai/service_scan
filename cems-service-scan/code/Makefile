SDK_PATH = ../../SDK-64
THRIFT_PATH = ${SDK_PATH}/thrift-0.9.0
LOG_PATH = ${SDK_PATH}/log4cplus

INCLUDE = -I./input/configService/src \
		  -I./input/vrvService/src \
		  -I${THRIFT_PATH}/include \
		  -I${LOG_PATH}/include

LIBS = -Wl,-R,"../lib" \
	   -L${THRIFT_PATH}/lib -lthrift-0.9.1 \
	   -L${LOG_PATH}/lib -llog4cplus

CXX = g++
CXXFLAGS = -O2 -Wall -Wl,--as-needed ${INCLUDE} ${LIBS} -std=c++11
PROJECT = FastScan
OBJECTS = .build/commonService_constants.o  .build/CommonService.o .build/CommonService_server.skeleton.o .build/commonService_types.o 

all: foremost ${PROJECT}

foremost:
	mkdir -p .build bin
	$(MAKE) -C input/vrvService
	$(MAKE) -C input/configService
	cp ${THRIFT_PATH}/lib/libthrift-0.9.1.so ./lib
	cp ${LOG_PATH}/lib/liblog4cplus-2.0.so.3 ./lib

${PROJECT}: ${OBJECTS}
	${CXX} -o bin/$@ $^ ${CXXFLAGS}

.build/commonService_constants.o: src/commonService_constants.cpp
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/CommonService.o: src/CommonService.cpp
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/CommonService_server.skeleton.o: src/CommonService_server.skeleton.cpp
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/commonService_types.o: src/commonService_types.cpp
	${CXX} -o $@ -c $^ ${CXXFLAGS}


.PHONY:clean
clean:
	$(RM) -r $(PROJECT) .build CEMS-SERVICE-SCAN.zip ./bin
	$(MAKE) -C input/configService clean
	$(MAKE) -C input/vrvService clean

package:
	cp -r ../pack/installZip ../pack/CEMS-SERVICE-SCAN
	cd ../pack/ && zip -r CEMS-SERVICE-SCAN.zip CEMS-SERVICE-SCAN
	rm -rf ../pack/CEMS-SERVICE-SCAN

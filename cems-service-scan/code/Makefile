SDK_PATH = ../../SDK-64
THRIFT_PATH = ${SDK_PATH}/thrift-0.9.1
LOG_PATH = ${SDK_PATH}/log4cplus
JSON_PATH = ${SDK_PATH}/jsoncpp
TINYXML_PATH = ${SDK_PATH}/tinyxml

INCLUDE = -I./input/configService/src \
		  -I./input/vrvService/src \
		  -I${THRIFT_PATH}/include \
		  -I${LOG_PATH}/include \
		  -I${THRIFT_PATH}/include/thrift \
		  -I${JSON_PATH}/include \
		  -I${TINYXML_PATH}/include

LIBS = -Wl,-R,"../lib" \
	   -L${THRIFT_PATH}/lib -lthrift-0.9.1 \
	   -L${LOG_PATH}/lib -llog4cplus \
	   -L${JSON_PATH}/lib -ljson \
	   -L${TINYXML_PATH}/lib -ltinyxml \
	   -L./lib -lconfig_service_export -lvrv_service_export -ldevscan \
	   -lssl -lcrypto

ifeq ($(debug), 1)
	DEBUG = -g -O0
else
	DEBUG = -O2
endif

CXX = g++
CXXFLAGS = -Wall -Wl,--as-needed ${INCLUDE} ${LIBS} -lpthread -D_REENTRANT ${DEBUG}
PROJECT = cemsscan
OBJECTS = .build/commonService_constants.o  .build/CommonService.o .build/CommonService_server.skeleton.o .build/commonService_types.o .build/parse_configure.o .build/service_reg.o .build/gen_algorithm.o .build/common_function.o .build/md5module.o .build/CRC32.o .build/mutex.o .build/fastscan.o .build/detect_host.o .build/upreport.o .build/conv.o .build/parse_policy.o .build/SnmpScan.o .build/SnmpReport.o .build/arp.o .build/transport_pool.o .build/dev_manager.o

all: foremost ${PROJECT}

foremost:
	mkdir -p .build bin
	$(MAKE) -C input/vrvService
	$(MAKE) -C input/configService
	cp ${THRIFT_PATH}/lib/libthrift-0.9.1.so ./lib
	cp ${LOG_PATH}/lib/liblog4cplus*.so* ./lib

${PROJECT}: ${OBJECTS}
	${CXX} -o bin/$@ $^ ${CXXFLAGS} -rdynamic -ldl

.build/commonService_constants.o: src/commonService_constants.cpp
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/CommonService.o: src/CommonService.cpp
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/CommonService_server.skeleton.o: src/CommonService_server.skeleton.cpp
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/commonService_types.o: src/commonService_types.cpp
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/parse_configure.o: src/parse_configure.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/service_reg.o: src/service_reg.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/gen_algorithm.o: src/gen_algorithm.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS} 

.build/common_function.o: src/common_function.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/md5module.o: src/md5module.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/CRC32.o: src/CRC32.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/mutex.o: src/mutex.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/fastscan.o: src/fastscan.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/detect_host.o: src/detect_host.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/upreport.o: src/upreport.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/conv.o: src/conv.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/parse_policy.o: src/parse_policy.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/SnmpScan.o: src/SnmpScan.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/SnmpReport.o: src/SnmpReport.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/arp.o: src/arp.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/transport_pool.o: src/transport_pool.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/dev_manager.o: src/dev_manager.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}


.PHONY:clean
clean:
	$(RM) -r $(PROJECT) .build CEMS-SERVICE-SCAN.zip ./bin ./logs/Sys.log
	$(MAKE) -C input/configService clean
	$(MAKE) -C input/vrvService clean

package:
	cp -r ./lib ../pack/installZip
	cp -r ./config ../pack/installZip
	cp -r ./bin ../pack/installZip
	cd ../pack/; rm -rf CEMS-SERVICE-SCAN.zip; cp -r installZip CEMS-SERVICE-SCAN; zip -r CEMS-SERVICE-SCAN.zip CEMS-SERVICE-SCAN; rm -rf CEMS-SERVICE-SCAN; cd -

upgrade:
	cp -r ./bin ../pack/upgradeZip
	cd ../pack; rm -rf CEMS-SERVICE-SCAN.zip; cp -r upgradeZip CEMS-SERVICE-SCAN; zip -r CEMS-SERVICE-SCAN.zip CEMS-SERVICE-SCAN; rm -rf CEMS-SERVICE-SCAN; cd -

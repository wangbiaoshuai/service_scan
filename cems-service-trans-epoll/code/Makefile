SDK_PATH = ../../SDK-64
THRIFT_PATH = ${SDK_PATH}/thrift-0.9.1
LOG_PATH = ${SDK_PATH}/log4cplus
JSON_PATH = ${SDK_PATH}/jsoncpp
TINYXML_PATH = ${SDK_PATH}/tinyxml
THREAD_POOL_PATH = ${SDK_PATH}/thread_pool

INCLUDE = -I./input/configService/src \
		  -I./input/vrvService/src \
		  -I${THRIFT_PATH}/include \
		  -I${LOG_PATH}/include \
		  -I${THRIFT_PATH}/include/thrift \
		  -I${JSON_PATH}/include \
		  -I${TINYXML_PATH}/include \
		  -I${THREAD_POOL_PATH}/include

LIBS = -Wl,-R,"../lib" \
	   -L${THRIFT_PATH}/lib -lthrift-0.9.1 \
	   -L${LOG_PATH}/lib -llog4cplus \
	   -L${JSON_PATH}/lib -ljson \
	   -L${TINYXML_PATH}/lib -ltinyxml \
	   -L${THREAD_POOL_PATH}/lib -lthread_pool \
	   -L./lib -lconfig_service_export -lvrv_service_export -lboost_system \
	   -lssl -lcrypto

ifeq ($(debug), 1)
	DEBUG = -g
else
	DEBUG = -O2
endif

CXX = g++
CXXFLAGS = -Wall -Wl,--as-needed ${INCLUDE} ${LIBS} -lpthread -D_REENTRANT ${DEBUG}
PROJECT = cemstrans
OBJECTS = .build/commonService_constants.o  .build/CommonService.o .build/CommonService_server.skeleton.o .build/commonService_types.o .build/parse_configure.o .build/service_reg.o .build/gen_algorithm.o .build/common_function.o .build/md5module.o .build/CRC32.o .build/mutex.o .build/conv.o .build/matchmsg.o .build/transfer_server.o .build/parse_policy.o .build/start_trans.o .build/transferserver.o

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

.build/conv.o: src/conv.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/matchmsg.o: src/matchmsg.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}
	
.build/transfer_server.o: src/transfer_server.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/parse_policy.o: src/parse_policy.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/start_trans.o: src/start_trans.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}

.build/transferserver.o: src/transferserver.cc
	${CXX} -o $@ -c $^ ${CXXFLAGS}


.PHONY:clean
clean:
	$(RM) -r $(PROJECT) .build CEMS-SERVICE-TRANS.zip ./bin ./logs/Sys.log
	$(MAKE) -C input/configService clean
	$(MAKE) -C input/vrvService clean

package:
	cp -r ./lib ../pack/installZip
	cp -r ./config ../pack/installZip
	cp -r ./bin ../pack/installZip
	cd ../pack/; rm -rf CEMS-SERVICE-TRANS.zip; cp -r installZip CEMS-SERVICE-TRANS; zip -r CEMS-SERVICE-TRANS.zip CEMS-SERVICE-TRANS; rm -rf CEMS-SERVICE-TRANS; cd -

upgrade:
	cp -r ./bin ../pack/upgradeZip
	cd ../pack; rm -rf CEMS-SERVICE-TRANS.zip; cp -r upgradeZip CEMS-SERVICE-TRANS; zip -r CEMS-SERVICE-TRANS.zip CEMS-SERVICE-TRANS; rm -rf CEMS-SERVICE-TRANS; cd -

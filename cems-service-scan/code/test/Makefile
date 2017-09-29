PROJECT = test_transport_pool
#test_parse_config test_service_reg test_fastscan

all: ${PROJECT}

SDK_PATH = ../../../SDK-64
LOG_PATH = ${SDK_PATH}/log4cplus
TINYXML_PATH = ${SDK_PATH}/tinyxml
THRIFT_PATH = ${SDK_PATH}/thrift-0.9.1
JSON_PATH = ${SDK_PATH}/jsoncpp

CXX = g++

INCLUDE = -I./include \
		  -I../src \
		  -I${LOG_PATH}/include \
		  -I${TINYXML_PATH}/include \
		  -I${THRIFT_PATH}/include/thrift \
		  -I${JSON_PATH}/include \
		  -I../input/configService/src \
		  -I../input/vrvService/src

LIBS = -Wl,-rpath "./lib" \
	   -L./lib -lgtest -llog4cplus -ltinyxml -ljson -lthrift-0.9.1 \
	   -lconfig_service_export -lvrv_service_export \
	   -lrt

CXXFLAGS = -Wall -O2 -Wall -D_REENTRANT -pthread -Wl,--as-needed ${INCLUDE} ${LIBS}

test_parse_config: src/test_parse_config.cc ../src/parse_configure.cc ../src/mutex.cc
	${CXX} -o $@ $^ ${CXXFLAGS}

test_service_reg: src/test_service_reg.cc
	${CXX} -o $@ $^ ${CXXFLAGS}

test_fastscan: src/test_fastscan.cc ../src/fastscan.cc ../src/parse_configure.cc  ../src/mutex.cc ../src/DetectHost.cc ../src/conv.cc ../src/common_function.cc ../src/service_reg.cc ../src/gen_algorithm.cc ../src/md5module.cc ../src/CommonService.cpp ../src/upreport.cc ../src/CRC32.cc ../src/parse_policy.cc
	${CXX} -o $@ $^ ${CXXFLAGS} -rdynamic -ldl

test_transport_pool: src/test_transport_pool.cc ../src/upreport.cc ../src/parse_configure.cc  ../src/mutex.cc ../src/conv.cc ../src/common_function.cc ../src/gen_algorithm.cc ../src/md5module.cc ../src/CommonService.cpp ../src/CRC32.cc ../src/transport_pool.cc 
	${CXX} -o $@ $^ ${CXXFLAGS} -rdynamic -ldl


.PHONY: clean

clean:
	${RM} ${PROJECT}

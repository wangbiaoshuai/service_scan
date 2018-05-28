#******************************************************************************************************************
# FastScan 依赖库 			   		by mjd						2016/6/16
# libMCurl.so 	网络访问
# libMZlib.so 	解压缩
# libMCrypt.so  加解密
#******************************************************************************************************************
curl_dir = curl/src
zlib_dir = zlib/src
crypto_dir = crypto/src

bin = bin

curl_src = $(wildcard ${curl_dir}/*.cpp)
curl_obj = $(patsubst %.cpp,  %.o,  $(curl_src))

zlib_src = $(wildcard ${zlib_dir}/*.cpp)
zlib_obj = $(zlib_src:%.cpp=%.o)

crypto_src = $(wildcard ${crypto_dir}/*.cpp)
crypto_obj = $(patsubst %.cpp, %.o, ${crypto_src});

FLAGS = -g -m64 -fPIC -Wall  -DDEBUG_TRACE 
DYNLIB = -lrt -ldl
CFLAGS = $(FLAGS)
CXX = g++
OPT_LEVEL = -O2 -shared

curl_STCLIB = ${curl_dir}/libcurl.a  ${curl_dir}/libssl.a  ${curl_dir}/libcrypto.a ${curl_dir}/libz.a
curl_PROJECT = ${bin}/libMCurl.so 
curl_OBJECTS = $(curl_obj)

zlib_STCLIB = ${zlib_dir}/libz.a
zlib_PROJECT = ${bin}/libMZlib.so
zlib_OBJECTS = $(zlib_obj)

crypto_STCLIB = ${crypto_dir}/libssl.a  ${crypto_dir}/libcrypto.a 
crypto_PROJECT = ${bin}/libMCrypt.so
crypto_OBJECTS = $(crypto_obj)

.PHONY: curl zlib crypto all clean clear

all: $(curl_PROJECT) $(zlib_PROJECT) $(crypto_PROJECT)

curl: $(curl_PROJECT)

$(curl_PROJECT): $(curl_OBJECTS)
	$(CXX) $(OPT_LEVEL) -o  $@  $^ $(curl_STCLIB) $(DYNLIB)

${curl_dir}/%.o: ${curl_dir}/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ 

zlib: $(zlib_PROJECT)

$(zlib_PROJECT): $(zlib_OBJECTS)
	$(CXX) $(OPT_LEVEL) -o  $@  $^ $(zlib_STCLIB) $(DYNLIB)

${zlib_dir}/%.o: ${zlib_dir}/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

crypto: $(crypto_PROJECT)

$(crypto_PROJECT): $(crypto_OBJECTS)
	$(CXX) $(OPT_LEVEL) -o  $@  $^ $(crypto_STCLIB) $(DYNLIB)

${crypto_dir}/%.o: ${crypto_dir}/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f ${curl_dir}/*.o ${zlib_dir}/*.o ${crypto_dir}/*.o

cleanall:
	rm -f $(curl_PROJECT) $(zlib_PROJECT) $(crypto_PROJECT)
	rm -f ${curl_dir}/*.o ${zlib_dir}/*.o ${crypto_dir}/*.o


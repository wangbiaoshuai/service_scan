#!/bin/sh
RES_DIR="./build_result"
ENCRYPT_DIR=$RES_DIR/encrypt
SCONS_DIR=$RES_DIR/scons
JSON_DIR=$RES_DIR/jsoncpp
TINYXML_DIR=$RES_DIR/tinyxml
LOG4CPLUS=$RES_DIR/log4cplus
BOOST_DIR=$RES_DIR/boost-1.53.0
THRIFT_DIR=$RES_DIR/thrift-0.9.1

function is_success()
{
    if [ "$1" != "0" ]
    then
        echo -e "\e[1;31m执行失败，退出(1).\e[0m"
        exit 1
    fi
    echo -e "\e[1;32m执行成功.\e[0m"
}

rm -rf $RES_DIR >/dev/null 2>&1
mkdir $RES_DIR >/dev/null 2>&1

#编译encrypt
echo -e "\e[1;32m开始编译&安装encrypt...\e[0m"
cd ./encrypt
make
is_success $?
cd -
echo 


#编译scons
echo -e "\e[1;32m开始编译&安装scons...\e[0m"
cd ./scons-2.2.0
python setup.py install
is_success $?
cd -
echo


#编译jsoncpp
mkdir $JSON_DIR
echo -e "\e[1;32m开始编译&安装jsoncpp...\e[0m"
cd ./jsoncpp/jsoncpp-src-0.5.0
scons platform=linux-gcc
is_success $?
cd -
cp -r ./jsoncpp/jsoncpp-src-0.5.0/include $JSON_DIR
mkdir $JSON_DIR/lib
cp -r ./jsoncpp/jsoncpp-src-0.5.0/libs/*/*.so $JSON_DIR/lib/libjson.so
cp -r ./jsoncpp/jsoncpp-src-0.5.0/libs/*/*.a $JSON_DIR/lib/libjson.a
echo 


#编译tinyxml
mkdir $TINYXML_DIR
echo -e "\e[1;32m开始编译&安装tinyxml...\e[0m"
cd ./tinyxml/
make
is_success $?
cd -
cp -r ./tinyxml/include $TINYXML_DIR
cp -r ./tinyxml/lib $TINYXML_DIR
echo 


#编译log4cplus
install_path=${PWD}/$LOG4CPLUS
mkdir $LOG4CPLUS
echo -e "\e[1;32m开始编译&安装log4cplus...\e[0m"
cd ./log4cplus/log4cplus-1.2.1-rc2/log4cplus-1.2.1-rc2
./configure CXXFLAGS="-std=c++0x" --prefix="$install_path" --enable-static=yes
make && make install
is_success $?
cd -
echo 


#编译boost_1_53_0
install_path=${PWD}/$BOOST_DIR
mkdir $BOOST_DIR
echo -e "\e[1;32m开始编译&安装boost-1.53.0...\e[0m"
cd ./boost_1_53_0
./bootstrap.sh --prefix="${install_path}"
is_success $?
./b2 install
is_success $?
cd -
echo 


#编译thrift-0.9.1
install_path=${PWD}/${THRIFT_DIR}
mkdir $THRIFT_DIR
echo -e "\e[1;32m开始编译&安装thrift-0.9.1...\e[0m"
cd ./thrift-0.9.1
./configure --with-lua=no --with-boost="${PWD}/${BOOST_DIR}" --prefix="${install_path}"
is_success $?
make && make install
yes | cp test/cpp/*.o test/cpp/.libs/
make && make install
is_success $?
#rm -rf "$THRIFT_DIR/bin"
cd -
echo 

exit 0

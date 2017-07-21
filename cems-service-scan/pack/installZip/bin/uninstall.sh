#!/bin/sh
procName=FastScan
serviceName=CEMS-SERVICE-SCAN
exitCode=65
basepath=$(cd `dirname $0`; pwd)
input=y

#echo "请确认是否卸载 $serviceName 服务[y / n]?"

#read input

echo "当前路径: $basepath"

if [ $input == y ] ; then
	echo "开始卸载 $serviceName 服务"
	echo "正在停止 $serviceName 服务..."
	service $serviceName stop >/dev/null 2>&1 &
	sleep 1
	chkconfig --del $serviceName >/dev/null 2>&1 &
	echo "正在停止 $procName 进程..."
	ID=`ps -ef | grep "$procName" | grep -v "$0" | grep -v "grep" | awk '{print $2}'`
	echo $ID
	for id in $ID
	do
	kill -9 $id
	echo "killed $id"
	done

	echo "正在删除相关程序文件..."
	rm -rf ../logs/*
	rm -rf ../lib/*
	rm -rf ../data/*
	rm -rf ../config/*
	rm -rf *

	COUNT=`ps -ef | grep $procName | grep -v "grep"|wc -l `
if [ $COUNT -lt 1 ]; then
	echo "卸载 $serviceName 成功"
	exit $exitCode
else
	echo "卸载 $serviceName 失败"
fi
	
else
	echo "退出卸载程序"
	exit
fi


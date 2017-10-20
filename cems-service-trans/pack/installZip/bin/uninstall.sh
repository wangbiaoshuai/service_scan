#!/bin/sh
procName=cemstrans
serviceName=CEMS-SERVICE-TRANS
exitCode=65
basepath=$(cd `dirname $0`; pwd)
input=y

#echo "请确认是否卸载 $serviceName 服务[y / n]?"

#read input

echo "当前路径: $basepath"

function is_runing()
{
    pid=$(pgrep $procName)
    if [ "$pid" != "" ] 
    then
        return 0
    fi
    return 1
}

if [ $input == y ] ; then
	echo "开始卸载 $serviceName 服务"
	echo "正在停止 $serviceName 服务..."
	service $serviceName stop

    is_runing
    if [ "$?" = "0" ]
    then
        echo "服务停止失败"
        exit 1
    fi

	echo "正在删除相关程序文件..."
	chkconfig --del $serviceName
    rm -rf /etc/init.d/$serviceName
    rm -rf "/usr/local/service/$serviceName"
    echo "$serviceName 卸载成功"

else
	echo "退出卸载程序"
	exit
fi


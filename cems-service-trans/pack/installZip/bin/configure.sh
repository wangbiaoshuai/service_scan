#!/bin/sh

CONF_FILE=../config/config.properties
SERVICE_IP=""
SERVER_IP=""

function check_device()
{
    devices=$(ip a | grep -Po '(?<=inet ).*(?=\/)' | grep -v "127.0.0.1")
    array_dev=("$devices")
    if [ "${#array_dev[*]}" -eq 1 ]
    then
        SERVICE_IP="$devices"
        echo "service ip: $SERVICE_IP"
        return 0
    fi

    echo "检测到本机有多个网卡:"
    i=0
    for dev in ${array_dev[*]}
    do
        echo "[$i]:${array_dev[$i]}"
        let i++
    done
    echo -n "请选择相应网卡[0-${#array_dev[*]}]:"
    read num
    SERVICE_IP=${array_dev[$num]}
    echo "service ip: $SERVICE_IP"
    return 0
}

function configure()
{
    echo -n "请输入服务器ip:"
    read SERVER_IP
    if [ "$SERVER_IP" = "" ]
    then
        SERVER_IP="$SERVICE_IP"
    fi

    if [ -f "$CONF_FILE" ]
    then
        \sed -i "/^server\.ip/s/=.*/=$SERVER_IP/" $CONF_FILE
        \sed -i "/^service\.ip/s/=.*/=$SERVICE_IP/" $CONF_FILE
    fi
}

function main()
{
    check_device
    configure
}

main

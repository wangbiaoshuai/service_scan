#!/bin/sh

CONF_FILE=../config/config.properties
SERVER_IP=""

function check_device()
{
    devices=$(ip a | grep -Po '(?<=inet ).*(?=\/)' | grep -v "127.0.0.1")
    array_dev=("$devices")
    if [ "${#array_dev[*]}" -eq 1 ]
    then
        SERVER_IP="$devices"
        echo "server ip: $SERVER_IP"
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
    SERVER_IP=${array_dev[$num]}
    echo "server ip: $SERVER_IP"
    return 0
}

function configure()
{
    if [ -f "$CONF_FILE" ]
    then
        \sed -i "/^server\.ip/s/=.*/=$SERVER_IP/" $CONF_FILE
        \sed -i "/^service\.ip/s/=.*/=$SERVER_IP/" $CONF_FILE
    fi
}

function main()
{
    check_device
    configure
}

main

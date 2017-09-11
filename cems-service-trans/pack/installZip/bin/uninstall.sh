#!/bin/sh

#stop service
service CEMS-SERVICE-TRANS stop >/dev/null 2>&1 &
sleep 1
chkconfig --del CEMS-SERVICE-TRANS >/dev/null 2>&1 &
rm -rf /etc/init.d/CEMS-SERVICE-TRANS
rm -rf /usr/local/service/CEMS-SERVICE-TRANS

echo "uninstall $serviceName success"


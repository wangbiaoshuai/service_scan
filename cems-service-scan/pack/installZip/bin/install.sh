#"#This script info[Version:1.0.20150709 updateTime:2015-07-09 Author:linwu_gao@163.com]"
#echo -n "############### 1.Please enter service name:"
#	read serviceName
#	echo "#### you input serviceName is [$serviceName] ####"

text="install CEMS-SERVICE-SCAN service"
#echo "${text}"

#stop service
service CEMS-SERVICE-SCAN stop >/dev/null 2>&1 &

sleep 1

chkconfig --del CEMS-SERVICE-SCAN >/dev/null 2>&1 &
#change file property

#own group change to root
chown root FastScan
chgrp root FastScan

#super manager
chmod u+s  FastScan
chmod +x   FastScan

#change own
chown root libMCurl.so
chgrp root libMCurl.so
chmod +x   libMCurl.so

#sh exec
chmod +x   restart.sh
chmod +x   self.sh

#change own group
chown root CEMS-SERVICE-SCAN
chgrp root CEMS-SERVICE-SCAN

#all user exec
chmod 755  CEMS-SERVICE-SCAN
chmod 666  ../config/policy.xml


rm -rf /etc/init.d/CEMS-SERVICE-SCAN

ln -s /usr/local/service/CEMS-SERVICE-SCAN/bin/CEMS-SERVICE-SCAN  /etc/init.d/CEMS-SERVICE-SCAN

chmod 755 /etc/init.d/CEMS-SERVICE-SCAN

#all user rw
chmod 666  ../config/config.properties

#add service
chkconfig --add  CEMS-SERVICE-SCAN 

#start service
service CEMS-SERVICE-SCAN start

echo ""

text="wait service start..."
echo "${text}"

sleep 1

c=$(netstat -anp | grep LISTEN | grep 10100 | wc -l)
if [ $c -lt 1 ]
then
  ## restart scan service
  echo ""
  echo "service listen port 10100 faild!"
else
  echo ""
  echo "install complete!"
fi



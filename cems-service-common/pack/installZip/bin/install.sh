#"#This script info[Version:1.0.20150709 updateTime:2015-07-09 Author:linwu_gao@163.com]"
#echo -n "############### 1.Please enter service name:"
#	read serviceName
#	echo "#### you input serviceName is [$serviceName] ####"

service_name="CEMS-SERVICE-SCAN"
service_path="/usr/local/service"
proc_name="cemsscan"
service_port=10100

text="install $service_name service"
#echo "${text}"

#stop service
service $service_name stop >/dev/null 2>&1 &

sleep 1

chkconfig --del $service_name >/dev/null 2>&1 &
#change file property

#own group change to root
chown root $proc_name
chgrp root $proc_name

#super manager
chmod +x   $proc_name
chmod u+s  $proc_name

#change own
#chown root libMCurl.so
#chgrp root libMCurl.so
#chmod +x   libMCurl.so

#sh exec
chmod +x   restart.sh
chmod +x   self.sh

#change own group
chown root $service_name
chgrp root $service_name

#all user exec
chmod 755  $service_name
chmod 666  ../config/policy.xml


rm -rf /etc/init.d/$service_name

ln -s /usr/local/service/$service_name/bin/$service_name  /etc/init.d/$service_name

chmod 755 /etc/init.d/$service_name

#all user rw
chmod 666  ../config/config.properties

#add service
chkconfig --add  $service_name

#start service
service $service_name start

c=$(netstat -anp | grep LISTEN | grep ${service_port} | wc -l)
if [ $c -lt 1 ]
then
  ## restart service
  echo ""
  echo "service listen port ${service_port} faild!"
else
  echo ""
  echo "install complete!"
fi



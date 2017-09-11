#"#This script info[Version:1.0.20150709 updateTime:2015-07-09 Author:linwu_gao@163.com]"
#echo -n "############### 1.Please enter service name:"
#	read serviceName
#	echo "#### you input serviceName is [$serviceName] ####"

text="install CEMS-SERVICE-TRANS service"
#echo "${text}"

#stop service
service CEMS-SERVICE-TRANS stop >/dev/null 2>&1 &

sleep 1

chkconfig --del CEMS-SERVICE-TRANS >/dev/null 2>&1 &
#change file property

#own group change to root
chown root CEMS.SERVICE.TRANS
chgrp root CEMS.SERVICE.TRANS
chown root transferServer
chgrp root transferServer

#super manager
chmod u+s  CEMS.SERVICE.TRANS
chmod +x   CEMS.SERVICE.TRANS
chmod u+s  transferServer
chmod +x   transferServer

#change own
chown root libMCrypt.so  libMZlib.so
chgrp root libMCrypt.so  libMZlib.so
chmod +x   libMCrypt.so  libMZlib.so

#sh exec
#chmod +x   restart.sh
#chmod +x   self.sh

#change own group
chown root CEMS-SERVICE-TRANS
chgrp root CEMS-SERVICE-TRANS

#all user exec
chmod 755  CEMS-SERVICE-TRANS
chmod 666  ../config/policy.xml


rm -rf /etc/init.d/CEMS-SERVICE-TRANS

ln -s /usr/local/service/CEMS-SERVICE-TRANS/bin/CEMS-SERVICE-TRANS  /etc/init.d/CEMS-SERVICE-TRANS

chmod 755 /etc/init.d/CEMS-SERVICE-TRANS

cp ../lib/libboost_system.so.1.61.0 /usr/lib64/
#all user rw
chmod 666  ../config/config.properties

#add service
chkconfig --add  CEMS-SERVICE-TRANS 

#start service
service CEMS-SERVICE-TRANS start

echo ""

text="wait service start..."
echo "${text}"

sleep 1

#c=$(netstat -anp | grep LISTEN | grep 10100 | wc -l)
#if [ $c -lt 1 ]
#then
  ## restart scan service
#  echo ""
#  echo "service listen port 10100 faild!"
#else
  echo ""
  echo "install complete!"
#fi



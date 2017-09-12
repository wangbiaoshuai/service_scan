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
chown root cemstrans
chgrp root cemstrans

#super manager
chmod +x   cemstrans
chmod u+s  cemstrans

#change own
#chown root libMCurl.so
#chgrp root libMCurl.so
#chmod +x   libMCurl.so

#change own group
chown root CEMS-SERVICE-TRANS
chgrp root CEMS-SERVICE-TRANS

#all user exec
chmod 755  CEMS-SERVICE-TRANS

rm -rf /etc/init.d/CEMS-SERVICE-TRANS

ln -s /usr/local/service/CEMS-SERVICE-TRANS/bin/CEMS-SERVICE-TRANS  /etc/init.d/CEMS-SERVICE-TRANS

chmod 755 /etc/init.d/CEMS-SERVICE-TRANS

#all user rw
chmod 666  ../config/config.properties

#add service
chkconfig --add  CEMS-SERVICE-TRANS

#start service
service CEMS-SERVICE-TRANS start

c=$(netstat -anp | grep LISTEN | grep 10500 | wc -l)
if [ $c -lt 1 ]
then
  ## restart trans service
  echo ""
  echo "service listen port 10500 faild!"
else
  echo ""
  echo "install complete!"
fi



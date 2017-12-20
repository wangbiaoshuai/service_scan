#!/bin/bash

if [ ! -d "/tmp/content" ]
then
	mkdir -p /tmp/content
fi

if [ -e /tmp/content/content_$1 ]
then
	wc -lcw /tmp/content/content_$1 | awk '{print $1}''{print $2}''{print $3}'
else
	wget --spider -T 5 -t 1 http://$1 -o /tmp/content/content_test_$1
	result=`cat /tmp/content/content_test_$1 | grep "200 OK" | wc -l`
	result1=`cat /tmp/content/content_test_$1 | grep "200 Ok" | wc -l`
	if [ $result -gt 0 ]
	then
		wget -T 5 -t 1 http://$1 -O /tmp/content/content_$1 -o /tmp/content/content_test_$1
		rm -rf /tmp/content/content_test_$1
		wc -lcw /tmp/content/content_$1 | awk '{print $1}''{print $2}''{print $3}'
	elif [ $result1 -gt 0 ]
	then
		wget -T 5 -t 1 http://$1 -O /tmp/content/content_$1 -o /tmp/content/content_test_$1
		rm -rf /tmp/content/content_test_$1
		wc -lcw /tmp/content/content_$1 | awk '{print $1}''{print $2}''{print $3}'
	fi
fi

exit
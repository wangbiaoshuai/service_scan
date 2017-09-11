#!/bin/bash
c=$(netstat -anp | grep LISTEN | grep 10100 | wc -l)
if [ $c -lt 1 ]
then
  ## restart scan service
  /sbin/service CEMS-SERVICE-SCAN start
fi

#!/bin/sh
PROGRAM=/usr/local/service/CEMS-SERVICE-SCAN/bin/restart.sh
CRONTAB_CMD="*/1 * * * * /bin/sh $PROGRAM  > /dev/null 2>&1 &"
(crontab -l 2>/dev/null | grep -Fv $PROGRAM; echo "$CRONTAB_CMD") | crontab -
COUNT=`crontab -l | grep $PROGRAM | grep -v "grep"|wc -l `
if [ $COUNT -lt 1 ]; then
        echo "fail to add crontab $PROGRAM"
        exit 1
fi 

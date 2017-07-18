#!/bin/sh

[ -e "Makefile" ] && make
if [ $? != 0 ]
then
    exit 1
fi

execute_file=`ls`

for file in $execute_file
do 
    if [ "$file" = $0 ]
    then
        continue
    fi
    is_exe=`file $file | grep "executable"`
    if [ "$is_exe" != "" ]
    then
        ./$file
    fi
done

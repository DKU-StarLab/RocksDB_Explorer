#!/bin/bash

fname=$1

if [ -f $fname ]; then
	rm $fname
fi
touch $fname

echo "Delay"
while :
do
	sleep 1
	line=`top -b -n 1 | grep db_bench | cut -d':' -f2 | cut -d' ' -f2`
	if [ x$line != x ]; then
		break
	fi
done

echo "Start"
while :
do
	sleep 1
	line=`top -b -n 1 | grep db_bench`
	check=`echo $line | cut -d':' -f2 | cut -d' ' -f2`
	if [ x$check != x ]; then
		echo $line >> $fname
	fi
done

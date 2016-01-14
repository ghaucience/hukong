#!/bin/sh

program="server485"
logfile="/tmp/daemon-$program.dat"

while true
do
    sleep 5
    pid=`pidof $program`
    if [ -z "$pid" ];then
        if [ ! -e $logfile ]; then
            echo "1" > $logfile
        else
            count=`head -n 1 $logfile`
            count=$(($count+1))
            echo $count > $logfile
        fi

        date +"%m-%d %H:%M:%S" >> $logfile
        /etc/init.d/xmlrpc start
    fi
done

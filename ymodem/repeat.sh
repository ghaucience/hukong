#!/bin/sh

i=1
while [ "$i" != "30" ]
do
    echo $i
    echo -e "1\n2" | ./upgrade.sh /tmp/STM8S105C6_LampCtrl_BBB.bin
    
    if [ "$?" != "0" ];then
        echo "$i fail"
        exit 1
    fi

    i=$(($i+1))
    sleep 2
done

#!/bin/sh 
# upgrade 5 devs
VERSION="0.0.1"

COUNT=1
COUNT_MAX=5

echo "准备升级$COUNT_MAX个设备, 固件:"
ls *.bin 2>/dev/null
if [ "$?" != "0" ];then
    echo "找不到固件，请复制到/root目录下."
    exit 1
fi

for firmware in *.bin
do
    while [ "$COUNT" -le "$COUNT_MAX" ]
    do
        echo $COUNT | ./upgrade.sh $firmware
        if [ "$?" != "0" ];then
            # try bootloader flash mode
            sleep 3
            ./upgrade.sh $firmware -f
            if [ "$?" != "0" ]; then
                echo "485地址$COUNT升级失败!!!"
                exit 1
            fi
        fi
        COUNT=$((COUNT+1))
        sleep 1
    done
done
echo "升级成功"
exit 0

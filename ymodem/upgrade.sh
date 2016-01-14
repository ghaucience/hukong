#!/bin/sh
VERSION="1.1.1"
COPY="Copyright (C) 2013 Lierda Science& Technology Group Co., Ltd"

if [ ! -f "$1" ] || [ -n "$2" ] && [ "$2" != "-f" -a "$2" != "-n" ]; then
    echo Version:$VERSION $COPY
    echo "Usage:"
    echo "  $0 upgrade_file [-f]"
    echo "    -f    flash directly, device don't active upgrade mode"
    echo "    -n    dont check hardware type and firmware type"
    exit 1
fi
upgrade_file=$1
flash_directly=0
disable_hd_type_check=0
if [ "$2" = "-f" ]; then
    flash_directly=1
elif [ "$2" = "-n" ]; then
    disable_hd_type_check=1
fi

[ -f "/etc/init.d/xmlrpc" ] && /etc/init.d/xmlrpc stop

[ -e "/dev/ttyUSB0" ] && dev="/dev/ttyUSB0" || dev="/dev/ttyS0"

is_meter=0

# check flash directly flag
if [ "$flash_directly" = "0" ];then

    echo $upgrade_file | grep -q LampCtrl  && opt="-l"
    echo $upgrade_file | grep -q CurtainCtrl  && opt="-c"
    echo $upgrade_file | grep -q mb9bf321k  && opt="-m" && is_meter=1
    if [ -z "$opt" ];then
        echo invalid filename
        exit 1
    fi

    if [ "$disable_hd_type_check" = "0" ];then
        if [ "$opt" = "-l" ] || [ "$opt" = "-c" ];then
            firmware_type=`echo $upgrade_file | cut -d'_' -f 3 | cut -c 1`
            echo "firmware type: $firmware_type"
        fi
    fi

    read -p "input 485 address:" addr
    if [ "$addr" -gt "0" -a "$addr" -lt "9" ]; then
        opt="$opt $addr"

        if [ -n "$firmware_type" ];then
            ret_type=`/root/server485 -S $opt $dev`
            if [ "$?" == "0" ];then
                echo "device type: $ret_type"
                if [ "$ret_type" = "$firmware_type" ];then
                    echo "firmware type and device hardware type matched"
                else
                    echo "error: firmware type and device hardware type not matched!"
                    exit 1
                fi
            else
                echo "warning: get device hardware type fail!"
            fi
        fi

        if [ -f "/root/server485" ]; then
            /root/server485 $opt $dev
        else
            ./server485* $opt $dev
        fi
            
        if [ ! "$?" = "0" ]; then
            echo execute server485 fail
            exit 1
        fi
    else
        echo "wrong 485 addr"
        exit 1
    fi
fi

#stty -F $dev 9600 raw parenb -parodd cs8 -cstopb
./ymodem_send* -t -d $dev
if [ "$?" != "0" ]; then
    echo open serial port fail
    exit 1
fi
sleep 1
echo -n c >$dev

sleep 1
echo -n 1 >$dev

sleep 1
./ymodem_send* -d $dev $upgrade_file

if [ "$?" != "0" ] ; then
    exit 1
else
    sleep 1
    if [ "$is_meter" = "1" ];then
        # meter is 5, other is 4
        echo -n 5 >$dev
    else
        echo -n 4 >$dev
    fi
fi

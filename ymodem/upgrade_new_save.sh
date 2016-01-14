#!/bin/sh
VERSION="1.2.0"
COPY="Copyright (C) 2013 Lierda Science& Technology Group Co., Ltd"

if [ ! -f "$1" ] || [ -n "$2" ] && [ "$2" != "-f" -a "$2" != "-n" -a "$2" != "-a" ]; then
    echo Version:$VERSION $COPY
    echo "Usage:"
    echo "  $0 upgrade_file [-f]"
    echo "    -f    flash directly, device don't active upgrade mode"
    echo "    -a    auto upgrade all *.bin in current dir"
    exit 1
fi

upgrade_file=$1
flash_directly=0
auto_upgrade=0
if [ "$2" = "-f" ]; then
    flash_directly=1
elif [ "$2" = "-a" ]; then
    auto_upgrade=1
fi

# type
LIGHT=1
CURTAIN=2
METER=3

dev="/dev/ttyUSB0"
ret=""

init()
{
    [ -f "/etc/init.d/xmlrpc" ] && /etc/init.d/xmlrpc stop
    [ -e "/dev/ttyUSB0" ] && dev="/dev/ttyUSB0" || dev="/dev/ttyS0"
}

get_firmware_device_type()
{
    file=$1
    ret=""
    echo $file | grep -q LampCtrl  && ret=$LIGHT && return 0
    echo $file | grep -q CurtainCtrl  && ret=$CURTAIN && return 0
    echo $file | grep -q mb9bf321k  && ret=$METER && return 0
    echo "Error: invalid firmware file! $file"
    return 1
}

get_firmware_device_type_ABCD()
{
    file=$1
    dev_type=$2
    ret=""
    if [ "$dev_type" = "$LIGHT" ] || [ "$dev_type" = "$CURTAIN" ];then
        ret=`echo $file | cut -d'_' -f 3 | cut -c 1`
        case "$ret" in 
            "A")
                ;;
            "B")
                ;;
            "C")
                ;;
            *)
                echo "Error: invalid light or curtain firmware file name no ABCD type info: $file"
                return 1
                ;;
        esac
        return 0
    fi
    echo "Waring: not light or curtain firmware file"
    return 1
}

get_opt_by_device_type()
{
    dev_type=$1
    ret=""
    case "$dev_type" in
        "$LIGHT")
            ret="-l "
            ;;
        "$CURTAIN")
            ret="-c "
            ;;
        "$METER")
            ret="-m "
            ;;
        *)
            echo "Error: invaild devtype $devtype"
            return 1
            ;;
    esac
    return 0
}

get_device_type_ABCD()
{
    addr=$1
    dev_type=$2
    ret=""

    get_opt_by_device_type $dev_type
    if [ "$?" != "0" ];then
        return 1
    fi

    opt="-S $ret $addr "
    ret=`/root/server485 $opt $dev`
    if [ "$?" != "0" ];then
        echo "warning: send cmd get device hardware type fail!: $opt "
        return 1
    else
        return 0
    fi
}

cmd_setup_mode()
{
    addr=$1
    dev_type=$2

    get_opt_by_device_type $dev_type
    if [ "$?" != "0" ]; then
        return 1
    fi
    opt="$ret $addr "

    if [ -f "/root/server485" ]; then
        /root/server485 $opt $dev
    else
        ./server485* $opt $dev
    fi
        
    if [ ! "$?" = "0" ]; then
        echo "execute server485 fail"
        return 1
    fi
    return 0
}

# check firmware and setup hukong upgrade mode
setup()
{
    if [ -n "$firmware_type" ];then
        ret_type=`/root/server485 -S $opt $dev`
        echo "device type: $ret_type"
        if [ "$ret_type" = "$firmware_type" ];then
            echo "firmware type and device hardware type matched"
        else
            echo "error: firmware type and device hardware type not matched!"
            return 1
        fi
    fi

}


flash()
{
    file=$1
    dev_type=$2

    #stty -F $dev 9600 raw parenb -parodd cs8 -cstopb
    ./ymodem_send* -t -d $dev
    if [ "$?" != "0" ]; then
        echo open serial port fail
        return 1
    fi
    sleep 1
    echo -n c >$dev

    sleep 1
    echo -n 1 >$dev

    sleep 1
    ./ymodem_send* -d $dev $file

    if [ "$?" != "0" ] ; then
        return 1
    else
        sleep 1
        if [ "$dev_type" = "$METER" ];then
            # meter is 5, other is 4
            echo meter upgrade
            echo -n 5 >$dev
        else
            echo -n 4 >$dev
        fi
    fi
}

# init
init

if [ "$flash_directly" = "0" ];then
    read -p "input 485 address:" addr
    if [ "$addr" -gt "0" -a "$addr" -lt "9" ]; then
        setup $upgrade_file $addr
        if [ "$?" != "0" ];then
            exit 1
        fi
    else
        echo "wrong 485 addr"
        exit 1
    fi
fi

#flash $upgrade_file
#exit $?

#!/bin/sh

#export DROPBEAR_PASSWORD=www.lierda.com
export DROPBEAR_PASSWORD=4c7c27a5
script_name=`basename $0`
version="0.0.1"

# 升级的IP地址和固件
ip_address=
bin_file=

error()
{
    echo $* 1>&2
    exit 1
}

help()
{
    echo "Version: $version"
    echo "    远大远程升级灯控、帘控固件"
    echo "Usage:"
    echo "    $script_name 固件 IP地址"
}

getopt()
{
    if [ "$#" != "2" ];then
        help
        error
    fi

    if [ -f "$1" ];then
        bin_file=$1
        ip_address=$2
    elif [ -f "$2" ];then
        bin_file=$2
        ip_address=$1
    else
        help
        error
    fi
}

main()
{
    getopt $*
    echo $ip_address $bin_file

    install_dir=`dirname $0`
    echo "准备安装设备固件..."
    ${install_dir}/ssh -y -T root@${ip_address} "rm -f /tmp/*.bin"
    if [ "$?" != "0" ];then
        error "无法连接!"
    fi

    echo "上传升级固件${bin_file}到${ip_address}"
    ${install_dir}/scp -y -S ${install_dir}/ssh ${bin_file} root@${ip_address}:/tmp
    if [ "$?" != "0" ];then
        error "上传固件失败!"
    fi
}

main $*

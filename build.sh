#!/bin/bash

#version=`grep -Po "MAJOR +\d+|MINOR +\d+|RELEASE +\d+" src/rpc_config.h | awk '{print $2}' | sed '$!N;$!N;$!N;s/\n/./g'`
version=$(echo -en '#include "src/rpc_config.h"\necho VERSION' | cpp - | sh | cut -d " " -f 1)
echo $version
sed "s/\(Version: *\)\(.*\)/\1$version/g" ./install/CONTROL/control > /tmp/control.tmp
######

#sudo cp /tmp/control.tmp ./install/CONTROL/control

#sudo cp ./server485 ./install/root/server485
#sudo cp ./svm/* ./install/usr/local/sedonaApp/
#sudo cp ./ymodem/upgrade.lua ./install/root/
#sudo cp ./ymodem/ymodem_send ./install/root/
#sudo chown root ./install/usr/local/sedonaApp/*
#sudo chgrp root ./install/usr/local/sedonaApp/*
#ipkg-build.sh install .

# zip install files
#sudo zip -r -y hukong_$version.zip install/*
#sudo zip -d hukong_$version.zip install/CONTROL install/CONTROL/*

cp /tmp/control.tmp ./install/CONTROL/control

cp ./server485 ./install/root/server485
cp ./svm/* ./install/usr/local/sedonaApp/
cp ./ymodem/upgrade.lua ./install/root/
cp ./ymodem/ymodem_send ./install/root/
chown root ./install/usr/local/sedonaApp/*
chgrp root ./install/usr/local/sedonaApp/*
ipkg-build.sh install .

# zip install files
zip -r -y hukong_$version.zip install/*
zip -d hukong_$version.zip install/CONTROL install/CONTROL/*

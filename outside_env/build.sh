#!/bin/sh

version=`./version.sh`
echo $version
sed "s/\(Version: *\)\(.*\)/\1$version/g" ./install/CONTROL/control > /tmp/control.tmp
sudo cp /tmp/control.tmp ./install/CONTROL/control

sudo cp ./outside_env ./install/usr/local/outside/
sudo chown root ./install/usr/local/outside/*
sudo chgrp root ./install/usr/local/outside/*
ipkg-build.sh install .

cp ./install/etc  /home/au/all/openwrt/attitude_adjustment/package/lierda/shc/hukong/files/ -rf
cp ./install/root /home/au/all/openwrt/attitude_adjustment/package/lierda/shc/hukong/files/ -rf
cp ./install/usr /home/au/all/openwrt/attitude_adjustment/package/lierda/shc/hukong/files/ -rf

cd /home/au/all/openwrt/attitude_adjustment/
make package/lierda/shc/hukong/compile -j1 V=99
cd -



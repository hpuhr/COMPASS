export QT_SELECT=5

rm -rf /app/workspace/compass/ub14_build # needed since binary becomes too big
mkdir -p /app/workspace/compass/ub14_build
cd /app/workspace/compass/ub14_build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo .. #
make -j16
sudo make install

cd /app/workspace/compass/docker

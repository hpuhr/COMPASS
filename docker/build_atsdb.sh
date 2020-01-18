export QT_SELECT=5

rm -rf /app/workspace/atsdb/ub14_build
mkdir -p /app/workspace/atsdb/ub14_build
cd /app/workspace/atsdb/ub14_build
cmake -DCMAKE_INSTALL_PREFIX=/usr .. #--config Release
make -j4
sudo make install

cd /app/workspace/atsdb/docker

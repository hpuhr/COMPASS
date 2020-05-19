mkdir -p /app/workspace/toe/ub14_build
cd /app/workspace/toe/ub14_build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make -j4
#sudo make install

cd /app/workspace/atsdb/docker

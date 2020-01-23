mkdir -p /app/workspace/jasterix/ub14_build
cd /app/workspace/jasterix/ub14_build
cmake -DCMAKE_INSTALL_PREFIX=/usr --config Release ..
make -j4
sudo make install

cd /app/workspace/atsdb/docker

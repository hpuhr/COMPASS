#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

export QT_SELECT=5

rm -rf /app/workspace/compass/build_$1 # needed since binary becomes too big
mkdir -p /app/workspace/compass/build_$1
cd /app/workspace/compass/build_$1
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo .. # -DCMAKE_PREFIX_PATH=$GL_PATH $CMAKE_OPTS
make -j $(nproc)
sudo make install

cd /app/workspace/compass/docker

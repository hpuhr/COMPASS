#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

mkdir -p /app/workspace/jasterix/build_$1
cd /app/workspace/jasterix/build_$1
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make -j $(nproc)
sudo make install

cd /app/workspace/compass/docker

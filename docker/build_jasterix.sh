#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

mkdir -p /app/workspace/jasterix/$1_build
cd /app/workspace/jasterix/$1_build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ..
make -j16
sudo make install

cd /app/workspace/compass/docker

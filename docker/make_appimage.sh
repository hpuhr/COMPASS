#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

./build_jasterix.sh $1
./build_compass.sh $1
./deploy_compass.sh $1

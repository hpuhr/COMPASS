#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

docker build --build-arg USER_NAME="$(id -un)" \
  --build-arg USER_ID="$(id -u)" \
  --build-arg USER_GROUP_ID="$(id -g)" \
  -t compass/build_$1 --file Dockerfile_$1 .



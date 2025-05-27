#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

if [[ $1 == "deb9" ]]
then
  docker build --build-arg USER_NAME="$(id -un)" \
  --build-arg USER_ID="$(id -u)" \
  --build-arg USER_GROUP_ID="$(id -g)" \
  -t compass/build_deb9 --file Dockerfile_deb9 .
elif [[ $1 == "deb10" ]]
then	
  docker build --build-arg USER_NAME="$(id -un)" \
  --build-arg USER_ID="$(id -u)" \
  --build-arg USER_GROUP_ID="$(id -g)" \
  -t compass/build_deb10 --file Dockerfile_deb10 .
else
  echo "Unknown argument"
fi


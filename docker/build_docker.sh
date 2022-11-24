#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

if [[ $1 == "oldos" ]]
then
  docker build -t sk/build_ub14 --file Dockerfile_ub14 .
elif [[ $1 == "newos" ]]
then	
  docker build -t sk/build_deb9 --file Dockerfile_deb9 .
else
  echo "Unknown argument"
fi


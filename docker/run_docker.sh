#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

if [[ $1 == "oldos" ]]
then
  docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --user $(id -u):$(id -g) -v $PWD/../../..:/app -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -it sk/build_ub14:latest
elif [[ $1 == "newos" ]]
then	
  docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --user $(id -u):$(id -g) -v $PWD/../../..:/app -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -it sk/build_deb9
else
  echo "Unknown argument"
fi



#xhost +local:docker needs to be run on host

#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

if [[ $1 == "deb9" ]]
then
  docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --user $(id -u):$(id -g) -v $PWD/../../..:/app -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -it compass/build_deb9:latest
elif [[ $1 == "deb10" ]]
then	
  docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --user $(id -u):$(id -g) -v $PWD/../../..:/app -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -it compass/build_deb10
elif [[ $1 == "ub18" ]]
then	
  docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --user $(id -u):$(id -g) -v $PWD/../../..:/app -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -it compass/build_ub18 
else
  echo "Unknown argument"
fi



#xhost +local:docker needs to be run on host

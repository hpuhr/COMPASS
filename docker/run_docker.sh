#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --user $(id -u):$(id -g) -v $PWD/../../..:/app -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -it compass/build_$1 

#xhost +local:docker needs to be run on host

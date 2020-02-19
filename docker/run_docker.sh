docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined --user $(id -u):$(id -g) -v $PWD/../../..:/app -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -it sk/v2.9_oldqt:latest
#xhost +local:docker needs to be run on host

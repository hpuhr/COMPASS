FROM ubuntu:14.04

RUN groupadd -r someusers --gid=1000 
RUN useradd -r -p "$(openssl passwd -1 sk)" -g someusers -G sudo --uid=1000 --shell /bin/bash sk #--create-home 

# Update and allow for apt over HTTPS
RUN apt-get update && apt-get install -y apt-utils
RUN apt-get install software-properties-common build-essential -y
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y 
RUN add-apt-repository ppa:george-edison55/cmake-3.x
RUN apt-get update
RUN apt-get install gcc-9 g++-9 cmake -y
RUN sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-9
RUN update-alternatives --set gcc "/usr/bin/gcc-9"
RUN apt-get install libboost-all-dev liblog4cpp5-dev libssl-dev -y
RUN apt-get install libarchive-dev -y

RUN apt-get install wget -y
RUN wget https://launchpad.net/ubuntu/+archive/primary/+files/libtbb2_2017~U7-8_amd64.deb
RUN dpkg -i libtbb2_2017~U7-8_amd64.deb
RUN wget https://launchpad.net/ubuntu/+archive/primary/+files/libtbb-dev_2017~U7-8_amd64.deb
RUN dpkg -i libtbb-dev_2017~U7-8_amd64.deb

RUN cp /etc/apt/sources.list /etc/apt/sources.list~
RUN sed -Ei 's/^# deb-src /deb-src /' /etc/apt/sources.list
RUN apt-get update

RUN apt-get install qtdeclarative5-dev -y #qt5-style-plugins
#RUN apt-get remove appmenu-qt appmenu-qt5 -y
ENV QT_SELECT=5
ENV QT_QPA_PLATFORMTHEME=gtk2

RUN apt-get build-dep libopenscenegraph-dev -y
RUN apt-get build-dep libosgearth-dev -y
RUN apt-get remove libopenscenegraph-dev -y # remove lib since manually installed

RUN apt-get remove libqt4-dev qt4-qmake -y

RUN apt-get install unzip -y

#osg

RUN wget https://github.com/openscenegraph/OpenSceneGraph/archive/OpenSceneGraph-3.4.1.zip
RUN unzip OpenSceneGraph-3.4.1.zip
RUN mkdir OpenSceneGraph-OpenSceneGraph-3.4.1/build
RUN cd OpenSceneGraph-OpenSceneGraph-3.4.1/build && cmake -DCMAKE_INSTALL_PREFIX=/usr --config Release ..
RUN cd OpenSceneGraph-OpenSceneGraph-3.4.1/build && make -j 4
RUN cd OpenSceneGraph-OpenSceneGraph-3.4.1/build && make install


#RUN wget https://github.com/openscenegraph/OpenSceneGraph/archive/OpenSceneGraph-3.6.4.zip
#RUN unzip OpenSceneGraph-3.6.4.zip
#RUN mkdir OpenSceneGraph-OpenSceneGraph-3.6.4/build
#RUN cd OpenSceneGraph-OpenSceneGraph-3.6.4/build && cmake -DCMAKE_INSTALL_PREFIX=/usr --config Release ..
#RUN cd OpenSceneGraph-OpenSceneGraph-3.6.4/build && make -j 4
#RUN cd OpenSceneGraph-OpenSceneGraph-3.6.4/build && make install

#RUN wget https://github.com/openscenegraph/osgQt/archive/Qt4.zip
#RUN unzip Qt4.zip
#RUN mkdir osgQt-Qt4/build
#RUN cd osgQt-Qt4/build && cmake -DCMAKE_INSTALL_PREFIX=/usr --config Release ..
#RUN cd osgQt-Qt4/build && make -j 4
#RUN cd osgQt-Qt4/build && make install

RUN apt-get install libgeos-dev -y

# osgEarth

RUN wget https://github.com/gwaldron/osgearth/archive/osgearth-2.9.zip
RUN unzip osgearth-2.9.zip
RUN mkdir osgearth-osgearth-2.9/build
RUN cd osgearth-osgearth-2.9/build && cmake -DCMAKE_INSTALL_PREFIX=/usr -DOSGEARTH_USE_QT=TRUE -DDESIRED_QT_VERSION=5 --config Release ..
RUN cd osgearth-osgearth-2.9/build && make -j 4
RUN cd osgearth-osgearth-2.9/build && make install

#RUN wget https://github.com/gwaldron/osgearth/archive/osgearth-2.10.2.zip
#RUN unzip osgearth-2.10.2.zip
#RUN mkdir osgearth-osgearth-2.10.2/build
#RUN cd osgearth-osgearth-2.10.2/build && cmake -DCMAKE_INSTALL_PREFIX=/usr -DOSGEARTH_USE_QT=TRUE -DDESIRED_QT_VERSION=5 --config Release ..
#RUN cd osgearth-osgearth-2.10.2/build && make -j 4
#RUN cd osgearth-osgearth-2.10.2/build && make install

RUN apt-get install libmysql++-dev libeigen3-dev libmysqlclient-dev libsqlite3-dev libgdal-dev -y

RUN apt-get remove libopenscenegraph99 openscenegraph openscenegraph-plugin-osgearth osgearth -y # remove lib since manually installed
#libopenthreads-dev libopenthreads14 

RUN apt-get install locate zip
RUN ldconfig -v /usr/lib64/ 
RUN updatedb

RUN apt-get install chrpath

ENV UBUNTU_MENUPROXY=
#ENV QT_QPA_PLATFORMTHEME=
ENV QT_STYLE_OVERRIDE=adwaita

USER sk
WORKDIR /app/workspace/atsdb/docker

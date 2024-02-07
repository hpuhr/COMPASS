#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

export ARCH=x86_64
export QT_SELECT=5

cd /app/workspace/compass/
rm -r appimage/appdir/*/
mkdir -p appimage/appdir/bin/

cp /usr/bin/compass_client appimage/appdir/bin/
mkdir -p appimage/appdir/lib/
cp /usr/lib/libcompass.a appimage/appdir/lib/

#cp -r /usr/lib64/osgPlugins-3.4.1 appimage/appdir/lib/
cp -r /usr/lib/osgPlugins-3.6.5 appimage/appdir/lib/
cp -r /usr/lib64/osgPlugins-3.6.5/ appimage/appdir/lib/
cp /usr/lib64/libosgEarth* appimage/appdir/lib/

if [[ $1 == "oldos" ]]
then
  cp -r /usr/lib/libproj.so.0.7.0 appimage/appdir/lib/libproj.so
#  chrpath -r '$ORIGIN' appimage/appdir/lib/libproj.so.0.7.0
#  patchelf --set-rpath '$ORIGIN/' appimage/appdir/lib/libproj.so.0.7.0
fi
#cp /usr/lib64/osgdb_* appimage/appdir/lib/

#chrpath -r '$ORIGIN' appimage/appdir/lib/*
#chrpath -r '$ORIGIN/osgPlugins-3.4.1' appimage/appdir/lib/osgPlugins-3.4.1/*

mkdir -p appimage/appdir/compass/
cp -r data appimage/appdir/compass/
cp -r conf appimage/appdir/compass/
cp -r presets appimage/appdir/compass/

mkdir -p appimage/appdir/lib

if [[ $1 == "oldos" ]]
then
  /app/tools/linuxdeployqt-oldos-x86_64.AppImage --appimage-extract-and-run appimage/appdir/compass.desktop -appimage -bundle-non-qt-libs -verbose=2 -extra-plugins=iconengines,platformthemes/libqgtk3.so 
elif [[ $1 == "newos" ]]
then	
  /app/tools/linuxdeployqt-continuous-x86_64.AppImage --appimage-extract-and-run appimage/appdir/compass.desktop -appimage -bundle-non-qt-libs -verbose=2 -extra-plugins=iconengines,platformthemes/libqgtk3.so
fi

# -exclude-libs=/usr/lib/x86_64-linux-gnu/libprotobuf-lite.so for ubuntu 16 libproto-light issue
#-qmake=/usr/lib/x86_64-linux-gnu/qt5/bin/qmake  -show-exclude-libs

cd /app/workspace/compass/docker

mv ../COMPASS-x86_64.AppImage ../COMPASS-x86_64_$1.AppImage

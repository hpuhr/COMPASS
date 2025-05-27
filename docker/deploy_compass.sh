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

cp -r /usr/lib/osgPlugins-3.6.5 appimage/appdir/lib/
cp -r /usr/lib64/osgPlugins-3.6.5/ appimage/appdir/lib/
cp /usr/lib64/libosgEarth* appimage/appdir/lib/

mkdir -p appimage/appdir/compass/
cp -r data appimage/appdir/compass/
cp -r conf appimage/appdir/compass/
cp -r presets appimage/appdir/compass/

mkdir -p appimage/appdir/lib

if [[ $1 == "deb9" ]]
then
  /app/workspace/compass/docker/linuxdeployqt/linuxdeployqt-deb9-x86_64.AppImage --appimage-extract-and-run appimage/appdir/compass.desktop -appimage -bundle-non-qt-libs -verbose=2 -exclude-libs=libglib-2.0.so.0,libgio-2.0.so.0
elif [[ $1 == "deb10" ]]
then	
  /app/workspace/compass/docker/linuxdeployqt/linuxdeployqt-deb10-x86_64.AppImage --appimage-extract-and-run appimage/appdir/compass.desktop -appimage -bundle-non-qt-libs -verbose=2 -exclude-libs=libglib-2.0.so.0,libgio-2.0.so.0 
fi

#-extra-plugins=iconengines,platformthemes/libqgtk3.so 

cd /app/workspace/compass/docker

#mv ../COMPASS-x86_64.AppImage ../COMPASS-x86_64_$1.AppImage

#!/bin/bash

# exit when any command fails
set -e

echo "os: '$1'"

export ARCH=x86_64
export QT_SELECT=5

cd /app/workspace/compass/
rm -rf appimage/appdir/*/ # deletes all subfolders
mkdir -p appimage/appdir/bin/

#cp /usr/bin/compass_client appimage/appdir/bin/
mkdir -p appimage/appdir/lib/
#cp /usr/lib/libcompass.a appimage/appdir/lib/

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
  #/app/workspace/compass/docker/linuxdeployqt/linuxdeployqt-deb9-x86_64.AppImage --appimage-extract-and-run appimage/appdir/compass.desktop -appimage -bundle-non-qt-libs -verbose=2 -extra-plugins=iconengines,platformthemes/libqgtk3.so # -show-exclude-libs -exclude-libs=libglib-2.0.so.0,libgio-2.0.so.0
  export EXTRA_QT_PLUGINS="iconengines"
  #export EXTRA_PLATFORM_PLUGINS="platformthemes/libqgtk3.so"
  export APPIMAGE_EXTRACT_AND_RUN=1
  export QMAKE=/qt/5.8/gcc_64/bin/qmake
  export LD_LIBRARY_PATH=/qt/5.8/gcc_64/lib:/usr/lib64/:$LD_LIBRARY_PATH
  export DEPLOY_GTK_VERSION=3
  cd /app/workspace/compass/docker/linuxdeploy/
  ./linuxdeploy-x86_64.AppImage --appdir /app/workspace/compass/appimage/appdir --executable /usr/bin/compass_client --desktop-file=/app/workspace/compass/appimage/compass.desktop --plugin qt --plugin gtk --icon-file /app/workspace/compass/appimage/ats.png --output appimage
  mv COMPASS-x86_64.AppImage /app/workspace/compass/COMPASS_$1-x86_64.AppImage
elif [[ $1 == "deb10" || $1 == "ub18" ]]
then	
  /app/workspace/compass/docker/linuxdeployqt/linuxdeployqt-deb10-x86_64.AppImage --appimage-extract-and-run appimage/appdir/compass.desktop -appimage -bundle-non-qt-libs -verbose=2 -extra-plugins=iconengines,platformthemes/libqgtk3.so #-exclude-libs=libglib-2.0.so.0,libgio-2.0.so.0
fi

cd /app/workspace/compass/docker

#mv ../COMPASS-x86_64.AppImage ../COMPASS-x86_64_$1.AppImage

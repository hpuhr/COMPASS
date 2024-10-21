export ARCH=x86_64

cd /app/workspace/jasterix/
mkdir -p appimage/appdir/bin/
cp /usr/bin/jasterix_client appimage/appdir/bin/
mkdir -p appimage/appdir/lib/
cp /usr/lib/libjasterix.a appimage/appdir/lib/


if [[ $1 == "oldos" ]]
then
/app/tools/linuxdeployqt-oldos-x86_64.AppImage --appimage-extract-and-run appimage/appdir/jasterix.desktop -appimage -bundle-non-qt-libs -verbose=2
elif [[ $1 == "newos" ]]
then	
/app/tools/linuxdeployqt-continuous-x86_64.AppImage --appimage-extract-and-run appimage/appdir/jasterix.desktop -appimage -bundle-non-qt-libs -verbose=2
fi

cd definitions/
zip -r ../jasterix_definitions.zip .

cd ../analyze/
zip -r ../analyze.zip . -x ".*" -x "__*" -x "*/__*"


cd /app/workspace/compass/docker


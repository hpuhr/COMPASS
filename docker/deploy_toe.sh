export ARCH=x86_64

cd /app/workspace/toe/
mkdir -p appimage/appdir/bin/
cp /app/workspace/toe/ub14_build/bin/toe appimage/appdir/bin/
mkdir -p appimage/appdir/lib/
cp /app/workspace/toe/ub14_build/lib/liblibtoe.a appimage/appdir/lib/
/app/tools/linuxdeployqt-oldos-x86_64.AppImage --appimage-extract-and-run appimage/appdir/toe.desktop -appimage -bundle-non-qt-libs -verbose=2

cd cfg/
zip -r ../toe_cfg.zip .

#cd ../analyze/
#zip -r ../analyze.zip . -x ".*" -x "__*" -x "*/__*"


cd /app/workspace/compass/docker


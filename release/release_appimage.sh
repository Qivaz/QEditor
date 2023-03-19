#!/bin/sh
#
# Make a Linux AppImage for QEditor
#

#wget -c -nv "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
chmod a+x ./linuxdeploy-x86_64.AppImage

mkdir QEditor_appimage
appimageDir=$PWD/QEditor_appimage


# Collect all deps first.
./release_linux
appDir=$PWD/QEditor_linux


#cp $PWD/../../build-QEditor-Desktop_Qt_5_14_2_GCC_64bit-Release/QEditor .

LD_LIBRARY_PATH=$appDir
export LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH

./linuxdeploy-x86_64.AppImage -e $appDir/QEditor -d QEditor1.desktop -i ../images/QEditorIcon.png --output appimage --appdir $appimageDir
#./linuxdeploy-x86_64.AppImage -e $appDir/QEditor -i ../images/QEditorIcon.png --output appimage --appdir $appimageDir
chmod 755 QEditor-*x86_64.AppImage
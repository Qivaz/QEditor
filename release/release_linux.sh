#!/bin/sh

appName="QEditor"
appDir=$PWD/QEditor_linux
mkdir $appDir

qtDir=~/Qt5.14.2

#cp -r $qtDir/Tools/QtCreator/lib/Qt/plugins/platforms $appDir
cp -r $qtDir/5.14.2/gcc_64/plugins/platforms $appDir
cp -r $qtDir/5.14.2/gcc_64/plugins/imageformats $appDir

cp $qtDir/5.14.2/gcc_64/lib/libQt5XcbQpa.so.5 $appDir
cp $qtDir/5.14.2/gcc_64/lib/libQt5XcbQpa.so.5.14 $appDir
cp $qtDir/5.14.2/gcc_64/lib/libQt5XcbQpa.so $appDir
cp $qtDir/5.14.2/gcc_64/lib/libQt5XcbQpa.so.5.14.2 $appDir

cp $qtDir/5.14.2/gcc_64/lib/libQt5DBus.so.5 $appDir
cp $qtDir/5.14.2/gcc_64/lib/libQt5DBus.so.5.14 $appDir
cp $qtDir/5.14.2/gcc_64/lib/libQt5DBus.so $appDir
cp $qtDir/5.14.2/gcc_64/lib/libQt5DBus.so.5.14.2 $appDir

cp $qtDir/5.14.2/gcc_64/lib/libQt5Svg.so.5 $appDir
cp $qtDir/5.14.2/gcc_64/lib/libQt5Svg.so.5.14 $appDir
cp $qtDir/5.14.2/gcc_64/lib/libQt5Svg.so $appDir
cp $qtDir/5.14.2/gcc_64/lib/libQt5Svg.so.5.14.2 $appDir

cp $PWD/../third_party/qssh/lib/libQSsh.so.1 $appDir
cp $PWD/../third_party/qssh/lib/libQSsh.so.1.0.0 $appDir

cp $PWD/../../build-QEditor-Desktop_Qt_5_14_2_GCC_64bit-Release/QEditor $appDir

cp $PWD/../zh_CN.qm $appDir

cp $PWD/QEditor.sh $appDir/

cd $appDir
deplist=$(ldd $appName | awk  '{if (match($3,"/")){ printf("%s "),$3 } }')
cp $deplist .
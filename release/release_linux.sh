#!/bin/sh
exe="QEditor"
folder=$PWD/QEditor_linux
rm -rf $folder
mkdir -p $folder
cp $exe $folder

files=$(ldd $exe | awk '{if (match($3, "/")){ printf("%s "),$3 } }')
cp $files $folder


: '
appname=`basename $0 | sed s,\.sh$,,`
dirname=`dirname $0`
tmp="${dirname#?}"
if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi
LD_LIBRARY_PATH=$dirname
export LD_LIBRARY_PATH

echo "dirname: " $dirname
echo "appname: " $appname
echo "input: " $@
$dirname/$appname "$@"
'

cd $folder
LD_LIBRARY_PATH=$PWD
export LD_LIBRARY_PATH
./$exe

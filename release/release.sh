#!/bin/sh
exe="QEditor"
pwd=$PWD
files=$(ldd $exe | awk '{if (match($3, "/")){ printf("%s "),$3 } }')
cp $files $pwd

appname=`basename $0 | sed s,\.sh$,,`
dirname=`dirname $0`
tmp="${dirname#?}"
if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi
LD_LIBRARY_PATH=$dirname
export LD_LIBRARY_PATH
$dirname/$appname "$@"

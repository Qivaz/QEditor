#!/bin/sh
appname=`basename $0 | sed s,\.sh$,,`
dirname=`dirname $0`
tmp="${dirname#?}"

#echo $appname
#echo $dirname
#echo $tmp

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi

#export QT_DEBUG_PLUGINS=1
#export QT_QPA_PLATFORM_PLUGIN_PATH=./platforms

LD_LIBRARY_PATH=$dirname
#echo $dirname
#echo $LD_LIBRARY_PATH
export LD_LIBRARY_PATH
$dirname/$appname "$@"
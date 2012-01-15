#!/bin/sh

arch=`arch`
working_directory=`pwd`
current_directory=`basename $working_directory`
if [ $current_directory = "QMA1-release-desktop" ]; then
  app_name="MMDAI";
elif [ $current_directory = "QMA2-release-desktop" ]; then
  app_name="MMDAI2";
else
  echo "current directory is $current_directory but should be 'QMA1-release-build' or 'QMA2-release-build'"
  exit
fi

export INSTALL_ROOT=${working_directory}
package_name="${app_name}-linux-${arch}"

rm -rf ${app_name}.zip ${package_name}
make
make install
mkdir ${package_name}
mv ${app_name} lib locales plugins ${package_name}
find ${package_name} -exec touch -t `date +%Y%m%d0000` {} \;
zip -r ${app_name}.zip ${package_name}


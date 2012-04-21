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
mkdir lib
ldd ${app_name} | grep assimp | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../assimp/lib/% lib
ldd ${app_name} | grep vpvl2 | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libvpvl2/release/lib/% lib
ldd ${app_name} | grep Bullet | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../bullet/release/lib/% lib
ldd ${app_name} | grep LinearMath | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../bullet/release/lib/% lib
ldd ${app_name} | grep libQt | perl -ne 'print [split(/\s+/, $_)]->[3], "\n"' | xargs -i% cp % lib
cp -r `qmake -query QT_INSTALL_PLUGINS` plugins
cd plugins
rm -rf bearer
rm -rf graphicssystems
rm -rf qmltooling
rm -rf designer
rm -rf iconengines
rm -rf sqldrivers
cd ..
mv ${app_name} lib locales plugins resources ${package_name}
find ${package_name} -exec touch -t `date +%Y%m%d0000` {} \;
zip -r ${app_name}.zip ${package_name}


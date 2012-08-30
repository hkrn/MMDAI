#!/bin/sh

arch=`arch | tr '_' '-'`
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
strip ${app_name}
cp ${INSTALL_ROOT}/lib/libassimp.so ${INSTALL_ROOT}/lib/libassimp.so.2
mkdir ${package_name}
rm -rf lib
mkdir lib
mv -f ../assimp/release/lib/libassimp.so ../assimp/release/lib/libassimp.so.2
ldd ${app_name} | grep libassimp.so.2 | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../assimp/release/lib/% lib
ldd ${app_name} | grep libvpvl2.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libvpvl2/release/lib/% lib
ldd ${app_name} | grep libvpvl2qtcommon.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libvpvl2/release/lib/% lib
ldd lib/libvpvl2.so.* | grep libvpvl.so | perl -ne 'print [split(/\s+/, $_)]->[3], "\n"' | xargs -i% cp % lib
ldd ${app_name} | grep libBullet | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../bullet/release/lib/% lib
ldd ${app_name} | grep libLinearMath.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../bullet/release/lib/% lib
ldd ${app_name} | grep libavcodec.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libav/libav_release/lib/% lib
ldd ${app_name} | grep libavformat.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libav/libav_release/lib/% lib
ldd ${app_name} | grep libavutil.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libav/libav_release/lib/% lib
ldd ${app_name} | grep libswscale.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libav/libav_release/lib/% lib
ldd ${app_name} | grep libportaudio.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../portaudio/build/scons/posix/% lib
ldd ${app_name} | grep libQt | perl -ne 'print [split(/\s+/, $_)]->[3], "\n"' | xargs -i% cp % lib
strip lib/*
rm -rf plugins
cp -r `qmake -query QT_INSTALL_PLUGINS` plugins
cd plugins
rm -rf bearer
rm -rf graphicssystems
rm -rf qmltooling
rm -rf designer
rm -rf iconengines
rm -rf sqldrivers
rm -rf phonon_backend
rm -rf script
rm -rf webkit
cd ..
mv ${app_name} lib locales plugins ${package_name}
find ${package_name} -exec touch -t `date +%Y%m%d0000` {} \;
zip -r ${app_name}.zip ${package_name}


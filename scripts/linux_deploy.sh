#!/bin/sh

arch=`arch | tr '_' '-'`
working_directory=`pwd`
current_directory=`basename $working_directory`
if [ $current_directory = "QMA1-release-desktop" ]; then
  app_name="MMDAI";
elif [ $current_directory = "VPVM-release-desktop" ]; then
  app_name="MMDAI2";
else
  echo "current directory is $current_directory but should be 'QMA1-release-build' or 'VPVM-release-build'"
  exit
fi

export INSTALL_ROOT=${working_directory}
package_name="${app_name}-linux-${arch}"

rm -rf ${app_name}.zip ${package_name}
make
make install
strip ${app_name}
mkdir ${package_name}
rm -rf lib
mkdir lib
ldd ${app_name} | grep libavcodec.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libav-src/libav_release/lib/% lib
ldd ${app_name} | grep libavformat.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libav-src/libav_release/lib/% lib
ldd ${app_name} | grep libavutil.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libav-src/libav_release/lib/% lib
ldd ${app_name} | grep libswscale.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../libav-src/libav_release/lib/% lib
ldd ${app_name} | grep libportaudio.so | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../portaudio-src/release_native/lib/% lib
ldd ${app_name} | grep libIL | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp ../devil-src/release_native/lib/% lib
ldd ${app_name} | grep libILU | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp % ../devil-src/release_native/lib/% lib
ldd ${app_name} | grep libILUT | perl -ne 'print [split(/\s+/, $_)]->[1], "\n"' | xargs -i% cp % ../devil-src/release_native/lib/% lib
ldd ${app_name} | grep libQt | perl -ne 'print [split(/\s+/, $_)]->[3], "\n"' | xargs -i% cp % lib
ldd ${app_name} | grep libCg | perl -ne 'print [split(/\s+/, $_)]->[3], "\n"' | xargs -i% cp % lib
ldd ${app_name} | grep libCgGL | perl -ne 'print [split(/\s+/, $_)]->[3], "\n"' | xargs -i% cp % lib
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


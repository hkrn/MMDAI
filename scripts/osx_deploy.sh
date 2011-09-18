#!/bin/bash

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

rm -r ${app_name}.app;
rm ${app_name}.dmg;
make || exit;
macdeployqt $app_name.app;
echo "deployed $app_name.app";
cd ${app_name}.app/Contents/Frameworks || exit;
rm -rf QtSql.framework;
rm -rf QtXmlPatterns.framework;
rm -rf QtDeclarative.framework;
rm -rf QtNetwork.framework;
rm -rf QtScript.framework;
rm -rf QtSvg.framework;
echo 'removed unused Qt frameworks';
cd ../PlugIns || exit;
rm -rf bearer
rm -rf graphicssystems
rm -rf qmltooling
cd ../../.. || exit
echo 'removed unused Qt plugins';
hdiutil create ${app_name}.dmg -srcfolder ${app_name}.app -format UDZO -volname $app_name


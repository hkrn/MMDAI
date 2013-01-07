#!/bin/bash

working_directory=`pwd`
current_directory=`basename $working_directory`
case "$current_directory" in
  QMA1-*)
         app_name="MMDAI";
         ;;
  VPVM-*)
         app_name="MMDAI2";
         ;;
  *)
     echo "current directory is $current_directory but should be 'QMA1-*' or 'VPVM-*'"
     exit
     ;;
esac

package_app_name=${app_name}.app
package_dmg_name="${app_name}-osx-intel.dmg"
package_zip_name="${app_name}-osx-intel.zip"

rm -rf ${package_app_name} ${package_dmg_name} ${package_zip_name};
make || exit;
macdeployqt ${package_app_name};
cp -r /Library/Frameworks/Cg.framework ${package_app_name}/Contents/Frameworks
echo "deployed ${package_app_name}";
cd ${package_app_name}/Contents/Frameworks || exit;
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
find ${package_app_name} -exec touch -t `date +%Y%m%d0000` {} \;
hdiutil create ${package_dmg_name} -srcfolder ${package_app_name} -format UDZO -volname $app_name
zip ${package_zip_name} ${package_dmg_name}


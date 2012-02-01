QT += testlib
QT -= gui
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app

macx {
  QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
}
unix {
  CONFIG(debug, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/debug-shared
  CONFIG(release, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/release-shared
  CONFIG(debug, debug|release):MOC_DIR = $${OUT_PWD}/.moc/debug-shared
  CONFIG(release, debug|release):MOC_DIR = $${OUT_PWD}/.moc/release-shared
}
linux-* {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/../../debug/lib
  QMAKE_RPATHDIR += \$\$ORIGIN/../../../bullet/debug/lib
  QMA_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${QMA_RPATH}\'
  QMAKE_RPATHDIR =
}

LIBS += -L../../debug/lib -lvpvl2_debug -L../../../bullet/debug/lib \
        -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath
INCLUDEPATH += ../../include ../../debug/include ../../../bullet/src /opt/local/include/libxml2 /usr/include/libxml2

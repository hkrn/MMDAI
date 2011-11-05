CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
QT += gui

macx {
  QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
}
unix {
  CONFIG(debug, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/debug-shared
  CONFIG(release, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/release-shared
  CONFIG(debug, debug|release):MOC_DIR = $${OUT_PWD}/.moc/debug-shared
  CONFIG(release, debug|release):MOC_DIR = $${OUT_PWD}/.moc/release-shared
}

LIBS += -L../../libvpvl/debug/lib -lvpvl_debug -L../../bullet/debug/lib \
        -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath
INCLUDEPATH += ../../libvpvl/include ../../libvpvl/debug/include ../../bullet/src

SOURCES += pvrtc.cc


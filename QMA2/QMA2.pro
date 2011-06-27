QT += core gui opengl
TARGET = MMDAI
TEMPLATE = app


LIBS += -L../libvpvl
CONFIG(debug, debug|release) {
  LIBS += -lvpvl_debug
}
CONFIG(release, debug|release) {
  LIBS += -lvpvl
}

LIBS += -L/usr/local/lib -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lGLEW
INCLUDEPATH += ../libvpvl/include /usr/local/include/bullet

SOURCES += main.cc\
           MainWindow.cc \
           SceneWidget.cc

HEADERS  += MainWindow.h \
            SceneWidget.h

RESOURCES += resources/QMA2.qrc

macx {
  QMAKE_INFO_PLIST = resources/Info.plist
  CONFIG(debug, debug|release) {
    CONFIG += x86_64
  }
}

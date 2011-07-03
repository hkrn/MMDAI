QT += core gui opengl
TARGET = MMDAI2
TEMPLATE = app


LIBS += -L../libvpvl/lib
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
           SceneWidget.cc \
           TimelineWidget.cc \
    HandleWidget.cc \
    FaceWidget.cc \
    PerspectionWidget.cc

HEADERS  += MainWindow.h \
            SceneWidget.h \
            TimelineWidget.h \
    HandleWidget.h \
    FaceWidget.h \
    PerspectionWidget.h

RESOURCES += resources/QMA2.qrc

macx {
  QMAKE_INFO_PLIST = resources/Info.plist
  CONFIG(debug, debug|release) {
    CONFIG += x86_64
  }
}

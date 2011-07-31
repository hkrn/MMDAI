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

LIBS += -L/opt/local/lib -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lGLEW
INCLUDEPATH += ../libvpvl/include /opt/local/include /opt/local/include/bullet

SOURCES += main.cc\
           MainWindow.cc \
           SceneWidget.cc \
           TimelineWidget.cc \
    HandleWidget.cc \
    FaceWidget.cc \
    CameraPerspectiveWidget.cc \
    TransformWidget.cc \
    TabWidget.cc \
    MotionBaseModel.cc \
    BoneMotionModel.cc \
    FaceMotionModel.cc \
    BoneDialog.cc

HEADERS  += MainWindow.h \
            SceneWidget.h \
            TimelineWidget.h \
    HandleWidget.h \
    FaceWidget.h \
    CameraPerspectiveWidget.h \
    TransformWidget.h \
    util.h \
    TabWidget.h \
    MotionBaseModel.h \
    BoneMotionModel.h \
    FaceMotionModel.h \
    BoneDialog.h

RESOURCES += resources/QMA2.qrc

macx {
  QMAKE_INFO_PLIST = resources/Info.plist
  CONFIG(debug, debug|release) {
    CONFIG += x86_64
  }
}

FORMS += \
    TransformWidget.ui \
    MainWindow.ui \
    TabWidget.ui \
    BoneDialog.ui

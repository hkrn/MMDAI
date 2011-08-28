QT += core gui opengl
TARGET = MMDAI2
TEMPLATE = app

CONFIG(debug, debug|release) {
  LIBS += -L../libvpvl/debug/lib -L../bullet/debug/lib -lvpvl_debug
}
CONFIG(release, debug|release) {
  LIBS += -L../libvpvl/release/lib -L../bullet/release/lib -lvpvl
}

exists(/opt/local/lib):LIBS += -L/opt/local/lib
exists(/opt/local/include):INCLUDEPATH += /opt/local/include
exists(/usr/local/lib):LIBS += -L/usr/local/lib
exists(/usr/local/include):INCLUDEPATH += /usr/local/include

LIBS += -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lGLEW
INCLUDEPATH += ../libvpvl/include ../bullet/src

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
    BoneDialog.cc \
    TimelineTabWidget.cc \
    InterpolationWidget.cc \
    VPDFile.cc \
    LicenseWidget.cc \
    Delegate.cc \
    SceneLoader.cc

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
    BoneDialog.h \
    TimelineTabWidget.h \
    InterpolationWidget.h \
    VPDFile.h \
    LicenseWidget.h \
    Delegate.h \
    World.h \
    SceneLoader.h

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
    BoneDialog.ui

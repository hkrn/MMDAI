QT += core gui opengl
TARGET = MMDAI2
TEMPLATE = app

exists(/opt/local/lib):LIBS += -L/opt/local/lib
exists(/opt/local/include):INCLUDEPATH += /opt/local/include
exists(/usr/local/lib):LIBS += -L/usr/local/lib
exists(/usr/local/include):INCLUDEPATH += /usr/local/include

LIBS += -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath
win32:LIBS += -lglew32
unix:LIBS += -lGLEW
INCLUDEPATH += ../libvpvl/include ../bullet/src

CONFIG(debug, debug|release) {
  win32:LIBS += -L../libvpvl/debug-mingw/lib -L../bullet/debug-mingw/lib
  unix:LIBS  += -L../libvpvl/debug/lib -L../bullet/debug/lib
  LIBS       += -lvpvl_debug
}
CONFIG(release, debug|release) {
  win32:LIBS += -L../libvpvl/release-mingw/lib -L../bullet/debug-mingw/lib
  unix:LIBS  += -L../libvpvl/release/lib -L../bullet/release/lib
  LIBS       += -lvpvl
}

# based on QtCreator's qmake spec
DEFINES += QT_NO_CAST_TO_ASCII
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
unix {
  CONFIG(debug, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/debug-shared
  CONFIG(release, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/release-shared
  CONFIG(debug, debug|release):MOC_DIR = $${OUT_PWD}/.moc/debug-shared
  CONFIG(release, debug|release):MOC_DIR = $${OUT_PWD}/.moc/release-shared
  RCC_DIR = $${OUT_PWD}/.rcc
  UI_DIR = $${OUT_PWD}/.uic
}
macx {
  QMAKE_CXXFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/PlugIns/
  QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../
  QMAKE_INFO_PLIST = resources/Info.plist
  QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
  CONFIG(debug, debug|release) {
    CONFIG += x86_64
  }
}
else:linux-* {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/..
  QMA_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${QMA_RPATH}\'
  QMAKE_RPATHDIR =
}

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
    SceneLoader.h \
    Grid.h \
    Application.h

RESOURCES += resources/QMA2.qrc

FORMS += \
    TransformWidget.ui \
    #MainWindow.ui \
    BoneDialog.ui

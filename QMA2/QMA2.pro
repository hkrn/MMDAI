QT += core gui opengl
TARGET = MMDAI2
TEMPLATE = app

# Linux, Darwin(OSX), etc...
exists(/opt/local/lib):LIBS += -L/opt/local/lib
exists(/opt/local/include):INCLUDEPATH += /opt/local/include
exists(/opt/local/include/libxml2):INCLUDEPATH += /opt/local/include/libxml2
exists(/usr/include/libxml2):INCLUDEPATH += /usr/include/libxml2
exists(/usr/local/lib):LIBS += -L/usr/local/lib
exists(/usr/local/include):INCLUDEPATH += /usr/local/include
exists(/usr/local/include/libxml2):INCLUDEPATH += /usr/local/include/libxml2

# GLEW and assimp and OpenCV
exists(../glew/lib):LIBS += -L../glew/lib
exists(../glew/include):INCLUDEPATH += ../glew/include
exists(../assimp/lib):LIBS += -L../assimp/lib -lassimp
exists(../assimp/include):INCLUDEPATH += ../assimp/include
exists(../opencv/include):INCLUDEPATH += ../opencv/include
exists(../opencv/modules/core/include):INCLUDEPATH += ../opencv/modules/core/include
exists(../opencv/modules/highgui/include):INCLUDEPATH += ../opencv/modules/highgui/include

# Basic Configuration
LIBS += -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lxml2
win32:LIBS += -lglew32

# VPVL and others configuration
INCLUDEPATH += ../libvpvl/include ../libvpvl/debug/include ../bullet/src
win32:INCLUDEPATH += ../libvpvl/msvc-build/include

# configuration by build type
CONFIG(debug, debug|release) {
  win32:LIBS       += -L../libvpvl/msvc-build/lib/debug -L../bullet/msvc-build/lib/debug -lvpvl
  macx:LIBS        += -framework Cg
  unix:LIBS        += -L../libvpvl/debug/lib -L../bullet/debug/lib -lvpvl_debug
  unix:INCLUDEPATH += ../libvpvl/debug/include
  exists(../assimp/code/debug):LIBS += -L../assimp/code/debug -lassimp
  exists(../opencv/debug/lib) {
    LIBS += -L../opencv/debug/lib -lopencv_core -lopencv_highgui
    DEFINES += OPENCV_FOUND
  }
  exists(../opencv/msvc-build/lib/Debug) {
    LIBS += -L../opencv/msvc-build/lib/Debug -lopencv_core231d -lopencv_highgui231d
    DEFINES += OPENCV_FOUND
  }
}
CONFIG(release, debug|release) {
  win32:LIBS       += -L../libvpvl/msvc-build/lib/release -L../bullet/msvc-build/lib/release -lvpvl
  unix:LIBS        += -L../libvpvl/release/lib -L../bullet/release/lib -lvpvl
  unix:INCLUDEPATH += ../libvpvl/release/include
  exists(../assimp/code/release):LIBS += -L../assimp/code/release -lassimp
  exists(../opencv/release/lib) {
    LIBS += -L../opencv/release/lib -lopencv_core -lopencv_highgui
    DEFINES += OPENCV_FOUND
  }
  exists(../opencv/msvc-build/lib/Release) {
    LIBS += -L../opencv/msvc-build/lib/Release -lopencv_core231 -lopencv_highgui231
    DEFINES += OPENCV_FOUND
  }
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

translations.files = resources/translations/MMDAI2_ja.qm \
                     $$[QT_INSTALL_TRANSLATIONS]/qt_ja.qm
win32 {
  RC_FILE = resources/icons/app.rc
}
macx {
  ICON = resources/icons/app.icns
  QMAKE_CXXFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/PlugIns/
  QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../
  QMAKE_INFO_PLIST = resources/Info.plist
  QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
  translations.path = Contents/Resources
  QMAKE_BUNDLE_DATA += translations
  CONFIG(debug, debug|release) {
    CONFIG += x86_64
  }
  # must add -DCMAKE_CXX_FLAGS="-fvisibility=hidden -fvisibility-inlines-hidden" to libvpvl and bullet
  CONFIG(release, debug|release) {
    CONFIG += x86 x86_64
  }
}
linux-* {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/..
  QMA_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${QMA_RPATH}\'
  QMAKE_RPATHDIR =
   LIBS += -lGLEW
}
!macx {
  translations.path = /locales
  INSTALLS += translations
}

SOURCES += main.cc \
    common/SceneWidget.cc \
    common/VPDFile.cc \
    common/SceneLoader.cc \
    common/LoggerWidget.cc \
    models/PMDMotionModel.cc \
    models/BoneMotionModel.cc \
    models/FaceMotionModel.cc \
    models/SceneMotionModel.cc \
    dialogs/BoneDialog.cc \
    dialogs/ExportVideoDialog.cc \
    dialogs/PlaySettingDialog.cc \
    dialogs/EdgeOffsetDialog.cc \
    widgets/TimelineWidget.cc \
    widgets/FaceWidget.cc \
    widgets/CameraPerspectiveWidget.cc \
    widgets/TransformWidget.cc \
    widgets/TabWidget.cc \
    widgets/TimelineTabWidget.cc \
    widgets/InterpolationWidget.cc \
    widgets/LicenseWidget.cc \
    widgets/AssetWidget.cc \
    MainWindow.cc \
    BoneUIDelegate.cc

HEADERS  += \
    common/SceneWidget.h \
    common/Handles.h \
    common/util.h \
    common/Delegate.h \
    common/World.h \
    common/SceneLoader.h \
    common/Grid.h \
    common/Application.h \
    common/VPDFile.h \
    common/InfoPanel.h \
    common/DebugDrawer.h \
    common/LoggerWidget.h \
    models/MotionBaseModel.h \
    models/BoneMotionModel.h \
    models/FaceMotionModel.h \
    models/SceneMotionModel.h \
    models/PMDMotionModel.h \
    dialogs/BoneDialog.h \
    dialogs/ExportVideoDialog.h \
    dialogs/PlaySettingDialog.h \
    dialogs/EdgeOffsetDialog.h \
    widgets/TimelineWidget.h \
    widgets/FaceWidget.h \
    widgets/CameraPerspectiveWidget.h \
    widgets/TransformWidget.h \
    widgets/TabWidget.h \
    widgets/TimelineTabWidget.h \
    widgets/InterpolationWidget.h \
    widgets/LicenseWidget.h \
    widgets/AssetWidget.h \
    MainWindow.h \
    BoneUIDelegate.h

CODECFORTR = UTF-8
RESOURCES += resources/QMA2.qrc
TRANSLATIONS += resources/translations/MMDAI2.ts

FORMS += \
    TransformWidget.ui
















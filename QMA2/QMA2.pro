QT += core gui opengl
TARGET = MMDAI2
TEMPLATE = app
DEFINES += IS_QMA2

# CMake prefix path (mainly for win32)
exists($$(CMAKE_PREFIX_PATH)/include):INCLUDEPATH += "$$(CMAKE_PREFIX_PATH)/include"
exists($$(CMAKE_PREFIX_PATH)/lib):LIBS += -L "$$(CMAKE_PREFIX_PATH)/lib"

# Linux, Darwin(OSX), etc...
exists(/usr/local/lib):LIBS += -L/usr/local/lib
exists(/usr/local/include):INCLUDEPATH += /usr/local/include

# libxml2
exists(/usr/include/libxml2):INCLUDEPATH += /usr/include/libxml2
exists(/usr/local/include/libxml2):INCLUDEPATH += /usr/local/include/libxml2

# PortAudio
exists(../portaudio/build/scons/posix):LIBS += -L../portaudio/build/scons/posix
exists(../portaudio/build/scons/darwin):LIBS += -L../portaudio/build/scons/darwin
exists(../portaudio/include):INCLUDEPATH += ../portaudio/include

# libxml2
exists(/usr/include/libxml2):INCLUDEPATH += /usr/include/libxml2
exists(/usr/local/include/libxml2):INCLUDEPATH += /usr/local/include/libxml2

# libvpvl and base libraries (MMDAgent for win32)
ASSIMP_PATH = ../assimp
BULLET_PATH = ../bullet
VPVL_PATH = ../libvpvl
VPVL2_PATH = ../libvpvl2
MMDA_PATH = ../../MMDAgent/MMDAgent

# Required libraries
LIBS += -L$${ASSIMP_PATH}/lib \
        -lassimp \
        -lBulletCollision \
        -lBulletDynamics \
        -lBulletSoftBody \
        -lLinearMath \
        -lportaudio \
        -lavcodec \
        -lavformat \
        -lavutil \
        -lswscale \
        -lxml2

# VPVL and others configuration
INCLUDEPATH +=  $${VPVL_PATH}/include \
                $${VPVL2_PATH}/include \
                $${ASSIMP_PATH}/include \
                $${BULLET_PATH}/src

win32:INCLUDEPATH += $${VPVL2_PATH}/msvc-build/include \
                     $${MMDA_PATH} \
                     $${MMDA_PATH}/Library_Julius/include \
                     $${MMDA_PATH}/Library_Open_JTalk/include \
                     $${MMDA_PATH}/Library_hts_engine_API/include \
                     $${MMDA_PATH}/Library_PortAudio/include

# configuration by build type
CONFIG(debug, debug|release) {
  win32:LIBS       += -L$${VPVL2_PATH}/msvc-build/lib/debug \
                      -L$${BULLET_PATH}/msvc-build/lib/debug
  macx:LIBS        += -framework OpenCL
  unix:LIBS        += -L$${BULLET_PATH}/debug/lib \
                      -L$${VPVL_PATH}/debug/lib \
                      -L$${VPVL2_PATH}/debug/lib
  unix:INCLUDEPATH += $${VPVL_PATH}/debug/include \
                      $${VPVL2_PATH}/debug/include
  LIBS             += -lvpvl_debug -lvpvl2_debug
}
CONFIG(release, debug|release) {
  win32:LIBS       += -L$${VPVL2_PATH}/msvc-build/lib/release \
                      -L$${BULLET_PATH}/msvc-build/lib/release
  macx:LIBS        += -framework OpenCL
  unix:LIBS        += -L$${BULLET_PATH}/release/lib \
                      -L$${VPVL_PATH}/release/lib \
                      -L$${VPVL2_PATH}/release/lib
  LIBS             += -lvpvl -lvpvl2
  unix:INCLUDEPATH += $${VPVL_PATH}/release/include \
                      $${VPVL2_PATH}/release/include
}
macx:LIBS += -framework OpenCL -framework CoreServices -framework OpenCL -framework CoreAudio -framework AudioToolbox -framework AudioUnit
linux-*:LIBS += -lGLU

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
  translations.path = Contents/Resources
  QMAKE_BUNDLE_DATA += translations
  DEFINES += USE_FILE32API
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
  QMAKE_RPATHDIR += \$\$ORIGIN/lib
  QMA_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${QMA_RPATH}\'
  QMAKE_RPATHDIR =
  libraries.path = /lib
  libraries.files = ../bullet/release/lib/libBulletCollision.so.* \
                    ../bullet/release/lib/libBulletDynamics.so.* \
                    ../bullet/release/lib/libBulletSoftBody.so.* \
                    ../bullet/release/lib/libLinearMath.so.* \
                    ../libvpvl/release/lib/libvpvl.so.* \
                    ../assimp/lib/libassimp.so.2 \
                    ../portaudio/build/scons/posix/libportaudio.so \
                    ../libav/libavcodec/libavcodec.so.* \
                    ../libav/libavformat/libavformat.so.* \
                    ../libav/libavutil/libavutil.so.* \
                    ../libav/libswscale/libswscale.so.* \
                    $$[QT_INSTALL_LIBS]/libQtCore.so.4 \
                    $$[QT_INSTALL_LIBS]/libQtGui.so.4 \
                    $$[QT_INSTALL_LIBS]/libQtOpenGL.so.4
  plugins.path = /plugins
  plugins.files = $$[QT_INSTALL_PLUGINS]/*
  INSTALLS += libraries plugins
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
    models/MorphMotionModel.cc \
    models/SceneMotionModel.cc \
    dialogs/BoneDialog.cc \
    dialogs/ExportVideoDialog.cc \
    dialogs/FrameWeightDialog.cc \
    dialogs/InterpolationDialog.cc \
    dialogs/PlaySettingDialog.cc \
    widgets/TimelineWidget.cc \
    widgets/MorphWidget.cc \
    widgets/CameraPerspectiveWidget.cc \
    widgets/TabWidget.cc \
    widgets/TimelineTabWidget.cc \
    widgets/TimelineTreeView.cc \
    widgets/LicenseWidget.cc \
    widgets/AssetWidget.cc \
    widgets/ModelTabWidget.cc \
    widgets/InterpolationGraphWidget.cc \
    MainWindow.cc \
    BoneUIDelegate.cc \
    common/Handles.cc \
    dialogs/FrameSelectionDialog.cc \
    widgets/ModelInfoWidget.cc \
    video/VideoEncoder.cc \
    ScenePlayer.cc \
    dialogs/GravitySettingDialog.cc \
    video/AudioDecoder.cc \
    video/AVCommon.cc \
    video/AudioPlayer.cc \
    dialogs/RenderOrderDialog.cc \
    unzip/unzip.c \
    unzip/ioapi.c \
    common/Archive.cc \
    widgets/SceneLightWidget.cc \
    widgets/ModelSettingWidget.cc \
    dialogs/ShadowMapSettingDialog.cc \
    dialogs/BackgroundImageSettingDialog.cc

HEADERS  += \
    common/SceneWidget.h \
    common/Handles.h \
    common/util.h \
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
    models/MorphMotionModel.h \
    models/SceneMotionModel.h \
    models/PMDMotionModel.h \
    dialogs/BoneDialog.h \
    dialogs/ExportVideoDialog.h \
    dialogs/FrameWeightDialog.h \
    dialogs/InterpolationDialog.h \
    dialogs/PlaySettingDialog.h \
    widgets/TimelineWidget.h \
    widgets/MorphWidget.h \
    widgets/CameraPerspectiveWidget.h \
    widgets/TabWidget.h \
    widgets/TimelineTabWidget.h \
    widgets/TimelineTreeView.h \
    widgets/LicenseWidget.h \
    widgets/AssetWidget.h \
    widgets/ModelTabWidget.h \
    widgets/InterpolationGraphWidget.h \
    MainWindow.h \
    BoneUIDelegate.h \
    dialogs/FrameSelectionDialog.h \
    widgets/ModelInfoWidget.h \
    video/VideoEncoder.h \
    ScenePlayer.h \
    common/TextureDrawHelper.h \
    dialogs/GravitySettingDialog.h \
    video/AudioDecoder.h \
    video/AVCommon.h \
    video/AudioPlayer.h \
    dialogs/RenderOrderDialog.h \
    unzip/unzip.h \
    unzip/ioapi.h \
    common/Archive.h \
    widgets/SceneLightWidget.h \
    widgets/ModelSettingWidget.h \
    dialogs/ShadowMapSettingDialog.h \
    common/BackgroundImage.h \
    dialogs/BackgroundImageSettingDialog.h

CODECFORTR = UTF-8
RESOURCES += resources/QMA2.qrc
TRANSLATIONS += resources/translations/MMDAI2.ts

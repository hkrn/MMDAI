QT += core gui opengl
QT += widgets concurrent
TARGET = MMDAI2
TEMPLATE = app
DEFINES += IS_VPVM

# libvpvl2 and base libraries (MMDAgent for win32)
ASSIMP_PATH = ../assimp-src
BULLET_PATH = ../bullet-src
VPVL1_PATH = ../libvpvl
VPVL2_PATH = ../libvpvl2
MMDA_PATH = ../../MMDAgent/MMDAgent
LIBAV_PATH = ../libav-src
NVTT_PATH = ../nvtt-src
PORTAUDIO_PATH = ../portaudio-src

# CMake prefix path (mainly for win32)
exists($$(CMAKE_PREFIX_PATH)/include):INCLUDEPATH += "$$(CMAKE_PREFIX_PATH)/include"
exists($$(CMAKE_PREFIX_PATH)/lib):LIBS += -L "$$(CMAKE_PREFIX_PATH)/lib"

# libxml2
exists(/usr/include/libxml2):INCLUDEPATH += /usr/include/libxml2
exists(/usr/local/include/libxml2):INCLUDEPATH += /usr/local/include/libxml2

# VPVL and others configuration
INCLUDEPATH +=  $${VPVL2_PATH}/include \
                $${ASSIMP_PATH}/include \
                $${BULLET_PATH}/src \
                $${NVTT_PATH}/src

win32:INCLUDEPATH += $${VPVL2_PATH}/msvc-build/include \
                     $${MMDA_PATH} \
                     $${MMDA_PATH}/Library_Julius/include \
                     $${MMDA_PATH}/Library_Open_JTalk/include \
                     $${MMDA_PATH}/Library_hts_engine_API/include \
                     $${MMDA_PATH}/Library_PortAudio/include

# configuration by build type
CONFIG(debug, debug|release) {
  unix:LIBS        += -L$${ASSIMP_PATH}/debug/lib \
                      -L$${BULLET_PATH}/debug/lib \
                      -L$${VPVL1_PATH}/debug/lib \
                      -L$${VPVL2_PATH}/debug/lib \
                      -L$${PORTAUDIO_PATH}/debug_native/lib \
                      -L$${LIBAV_PATH}/debug_native/lib \
                      -L$${NVTT_PATH}/build-debug/lib
  unix:INCLUDEPATH += $${VPVL2_PATH}/debug/include \
                      $${PORTAUDIO_PATH}/debug_native/include
  # should not change link order because of static library link order
  LIBS             +=  -lvpvl2qtcommon_debug -lvpvl2_debug -lvpvl_debug
  INCLUDEPATH      += $${LIBAV_PATH}/debug_native/include
}
CONFIG(release, debug|release) {
  unix:LIBS        += -L$${ASSIMP_PATH}/release/code \
                      -L$${BULLET_PATH}/release/lib \
                      -L$${VPVL1_PATH}/release/lib \
                      -L$${VPVL2_PATH}/release/lib \
                      -L$${PORTAUDIO_PATH}/release_native/lib \
                      -L$${LIBAV_PATH}/release_native/lib \
                      -L$${NVTT_PATH}/build-release/lib
  unix:INCLUDEPATH += $${VPVL2_PATH}/release/include
  # should not change link order because of static library link order
  LIBS             += -lvpvl2qtcommon -lvpvl2 -lvpvl
  INCLUDEPATH      += $${LIBAV_PATH}/release_native/include
}

# Required libraries
LIBS += -lassimp \
        -lBulletSoftBody \
        -lBulletDynamics \
        -lBulletCollision \
        -lLinearMath \
        -lportaudio \
        -lavcodec \
        -lavformat \
        -lavutil \
        -lswscale \
        -lnvimage \
        -lnvmath \
        -lnvcore \
        -lxml2

macx:LIBS += -framework OpenCL \
             -framework CoreServices \
             -framework Cg \
             -framework CoreAudio \
             -framework AudioToolbox \
             -framework AudioUnit
linux-*:LIBS += -lGLU -lCg -lCgGL

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

translations.files = resources/translations/VPVM_ja.qm \
                     $$[QT_INSTALL_TRANSLATIONS]/qt_ja.qm
win32 {
  RC_FILE = resources/icons/app.rc
}
macx {
  ICON = resources/icons/app.icns
  QMAKE_CXXFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../
  QMAKE_INFO_PLIST = resources/Info.plist
  translations.path = Contents/Resources
  QMAKE_BUNDLE_DATA += translations
  CONFIG(debug, debug|release) {
    CONFIG += x86_64
  }
  # must add -DCMAKE_CXX_FLAGS="-fvisibility=hidden -fvisibility-inlines-hidden" to libvpvl2 and bullet
  CONFIG(release, debug|release) {
    CONFIG += x86 x86_64
  }
  LIBS += -framework Cocoa
}
linux-* {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/lib
  QMA_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${QMA_RPATH}\'
  QMAKE_RPATHDIR =
  libraries.path = /lib
  libraries.files = $${LIBAV_PATH}/release_native/lib/libavcodec.so.* \
                    $${LIBAV_PATH}/release_native/lib/libavformat.so.* \
                    $${LIBAV_PATH}/release_native/lib/libavutil.so.* \
                    $${LIBAV_PATH}/release_native/lib/libswscale.so.* \
                    $${DEVIL_PATH}/release_native/lib/libIL.so.* \
                    $${DEVIL_PATH}/release_native/lib/libILU.so.* \
                    $${DEVIL_PATH}/release_native/lib/libILUT.so.* \
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
    common/StringHelper.cc \
    models/PMDMotionModel.cc \
    models/BoneMotionModel.cc \
    models/MorphMotionModel.cc \
    models/MotionBaseModel.cc \
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
    widgets/SceneLightWidget.cc \
    widgets/ModelSettingWidget.cc \
    dialogs/ShadowMapSettingDialog.cc \
    dialogs/BackgroundImageSettingDialog.cc

HEADERS  += \
    common/SceneWidget.h \
    common/Handles.h \
    common/util.h \
    common/SceneLoader.h \
    common/Grid.h \
    common/Application.h \
    common/VPDFile.h \
    common/InfoPanel.h \
    common/DebugDrawer.h \
    common/LoggerWidget.h \
    common/StringHelper.h \
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
    video/IAudioDecoder.h \
    video/IVideoEncoder.h \
    dialogs/RenderOrderDialog.h \
    widgets/SceneLightWidget.h \
    widgets/ModelSettingWidget.h \
    dialogs/ShadowMapSettingDialog.h \
    common/BackgroundImage.h \
    dialogs/BackgroundImageSettingDialog.h \
    common/VertexBundle.h

CODECFORTR = UTF-8
RESOURCES += resources/VPVM.qrc
TRANSLATIONS += resources/translations/VPVM.ts

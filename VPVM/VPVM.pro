QT += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4):QT += widgets concurrent

TARGET = MMDAI2
TEMPLATE = app
DEFINES += IS_VPVM
macx:DEFINES += USE_FILE32API

# libvpvl2 and base libraries
ALSOFT_PATH = ../openal-soft-src
ALURE_PATH = ../alure-src
ASSIMP_PATH = ../assimp-src
BULLET_PATH = ../bullet-src
VPVL1_PATH = ../libvpvl
VPVL2_PATH = ../libvpvl2
LIBAV_PATH = ../libav-src
NVTT_PATH = ../nvtt-src
GLEW_PATH = ../glew-src
GLM_PATH = ../glm-src
PORTAUDIO_PATH = ../portaudio-src
LIBXML2_PATH = ../libxml2-src
ICU_PATH = ../icu-src
ZLIB_PATH = ../zlib-src
INSTALL_ROOT_DIR = install-root

# CMake prefix path (mainly for win32)
exists($$(CMAKE_PREFIX_PATH)/include):INCLUDEPATH += "$$(CMAKE_PREFIX_PATH)/include"
exists($$(CMAKE_PREFIX_PATH)/lib):LIBS += -L"$$(CMAKE_PREFIX_PATH)/lib"

# configuration by build type
CONFIG(debug, debug|release) {
  BUILD_TYPE = debug
  VPVL2_LIBRARY_SUFFIX = _debug
}
CONFIG(release, debug|release):BUILD_TYPE = release

# configuration by CPU architecure
CONFIG(x86, x86|x86_64):BUILD_TYPE_SUFFIX = -32
CONFIG(x86_64, x86|x86_64):BUILD_TYPE_SUFFIX = -64

greaterThan(QT_MAJOR_VERSION, 4):BUILD_DIRECTORY_VPVL2_SUFFIX  = -qt5
BUILD_DIRECTORY = build-$${BUILD_TYPE}
BASE_LIBRARY_PATH = $${BUILD_DIRECTORY}/lib
PRODUCT_DIRECTORY = $${BUILD_DIRECTORY}/$${INSTALL_ROOT_DIR}
VPVL2_BUILD_DIRECTORY = $${BUILD_DIRECTORY}$${BUILD_DIRECTORY_VPVL2_SUFFIX}
win32 {
  LIBRARY_PATH = $${BASE_LIBRARY_PATH}/$${BUILD_TYPE}
  ASSIMP_LIBRARY_PATH = $${ASSIMP_PATH}/$${BUILD_DIRECTORY}/code/$${BUILD_TYPE}
  BULLET_LIBRARY_PATH = $${BULLET_PATH}/$${LIBRARY_PATH}
  GLEW_LIBRARY_PATH = $${GLEW_PATH}/lib
  ICU_LIBRARY_PATH = $${ICU_PATH}/lib
  LIBXML2_LIBRARY_PATH = $${LIBXML2_PATH}/win32/bin.msvc
  VPVL1_LIBRARY_PATH = $${VPVL1_PATH}/$${LIBRARY_PATH}
  VPVL2_LIBRARY_PATH = $${VPVL2_PATH}/$${VPVL2_BUILD_DIRECTORY}/lib/$${BUILD_TYPE}
  ZLIB_LIBRARY_PATH = $${ZLIB_PATH}/$${LIBRARY_PATH}
  LIBS += -L$${NVTT_PATH}/$${BUILD_DIRECTORY}/src/nvcore/$${BUILD_TYPE} \
          -L$${NVTT_PATH}/$${BUILD_DIRECTORY}/src/nvimage/$${BUILD_TYPE} \
          -L$${NVTT_PATH}/$${BUILD_DIRECTORY}/src/nvmath/$${BUILD_TYPE} \
          -L$$(TBB_INSTALL_DIR)/lib/$$(TBB_ARCH_PLATFORM)
}
!win32 {
  CONFIG(debug, debug|release) {
    VPVL2_LIBRARY_PATH = $${VPVL2_PATH}/$${BUILD_DIRECTORY}/lib
    VPVL2_INCLUDE_PATH = $${VPVL2_PATH}/include $${VPVL2_PATH}/$${BUILD_DIRECTORY}/include
  }
  else {
    VPVL2_LIBRARY_PATH = $${VPVL2_PATH}/$${BUILD_DIRECTORY}/$${INSTALL_ROOT_DIR}/lib
    VPVL2_INCLUDE_PATH = $${VPVL2_PATH}/$${PRODUCT_DIRECTORY}/include
  }
  ASSIMP_LIBRARY_PATH = $${ASSIMP_PATH}/$${PRODUCT_DIRECTORY}/lib
  BULLET_LIBRARY_PATH = $${BULLET_PATH}/$${PRODUCT_DIRECTORY}/lib
  GLEW_LIBRARY_PATH = $${GLEW_PATH}/lib
  ICU_LIBRARY_PATH = $${ICU_PATH}/$${PRODUCT_DIRECTORY}/lib
  LIBXML2_LIBRARY_PATH = $${LIBXML2_PATH}/$${PRODUCT_DIRECTORY}/lib
  VPVL1_LIBRARY_PATH = $${VPVL1_PATH}/$${PRODUCT_DIRECTORY}/lib
  ZLIB_LIBRARY_PATH = $${ZLIB_PATH}/$${BASE_LIBRARY_PATH}
  CONFIG(debug, debug|release):LIBS += -L$${NVTT_PATH}/$${PRODUCT_DIRECTORY}/lib
  CONFIG(release, debug|release):LIBS += -L$${NVTT_PATH}/$${PRODUCT_DIRECTORY}/lib/static
}

# VPVL and others configuration
LIBS        += -L$${VPVL1_LIBRARY_PATH} \
               -L$${VPVL2_LIBRARY_PATH} \
               -L$${ASSIMP_LIBRARY_PATH} \
               -L$${BULLET_LIBRARY_PATH} \
               -L$${ICU_LIBRARY_PATH} \
               -L$${GLEW_LIBRARY_PATH} \
               -L$${ZLIB_LIBRARY_PATH} \
               -L$${LIBXML2_LIBRARY_PATH}
INCLUDEPATH += $${ALSOFT_PATH}/$${PRODUCT_DIRECTORY}/include \
               $${ALURE_PATH}/$${PRODUCT_DIRECTORY}/include \
               $${ASSIMP_PATH}/$${PRODUCT_DIRECTORY}/include/assimp \
               $${BULLET_PATH}/$${PRODUCT_DIRECTORY}/include/bullet \
               $${NVTT_PATH}/$${BUILD_DIRECTORY}/src \
               $${NVTT_PATH}/extern/poshlib \
               $${NVTT_PATH}/src \
               $${GLEW_PATH}/include \
               $${GLM_PATH} \
               $${LIBXML2_PATH}/$${PRODUCT_DIRECTORY}/include/libxml2 \
               $${ZLIB_PATH} \
               $${ZLIB_PATH}/$${BUILD_DIRECTORY} \
               $${LIBAV_PATH}/$${PRODUCT_DIRECTORY}/include \
               $${VPVL2_INCLUDE_PATH}

# Required libraries
LIBS += -lvpvl2qtcommon \
        -lvpvl2 \
        -lvpvl \
        -lassimp \
        -lBulletSoftBody \
        -lBulletDynamics \
        -lBulletCollision \
        -lLinearMath \
        -lnvimage \
        -lnvmath \
        -lnvcore \
        -ltbb

CONFIG(debug, debug|release):LIBS += -ltbb_debug -ltbbmalloc_debug
CONFIG(release, debug|release):LIBS += -ltbb -ltbbmalloc

win32 {
  INCLUDEPATH += $${ICU_PATH}/include
  CONFIG(debug, debug|release):LIBS += -llibxml2_a -lglew32sd -lglew32mxsd -licuind -licuucd -lzlibstaticd
  CONFIG(release, debug|release):LIBS += -llibxml2_a -lglew32s -lglew32mxs -licuin -licuuc -lzlibstatic
  LIBS += -licudt
}
!win32 {
  INCLUDEPATH += $${ICU_PATH}/source/common \
                 $${ICU_PATH}/source/i18n
  LIBS +=  -lxml2 \
           -lGLEW \
           -licui18n \
           -licuuc \
           -licudata \
           -lz
}

macx:LIBS += -framework OpenCL \
             -framework CoreServices \
             -framework Cg \
             -framework CoreAudio \
             -framework AudioToolbox \
             -framework AudioUnit
linux-*:LIBS += -lCg -lCgGL

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
  NVIDIA_CG_PATH  = "C:/Program Files (x86)/NVIDIA Corporation/Cg"
  LIBS           += -L$${NVIDIA_CG_PATH}/lib -lcg -lcggl
  INCLUDEPATH    += $${NVIDIA_CG_PATH}/include
  QMAKE_CFLAGS   += /wd4250 /wd4251 /wd4819
  QMAKE_CXXFLAGS += /wd4250 /wd4251 /wd4819
  RC_FILE         = resources/icons/app.rc
}
macx {
  ICON = resources/icons/app.icns
  QMAKE_CXXFLAGS *= -mmacosx-version-min=10.6
  QMAKE_LFLAGS *= -mmacosx-version-min=10.6
  QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../
  QMAKE_INFO_PLIST = resources/Info.plist
  translations.path = Contents/Resources
  QMAKE_BUNDLE_DATA += translations
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
    common/Handles.cc \
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
    dialogs/FrameSelectionDialog.cc \
    dialogs/PhysicsSettingDialog.cc \
    dialogs/RenderOrderDialog.cc \
    dialogs/ShadowMapSettingDialog.cc \
    dialogs/BackgroundImageSettingDialog.cc \
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
    widgets/ModelInfoWidget.cc \
    widgets/SceneLightWidget.cc \
    widgets/ModelSettingWidget.cc \
    MainWindow.cc \
    BoneUIDelegate.cc \
    ScenePlayer.cc \
    video/AVFactory.cc \
    common/Grid.cc \
    common/BackgroundImage.cc

HEADERS  += \
    common/SceneWidget.h \
    common/Handles.h \
    common/SceneLoader.h \
    common/Grid.h \
    common/Application.h \
    common/VPDFile.h \
    common/InfoPanel.h \
    common/LoggerWidget.h \
    common/StringHelper.h \
    common/BackgroundImage.h \
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
    dialogs/FrameSelectionDialog.h \
    dialogs/PhysicsSettingDialog.h \
    dialogs/RenderOrderDialog.h \
    dialogs/ShadowMapSettingDialog.h \
    dialogs/BackgroundImageSettingDialog.h \
    widgets/MorphWidget.h \
    widgets/CameraPerspectiveWidget.h \
    widgets/TabWidget.h \
    widgets/TimelineTabWidget.h \
    widgets/TimelineTreeView.h \
    widgets/LicenseWidget.h \
    widgets/AssetWidget.h \
    widgets/ModelTabWidget.h \
    widgets/InterpolationGraphWidget.h \
    widgets/ModelInfoWidget.h \
    widgets/SceneLightWidget.h \
    widgets/ModelSettingWidget.h \
    widgets/TimelineWidget.h \
    video/IAudioDecoder.h \
    video/IVideoEncoder.h \
    MainWindow.h \
    BoneUIDelegate.h \
    ScenePlayer.h \
    video/AVFactory.h

!win32 {
    SOURCES += video/AudioDecoder.cc \
        video/AVCommon.cc \
        video/VideoEncoder.cc
    HEADERS += video/AudioDecoder.h \
        video/AVCommon.h \
        video/VideoEncoder.h
    DEFINES += VPVM_ENABLE_VIDEO
    LIBS += -L$${ALSOFT_PATH}/$${PRODUCT_DIRECTORY}/lib \
            -L$${ALURE_PATH}/$${PRODUCT_DIRECTORY}/lib \
            -L$${LIBAV_PATH}/$${PRODUCT_DIRECTORY}/lib \
            -lopenal \
            -lalure-static \
            -lavcodec \
            -lavformat \
            -lavutil \
            -lswscale
}

CODECFORTR = UTF-8
RESOURCES += resources/VPVM.qrc
TRANSLATIONS += resources/translations/VPVM.ts

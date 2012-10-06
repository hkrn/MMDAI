# libvpvl2.pro for MSVC

QT += core gui opengl concurrent
TARGET = libvpvl2qtcommon
TEMPLATE = lib
DEFINES += QT_NO_CAST_TO_ASCII \
           VPVL2_LINK_QT \
           VPVL2_OPENGL_RENDERER \
           _CRT_SECURE_NO_WARNINGS

ASSIMP_PATH = ../../assimp-src
BULLET_PATH = ../../bullet-src
VPVL_PATH = ../../libvpvl
VPVL2_PATH = ../

exists($$(CMAKE_PREFIX_PATH)/include):INCLUDEPATH += "$$(CMAKE_PREFIX_PATH)/include"
exists($$(CMAKE_PREFIX_PATH)/lib):LIBS += -L"$$(CMAKE_PREFIX_PATH)/lib"

INCLUDEPATH +=  $${ASSIMP_PATH}/include \
                $${BULLET_PATH}/src \
                $${VPVL_PATH}/include \
                $${VPVL2_PATH}/include \
                $${VPVL2_PATH}/include/vpvl2/qt

INCLUDEPATH += $${VPVL_PATH}/msvc-build/include \
                     $${VPVL2_PATH}/msvc-build/include

CONFIG(debug, debug|release) {
  LIBS += -L$${ASSIMP_PATH}/msvc-build/code/debug \
                -L$${BULLET_PATH}/msvc-build/lib/debug \
                -L$${VPVL_PATH}/msvc-build/lib/debug \
                -L$${VPVL2_PATH}/msvc-build/lib/debug
}
CONFIG(release, debug|release) {
  LIBS += -L$${ASSIMP_PATH}/msvc-build/code/release \
                -L$${BULLET_PATH}/msvc-build/lib/release \
                -L$${VPVL_PATH}/msvc-build/lib/release \
                -L$${VPVL2_PATH}/msvc-build/lib/release
}

LIBS += -lassimp \
        -lBulletSoftBody \
        -lBulletDynamics \
        -lBulletCollision \
        -lLinearMath \
        -lvpvl \
        -lvpvl2 \
        -lz

SOURCES += src/engine/gl2/AssetRenderEngine.cc \
           src/engine/gl2/PMDRenderEngine.cc \
           src/engine/gl2/PMXRenderEngine.cc \
           src/qt/common/Archive.cc \
           src/qt/common/CString.cc \
           src/qt/common/Delegate.cc \
           src/qt/common/Encoding.cc \
           src/qt/common/World.cc \
           src/qt/unzip/ioapi.c \
           src/qt/unzip/unzip.c

HEADERS += include/vpvl2/gl2/AssetRenderEngine.h \
           include/vpvl2/gl2/PMDRenderEngine.h \
           include/vpvl2/gl2/PMXRenderEngine.h \
           include/vpvl2/qt/Archive.h \
           include/vpvl2/qt/CString.h \
           include/vpvl2/qt/Delegate.h \
           include/vpvl2/qt/Encoding.h \
           include/vpvl2/qt/World.h \
           include/vpvl2/qt/ioapi.h \
           include/vpvl2/qt/unzip.h

QMAKE_CXXFLAGS += /wd4100 /wd4819

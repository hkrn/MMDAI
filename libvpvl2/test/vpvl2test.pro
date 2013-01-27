QT += core gui opengl

TARGET = vpvl2test
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
macx:DEFINES += USE_FILE32API

QMAKE_CXXFLAGS = -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 -Wwrite-strings

LIBS += -L../../libvpvl/build-debug/lib -L../build-debug/lib -lvpvl2_debug -L../../bullet-src/build-debug/lib \
         -lvpvl2qtcommon_debug -lvpvl_debug -licuuc -licui18n -lGLEW \
         -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lz
macx:LIBS += -framework Cg
linux-*:LIBS += -lCg -lCgGL

INCLUDEPATH += ../test/gtest-1.6.0 ../test/gmock-1.6.0 \
               ../test/gtest-1.6.0/include ../test/gmock-1.6.0/include \
               ../include ../build-debug/include ../../libvpvl/include ../../libvpvl/build-debug/include \
               ../../bullet-src/src ../../assimp-src/include ../include/vpvl2/extensions/minizip

linux-* {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/lib
  VPVL2_TEST_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${VPVL2_TEST_RPATH}\'
}

HEADERS += mock/Bone.h \
    mock/BoneKeyframe.h \
    mock/Camera.h \
    mock/CameraKeyframe.h \
    mock/Effect.h \
    mock/Label.h \
    mock/Light.h \
    mock/LightKeyframe.h \
    mock/Model.h \
    mock/Morph.h \
    mock/MorphKeyframe.h \
    mock/Motion.h \
    mock/RenderContext.h \
    Common.h

SOURCES += main.cc \
    InternalTest.cc \
    ModelTest.cc \
    VMDMotionTest.cc \
    MVDMotionTest.cc \
    ProjectTest.cc \
    StringTest.cc \
    EncodingTest.cc \
    ArchiveTest.cc \
    EffectTest.cc \
    FactoryTest.cc \
    SceneTest.cc \
    Common.cc \
    ../src/minizip/ioapi.c \
    ../src/minizip/unzip.c \
    gmock-1.6.0/src/gmock-all.cc \
    gtest-1.6.0/src/gtest-all.cc

RESOURCES += \
    fixtures.qrc

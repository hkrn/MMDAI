QT += core gui opengl

TARGET = vpvl2test
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
macx:DEFINES += USE_FILE32API

QMAKE_CXXFLAGS = -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 -Wwrite-strings

ASSIMP_PATH = ../../assimp-src
BULLET_PATH = ../../bullet-src
ICU_PATH = ../../icu-src
VPVL1_PATH = ../../libvpvl
VPVL2_PATH = ..

LIBS += -L$${VPVL1_PATH}/build-debug/lib \
        -L$${VPVL2_PATH}/build-debug/lib \
        -L$${BULLET_PATH}/build-debug/lib \
        -L$${ASSIMP_PATH}/build-debug/lib \
        -L$${ICU_PATH}/build-debug/lib \
        -lvpvl2qtcommon_debug -lvpvl2_debug -lvpvl_debug -licuuc -licui18n -licudata -lGLEW \
         -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lassimp -lz
macx:LIBS += -framework Cg
linux-*:LIBS += -lCg -lCgGL

INCLUDEPATH += gtest-1.6.0 gmock-1.6.0 \
               gtest-1.6.0/include gmock-1.6.0/include \
               $${VPVL2_PATH}/include \
               $${VPVL2_PATH}/build-debug/include \
               $${VPVL1_PATH}/include \
               $${VPVL1_PATH}/build-debug/include \
               $${BULLET_PATH}/src \
               $${ASSIMP_PATH}/include \
               $${ICU_PATH}/source/common \
               $${ICU_PATH}/icu-src/source/i18n

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
    gmock-1.6.0/src/gmock-all.cc \
    gtest-1.6.0/src/gtest-all.cc

RESOURCES += \
    fixtures.qrc

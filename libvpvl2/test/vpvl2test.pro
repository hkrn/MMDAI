QT += core gui opengl

TARGET = vpvl2test
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

QMAKE_CXXFLAGS = -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=2 -Wwrite-strings

LIBS += -L../../libvpvl/debug/lib -lvpvl_debug \
        -L../debug/lib -lvpvl2_debug -lvpvl2qtcommon_debug -licuuc -licui18n \
        -L../../bullet-src/debug/lib -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath
macx:LIBS += -framework Cg
linux-*:LIBS += -lCg -lCgGL

INCLUDEPATH += ../test/gtest-1.6.0 ../test/gmock-1.6.0 \
               ../test/gtest-1.6.0/include ../test/gmock-1.6.0/include \
               ../include ../debug/include ../../libvpvl/include ../../libvpvl/debug/include \
               ../../bullet-src/src ../../assimp-src/include

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

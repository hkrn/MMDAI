QT += core
QT -= gui

TARGET = test
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

LIBS += -L../test/gtest-1.6.0/debug -lgtest -lgtest_main \
        -L../test/gmock-1.6.0/debug -lgmock -lgmock_main \
        -L../debug/lib -lvpvl2_debug -L../../bullet/debug/lib \
        -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath
INCLUDEPATH += ../test/gtest-1.6.0/include ../test/gmock-1.6.0/include \
               ../include ../debug/include \
               ../../bullet/src /opt/local/include/libxml2 /usr/include/libxml2

linux-* {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/lib
  VPVL2_TEST_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${VPVL2_TEST_RPATH}\'
}

HEADERS += Common.h \
    mock/Bone.h \
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

SOURCES += main.cc \
    Internal.cc \
    Model.cc \
    Motion.cc \
    Project.cc

QT += core
QT -= gui

TARGET = test
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

LIBS += -L/Users/hkrn/src/gtest-1.6.0/release -lgtest -lgtest_main \
        -L/Users/hkrn/src/gmock-1.6.0/release -lgmock -lgmock_main \
        -L../debug/lib -lvpvl2_debug -L../../bullet/debug/lib \
        -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath
INCLUDEPATH += /Users/hkrn/src/gtest-1.6.0/include /Users/hkrn/src/gmock-1.6.0/include \
               ../include ../debug/include \
               ../../bullet/src /opt/local/include/libxml2 /usr/include/libxml2

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

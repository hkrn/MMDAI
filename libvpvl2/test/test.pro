QT += core
QT -= gui

TARGET = test
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

LIBS += -L/Users/hkrn/src/gtest-1.6.0/release -lgtest -lgtest_main \
        -L../debug/lib -lvpvl2_debug -L../../bullet/debug/lib \
        -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath
INCLUDEPATH += /Users/hkrn/src/gtest-1.6.0/include ../include ../debug/include \
               ../../bullet/src /opt/local/include/libxml2 /usr/include/libxml2

HEADERS += Common.h
SOURCES += main.cc \
    Internal.cc \
    Model.cc \
    Motion.cc \
    Project.cc

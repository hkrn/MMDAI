QT += testlib
QT -= gui
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
LIBS += -L../../debug/lib -lvpvl_debug -L../../../bullet/debug/lib \
        -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lGLEW
INCLUDEPATH += ../../include ../../../bullet/src

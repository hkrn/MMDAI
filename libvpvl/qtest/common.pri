QT += testlib
QT -= gui
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
LIBS += -L../../../lib -lvpvl_debug -L/usr/local/lib -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lGLEW
INCLUDEPATH += ../../../include /usr/local/include/bullet

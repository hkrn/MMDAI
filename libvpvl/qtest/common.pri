QT += testlib
QT -= gui
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
LIBS += -L../../dist/lib -lvpvl_debug -L/opt/local/lib -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lGLEW
INCLUDEPATH += ../../include /opt/local/include/bullet

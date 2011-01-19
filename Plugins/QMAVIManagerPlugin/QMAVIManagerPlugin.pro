TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../..
TARGET = $$qtLibraryTarget(QMAVIManagerPlugin)
DESTDIR = ../plugins

unix {
    LIBS += -L/usr/local/lib -lMMDAI -lMMDFiles -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
    INCLUDEPATH += /usr/include/bullet /usr/local/include/bullet
}

HEADERS += \
    QMAVIManagerPlugin.h \
    VIManager.h \
    VIManager_Thread.h

SOURCES += \
    QMAVIManagerPlugin.cc \
    VIManager.cpp \
    VIManager_Thread.cpp

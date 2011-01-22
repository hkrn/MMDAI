TEMPLATE = lib
CONFIG += plugin
QT += phonon
INCLUDEPATH += ../..
TARGET = $$qtLibraryTarget(QMAAudioPlugin)
DESTDIR = ../plugins

unix {
    LIBS += -L/usr/local/lib -lMMDAI -lMMDFiles -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
    INCLUDEPATH += /usr/include/bullet /usr/local/include/bullet
}

HEADERS += \
    QMAAudioPlugin.h

SOURCES += \
    QMAAudioPlugin.cc

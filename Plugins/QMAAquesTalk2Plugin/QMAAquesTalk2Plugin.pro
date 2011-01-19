TEMPLATE = lib
CONFIG += plugin
QT += phonon
INCLUDEPATH += ../..
TARGET = $$qtLibraryTarget(QMAAquesTalk2Plugin)
DESTDIR = ../plugins

unix {
    LIBS += -L/usr/local/lib -lMMDAI -lMMDFiles -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
    INCLUDEPATH += /usr/include/bullet /usr/local/include/bullet /usr/local/include
}
macx {
    LIBS += -framework AquesTalk2
}

HEADERS += \
    QMAAquesTalk2Plugin.h

SOURCES += \
    QMAAquesTalk2Plugin.cc

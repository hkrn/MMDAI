TEMPLATE = lib
CONFIG += plugin
QT += phonon
INCLUDEPATH += ../..
TARGET = $$qtLibraryTarget(QMAAquesTalk2Plugin)
DESTDIR = ../plugins

unix {
    LIBS += -L/usr/local/lib -lMMDAI -lMMDFiles -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
    INCLUDEPATH += /usr/local/include/bullet
}
macx {
    LIBS += -framework AquesTalk2
}

HEADERS += \
    QMAAquesTalk2Plugin.h

SOURCES += \
    QMAAquesTalk2Plugin.cc

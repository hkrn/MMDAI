TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../..
TARGET = $$qtLibraryTarget(QMAOpenJTalkPlugin)
DESTDIR = ../plugins

unix {
    LIBS += -L/usr/local/lib -lMMDAI -lMMDFiles -lGLee -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
    INCLUDEPATH += /usr/local/include/MMDAI /usr/local/include/MMDFiles /usr/local/include/bullet
}

HEADERS += \
    QMAOpenJTalkPlugin.h

SOURCES += \
    QMAOpenJTalkPlugin.cc

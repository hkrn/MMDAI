TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../..
TARGET = $$qtLibraryTarget(QMAJuliusPlugin)
DESTDIR = ../plugins

unix {
    LIBS += -L/usr/local/lib -lMMDAI -lMMDFiles -ljulius -lsent -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath -lz
    INCLUDEPATH += /usr/local/include/MMDAI /usr/local/include/MMDFiles /usr/local/include/bullet
}
macx {
    LIBS += -framework AudioToolbox -framework AudioUnit -framework CoreAudio -framework CoreServices
}

HEADERS += \
    QMAJuliusPlugin.h \
    Julius_Thread.h \
    CommandDispatcher.h \
    QMAJuliusInitializer.h

SOURCES += \
    QMAJuliusPlugin.cc \
    Julius_Thread.cpp \
    QMAJuliusInitializer.cc

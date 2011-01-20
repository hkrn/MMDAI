TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../..
TARGET = $$qtLibraryTarget(QMAJuliusPlugin)
DESTDIR = ../plugins

unix {
    INCLUDEPATH += /usr/include/bullet /usr/local/include/bullet /usr/local/include
    LIBS += -L/usr/local/lib -lMMDAI -lMMDFiles -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
    LIBS += $$system(libsent-config --libs) $$system(libjulius-config --libs)
    QMAKE_CXXFLAGS += $$system(libsent-config --cflags) $$system(libjulius-config --libs)
}

HEADERS += \
    QMAJuliusPlugin.h \
    Julius_Thread.h \
    QMAJuliusInitializer.h

SOURCES += \
    QMAJuliusPlugin.cc \
    Julius_Thread.cpp \
    QMAJuliusInitializer.cc

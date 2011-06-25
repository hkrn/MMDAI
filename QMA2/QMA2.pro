QT += core gui opengl
TARGET = MMDAI
TEMPLATE = app
CONFIG += x86

LIBS += -L../libvpvl -lvpvl -L/usr/local/lib -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath
INCLUDEPATH += ../libvpvl/include /usr/local/include/bullet

SOURCES += main.cc\
        MainWindow.cc \
    SceneWidget.cc

HEADERS  += MainWindow.h \
    SceneWidget.h

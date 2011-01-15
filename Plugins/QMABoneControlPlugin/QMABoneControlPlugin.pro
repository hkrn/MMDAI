TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../..
TARGET = $$qtLibraryTarget(QMABoneControlPlugin)
DESTDIR = ../plugins

unix {
    LIBS += -L/usr/local/lib -lMMDAI -lMMDFiles -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
    INCLUDEPATH += /usr/local/include/MMDAI /usr/local/include/MMDFiles /usr/local/include/bullet
}

HEADERS += \
    BoneController.h \
    QMABoneControlPlugin.h

SOURCES += \
    QMABoneControlPlugin.cc

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../
DESTDIR = ../plugins

unix:LIBS += -L/usr/lib -L/usr/local/lib
unix:INCLUDEPATH += /usr/include /usr/local/include /usr/include/bullet /usr/local/include/bullet
win32:LIBS += -L$$(MMDME_LIBRARY_DIR) -L$$(MMDAI_LIBRARY_DIR) -L$$(BULLET_LIBRARY_DIR)
win32:INCLUDEPATH += $$(MMDME_INCLUDE_DIR) $$(MMDAI_INCLUDE_DIR) $$(BULLET_INCLUDE_DIR)
LIBS += -lMMDAI -lMMDME -lglee -lBulletDynamics -lBulletCollision -lBulletSoftBody -lLinearMath

CONFIG(debug, debug|release) {
    macx:CONFIG += x86
}


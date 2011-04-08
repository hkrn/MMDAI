unix:LIBS += -L/usr/lib -L/usr/local/lib
unix:INCLUDEPATH += /usr/include /usr/local/include /usr/include/bullet /usr/local/include/bullet
win32:LIBS += -L$$(MMDME_LIBRARY_DIR) -L$$(MMDAI_LIBRARY_DIR) -L$$(BULLET_LIBRARY_DIR) -L$$(GLEE_DIR)
win32:INCLUDEPATH += $$(MMDME_INCLUDE_DIR) $$(MMDAI_INCLUDE_DIR) $$(BULLET_INCLUDE_DIR) $$(GLEE_DIR)
win32:DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE _SCL_SECURE_NO_WARNINGS
LIBS += -lMMDAI -lMMDME -lglee -lBulletDynamics -lBulletCollision -lBulletSoftBody -lLinearMath

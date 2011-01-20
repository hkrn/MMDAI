TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../..
TARGET = $$qtLibraryTarget(QMAOpenJTalkPlugin)
DESTDIR = ../plugins

unix {
    # $ cd /usr/local/include
    # $ sudo mkdir jtalk
    # $ cd jtalk
    # $ sudo find $OPEN_JTALK_SRC_DIR -name '*.h' -exec ln -s {} \;
    # $ cd /usr/local/lib
    # $ sudo find $OPEN_JTALK_SRC_DIR -name '*.a' -exec ln -s {} \;
    LIBS += -L/usr/local/lib -lMMDAI -lMMDFiles -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath \
            -lHTSEngine -ljpcommon -lmecab2njd -lnjd -lnjd2jpcommon -lnjd_set_accent_phrase -lnjd_set_accent_type \
            -lnjd_set_digit -lnjd_set_long_vowel -lnjd_set_pronunciation -lnjd_set_unvoiced_vowel -ltext2mecab
    INCLUDEPATH += /usr/include/bullet /usr/local/include/bullet /usr/local/include/jtalk
}

linux-g++ {
    LIBS += -lmecab
}

macx {
    # $ cd /usr/local/include
    # $ sudo mkdir jtalk
    # $ cd jtalk
    # $ sudo gfind $OPEN_JTALK_SRC_DIR -name '*.h' -exec ln -s {} \;
    # $ cd /usr/local/lib
    # $ sudo gfind $OPEN_JTALK_SRC_DIR -name '*.a' -exec ln -s {} \;
    #
    # on MacOSX, mecab has been installed in /usr, we use jtalk's mecab as libmecab_custom.a
    LIBS += -liconv -lmecab_custom
}

HEADERS += \
    QMAOpenJTalkPlugin.h \
    Open_JTalk.h \
    Open_JTalk_Thread.h

SOURCES += \
    QMAOpenJTalkPlugin.cc \
    Open_JTalk.cpp \
    Open_JTalk_Thread.cpp

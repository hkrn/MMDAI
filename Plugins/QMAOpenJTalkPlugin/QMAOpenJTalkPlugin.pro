TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../..
TARGET = $$qtLibraryTarget(QMAOpenJTalkPlugin)
DESTDIR = ../plugins

unix {
    # cd /usr/local/lib
    # sudo gfind ~/src/open_jtalk-1.02/ -name '*.a' -exec ln -s {} \;
    LIBS += -L/usr/local/lib -liconv -lMMDAI -lMMDFiles -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath \
            -lHTSEngine -ljpcommon -lmecab_custom -lmecab2njd -lnjd -lnjd2jpcommon -lnjd_set_accent_phrase -lnjd_set_accent_type \
            -lnjd_set_digit -lnjd_set_long_vowel -lnjd_set_pronunciation -lnjd_set_unvoiced_vowel -ltext2mecab
    INCLUDEPATH += /usr/local/include/MMDAI /usr/local/include/MMDFiles /usr/local/include/bullet /usr/local/include/jtalk
}

HEADERS += \
    QMAOpenJTalkPlugin.h \
    Open_JTalk.h \
    Open_JTalk_Thread.h

SOURCES += \
    QMAOpenJTalkPlugin.cc \
    Open_JTalk.cpp \
    Open_JTalk_Thread.cpp

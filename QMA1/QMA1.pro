QT += core gui opengl
TARGET = MMDAI
TEMPLATE = app

exists(/opt/local/lib):LIBS += -L/opt/local/lib
exists(/opt/local/include):INCLUDEPATH += /opt/local/include
exists(/usr/local/lib):LIBS += -L/usr/local/lib
exists(/usr/local/include):INCLUDEPATH += /usr/local/include

# GLEW and assimp
exists(../glew/lib):LIBS += -L../glew/lib
exists(../glew/include):INCLUDEPATH += ../glew/include
exists(../assimp/include):INCLUDEPATH += ../assimp/include
exists(../assimp/lib):LIBS += -L../assimp/lib -lassimp

# Basic Configuration
LIBS += -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath -lassimp
win32:LIBS += -lglew32
unix:LIBS += -lGLEW

# VPVL and others configuration
INCLUDEPATH += ../libvpvl/include ../bullet/src
win32:INCLUDEPATH += ../libvpvl/msvc-build/include

# configuration by build type
CONFIG(debug, debug|release) {
  win32:LIBS       += -L../libvpvl/msvc-build/lib/debug -L../bullet/msvc-build/lib/debug -lvpvl
  unix:LIBS        += -L../libvpvl/debug/lib -L../bullet/debug/lib -lvpvl_debug
  unix:INCLUDEPATH += ../libvpvl/debug/include
  exists(../assimp/debug/code):LIBS += -L../assimp/debug/code -lassimp
}
CONFIG(release, debug|release) {
  win32:LIBS       += -L../libvpvl/msvc-build/lib/release -L../bullet/msvc-build/lib/release -lvpvl
  unix:LIBS        += -L../libvpvl/release/lib -L../bullet/release/lib -lvpvl
  unix:INCLUDEPATH += ../libvpvl/release/include
  exists(../assimp/release/code):LIBS += -L../assimp/release/code -lassimp
}
LIBS += -lOpenJTalk -lHTSEngine -ljulius -lportaudio

# based on QtCreator's qmake spec
DEFINES += QT_NO_CAST_TO_ASCII
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
unix {
  CONFIG(debug, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/debug-shared
  CONFIG(release, debug|release):OBJECTS_DIR = $${OUT_PWD}/.obj/release-shared
  CONFIG(debug, debug|release):MOC_DIR = $${OUT_PWD}/.moc/debug-shared
  CONFIG(release, debug|release):MOC_DIR = $${OUT_PWD}/.moc/release-shared
  RCC_DIR = $${OUT_PWD}/.rcc
  UI_DIR = $${OUT_PWD}/.uic
}
macx {
  QMAKE_CXXFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/PlugIns/
  QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../
  QMAKE_INFO_PLIST = resources/Info.plist
  QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
  LIBS += -framework CoreAudio -framework CoreServices -framework AudioToolbox -framework AudioUnit
  QMAKE_INFO_PLIST = res/Info.plist
  resources.files = res/translations/MMDAI1_ja.qm \
                    $$[QT_INSTALL_TRANSLATIONS]/qt_ja.qm
  resources.path = Contents/Resources
  QMAKE_BUNDLE_DATA += resources
  CONFIG(debug, debug|release) {
    CONFIG += x86_64
  }
}
else:linux-* {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/..
  QMA_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${QMA_RPATH}\'
  QMAKE_RPATHDIR =
}

SOURCES += Delegate.cc \
           LicenseWidget.cc \
           SceneLoader.cc \
           SceneWidget.cc \
           main.cc \
           MainWindow.cc \
           Script.cc \
           TiledStage.cc \
           LipSync.cc \
           OpenJTalkSpeechEngine.cc \
           JuliusSpeechRecognitionEngine.cc \
           LoggerWidget.cc

HEADERS  += Delegate.h \
            World.h \
            LicenseWidget.h \
            SceneLoader.h \
            SceneWidget.h \
            util.h \
            MainWindow.h \
            Script.h \
            TiledStage.h \
            LipSync.h \
            OpenJTalkSpeechEngine.h \
            JuliusSpeechRecognitionEngine.h \
            LoggerWidget.h \
    Application.h

CODECFORTR = UTF-8
RESOURCES += ../QMA2/resources/QMA2.qrc \
             res/QMA1.qrc
TRANSLATIONS += res/translations/MMDAI1.ts

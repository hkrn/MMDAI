QT += core gui opengl
TARGET = MMDAI
TEMPLATE = app
DEFINES += QMA_ENABLE_MULTIPLE_MOTION

exists(/opt/local/lib):LIBS += -L/opt/local/lib
exists(/opt/local/include):INCLUDEPATH += /opt/local/include
exists(/usr/local/lib):LIBS += -L/usr/local/lib
exists(/usr/local/include):INCLUDEPATH += /usr/local/include
exists(/opt/local/include/libxml2):INCLUDEPATH += /opt/local/include/libxml2
exists(/usr/include/libxml2):INCLUDEPATH += /usr/include/libxml2
exists(/usr/local/include/libxml2):INCLUDEPATH += /usr/local/include/libxml2

# GLEW and assimp
exists(../glew/lib):LIBS += -L../glew/lib
exists(../glew/include):INCLUDEPATH += ../glew/include
exists(../assimp/lib):LIBS += -L../assimp/lib -lassimp
exists(../assimp/include):INCLUDEPATH += ../assimp/include

# Basic Configuration
LIBS += -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath
win32:MMDA_PATH = ../../MMDAgent/MMDAgent
win32:LIBS += -L$${MMDA_PATH}/Library_hts_engine_API/lib -L$${MMDA_PATH}/Library_Julius/lib \
      -L$${MMDA_PATH}/Library_Open_JTalk/lib -L$${MMDA_PATH}/Library_PortAudio/lib -lglew32 -ldsound
unix:LIBS += -lGLEW -lOpenJTalk -lHTSEngine -ljulius -lportaudio -lxml2

# VPVL and others configuration
INCLUDEPATH += ../libvpvl/include ../bullet/src $${MMDA_PATH}/Library_Julius/include \
       $${MMDA_PATH}/Library_Open_JTalk/include $${MMDA_PATH}/Library_hts_engine_API/include \
       $${MMDA_PATH}/Library_PortAudio/include
win32:INCLUDEPATH += ../libvpvl/msvc-build/include $${MMDA_PATH}

# configuration by build type
CONFIG(debug, debug|release) {
  win32:LIBS       += -L../libvpvl/msvc-build/lib/debug -L../bullet/msvc-build/lib/debug \
                      -lvpvl -lPortAudio_D -lhts_engine_API_D -lJulius_D -lOpen_JTalk_D -lws2_32
  unix:LIBS        += -L../libvpvl/debug/lib -L../bullet/debug/lib -lvpvl_debug
  unix:INCLUDEPATH += ../libvpvl/debug/include
  exists(../assimp/code/debug):LIBS += -L../assimp/code/debug -lassimp
}
CONFIG(release, debug|release) {
  win32:LIBS       += -L../libvpvl/msvc-build/lib/release -L../bullet/msvc-build/lib/release \
                      -lvpvl -lPortAudio -lhts_engine_API -lJulius -lOpen_JTalk -lws2_32
  unix:LIBS        += -L../libvpvl/release/lib -L../bullet/release/lib -lvpvl
  unix:INCLUDEPATH += ../libvpvl/release/include
  exists(../assimp/code/release):LIBS += -L../assimp/code/release -lassimp
}

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

translations.files = resources/translations/MMDAI1_ja.qm \
                     $$[QT_INSTALL_TRANSLATIONS]/qt_ja.qm
julius.files = resources/AppData/Julius/jconf.txt \
               resources/AppData/Julius/lang_m/web.60k.8-8.bingramv5.gz \
               resources/AppData/Julius/lang_m/web.60k.htkdic \
               resources/AppData/Julius/phone_m/clustered.mmf.16mix.all.julius.binhmm \
               resources/AppData/Julius/phone_m/tri_tied.list.bin
openjtalk.files = resources/AppData/Open_JTalk/char.bin \
                  resources/AppData/Open_JTalk/left-id.def \
                  resources/AppData/Open_JTalk/matrix.bin \
                  resources/AppData/Open_JTalk/pos-id.def \
                  resources/AppData/Open_JTalk/rewrite.def \
                  resources/AppData/Open_JTalk/right-id.def \
                  resources/AppData/Open_JTalk/sys.dic \
                  resources/AppData/Open_JTalk/unk.dic

win32 {
  RC_FILE = ../obsoletes/QMA/res/MMDAI.rc
}
macx {
  QMAKE_CXXFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS_SONAME = -Wl,-install_name,@rpath/PlugIns/
  QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../
  QMAKE_INFO_PLIST = resources/Info.plist
  QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
  LIBS += -framework CoreAudio -framework CoreServices -framework AudioToolbox -framework AudioUnit
  ICON = ../obsoletes/QMA/res/MMDAI.icns
  QMAKE_INFO_PLIST = resources/Info.plist
  translations.path = Contents/Resources
  julius.path = Contents/Resources/Julius
  openjtalk.path = Contents/Resources/OpenJTalk
  QMAKE_BUNDLE_DATA += translations julius openjtalk
  CONFIG(debug, debug|release) {
    CONFIG += x86_64
  }
}
linux-* {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/..
  QMA_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${QMA_RPATH}\'
  QMAKE_RPATHDIR =
}
!macx {
  translations.path = /locales
  julius.path = /resources/Julius
  openjtalk.path = /resources/OpenJTalk
  INSTALLS += translations julius openjtalk
}

SOURCES += main.cc \
    ../QMA2/common/SceneWidget.cc \
    ../QMA2/common/VPDFile.cc \
    ../QMA2/common/SceneLoader.cc \
    ../QMA2/common/LoggerWidget.cc \
    ../QMA2/common/Handles.cc \
    LicenseWidget.cc \
    MainWindow.cc \
    Script.cc \
    TiledStage.cc \
    LipSync.cc \
    OpenJTalkSpeechEngine.cc \
    JuliusSpeechRecognitionEngine.cc \
    ExtendedSceneWidget.cc \

OBJECTIVE_SOURCES += Transparent.mm
macx:LIBS += -framework AppKit

INCLUDEPATH += ../QMA2/common
HEADERS  += \
    ../QMA2/common/SceneWidget.h \
    ../QMA2/common/Handles.h \
    ../QMA2/common/util.h \
    ../QMA2/common/Delegate.h \
    ../QMA2/common/World.h \
    ../QMA2/common/SceneLoader.h \
    ../QMA2/common/Grid.h \
    ../QMA2/common/Application.h \
    ../QMA2/common/VPDFile.h \
    ../QMA2/common/InfoPanel.h \
    ../QMA2/common/DebugDrawer.h \
    ../QMA2/common/LoggerWidget.h \
    LicenseWidget.h \
    MainWindow.h \
    Script.h \
    TiledStage.h \
    LipSync.h \
    OpenJTalkSpeechEngine.h \
    JuliusSpeechRecognitionEngine.h \
    ExtendedSceneWidget.h \

CODECFORTR = UTF-8
RESOURCES += ../QMA2/resources/QMA2.qrc \
             resources/QMA1.qrc
TRANSLATIONS += resources/translations/MMDAI1.ts

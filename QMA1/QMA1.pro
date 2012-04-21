QT += core gui opengl
TARGET = MMDAI
TEMPLATE = app

# CMake prefix path (mainly for win32)
exists($$(CMAKE_PREFIX_PATH)/include):INCLUDEPATH += $$(CMAKE_PREFIX_PATH)/include
exists(-L$$(CMAKE_PREFIX_PATH)/lib):LIBS += -L$$(CMAKE_PREFIX_PATH)/lib

# Linux, Darwin(OSX), etc...
exists(/usr/local/lib):LIBS += -L/usr/local/lib
exists(/usr/local/include):INCLUDEPATH += /usr/local/include

# libxml2
exists(/usr/include/libxml2):INCLUDEPATH += /usr/include/libxml2
exists(/usr/local/include/libxml2):INCLUDEPATH += /usr/local/include/libxml2

# libvpvl and base libraries (MMDAgent for win32)
ASSIMP_PATH = ../assimp
BULLET_PATH = ../bullet
VPVL_PATH = ../libvpvl
VPVL2_PATH = ../libvpvl2
MMDA_PATH = ../../MMDAgent/MMDAgent

# Required libraries
LIBS += -L$${ASSIMP_PATH}/lib \
        -lassimp \
        -lBulletCollision \
        -lBulletDynamics \
        -lBulletSoftBody \
        -lLinearMath \

unix:LIBS += -lOpenJTalk \
             -lHTSEngine \
             -ljulius \
             -lportaudio \
             -lxml2

win32:LIBS += -L$${MMDA_PATH}/Library_hts_engine_API/lib \
      -L$${MMDA_PATH}/Library_Julius/lib \
      -L$${MMDA_PATH}/Library_Open_JTalk/lib \
      -L$${MMDA_PATH}/Library_PortAudio/lib \
      -lglew32 \-ldsound

# VPVL and others configuration
INCLUDEPATH +=  $${VPVL_PATH}/include \
                $${VPVL2_PATH}/include \
                $${ASSIMP_PATH}/include \
                $${BULLET_PATH}/src

win32:INCLUDEPATH += $${VPVL2_PATH}/msvc-build/include \
                     $${MMDA_PATH} \
                     $${MMDA_PATH}/Library_Julius/include \
                     $${MMDA_PATH}/Library_Open_JTalk/include \
                     $${MMDA_PATH}/Library_hts_engine_API/include \
                     $${MMDA_PATH}/Library_PortAudio/include

# configuration by build type
CONFIG(debug, debug|release) {
  win32:LIBS       += -L$${VPVL2_PATH}/msvc-build/lib/debug \
                      -L$${BULLET_PATH}/msvc-build/lib/debug \
                      -lPortAudio_D \
                      -lhts_engine_API_D \
                      -lJulius_D \
                      -lOpen_JTalk_D \
                      -lws2_32
  macx:LIBS        += -framework OpenCL
  unix:LIBS        += -L$${BULLET_PATH}/debug/lib \
                      -L$${VPVL_PATH}/debug/lib \
                      -L$${VPVL2_PATH}/debug/lib
  unix:INCLUDEPATH += $${VPVL_PATH}/debug/include \
                      $${VPVL2_PATH}/debug/include
  LIBS             += -lvpvl_debug -lvpvl2_debug
  exists($${ASSIMP_PATH}/debug):LIBS += -L$${ASSIMP_PATH}/code/debug -lassimp
}
CONFIG(release, debug|release) {
  win32:LIBS       += -L$${VPVL2_PATH}/msvc-build/lib/release \
                      -L$${BULLET_PATH}/msvc-build/lib/release \
                      -lPortAudio \
                      -lhts_engine_API \
                      -lJulius \
                      -lOpen_JTalk \
                      -lws2_32 \
                      -lvpvl \
                      -lvpvl2
  macx:LIBS        += -framework OpenCL
  unix:LIBS        += -L$${BULLET_PATH}/release/lib \
                      -L$${VPVL_PATH}/release/lib \
                      -L$${VPVL2_PATH}/release/lib
  LIBS             += -lvpvl -lvpvl2
  unix:INCLUDEPATH += $${VPVL_PATH}/release/include \
                      $${VPVL2_PATH}release/include
  exists($${ASSIMP_PATH}/release):LIBS += -L$${ASSIMP_PATH}/release -lassimp
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
  DEFINES += USE_FILE32API
  CONFIG(debug, debug|release) {
    CONFIG += x86_64
  }
}
linux-* {
  QMAKE_RPATHDIR += \$\$ORIGIN
  QMAKE_RPATHDIR += \$\$ORIGIN/lib
  QMA_RPATH = $$join(QMAKE_RPATHDIR, ":")
  QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF -Wl,-z,origin \'-Wl,-rpath,$${QMA_RPATH}\'
  QMAKE_RPATHDIR =
  LIBS += -lGLU
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
    ../QMA2/common/Archive.cc \
    ../QMA2/unzip/ioapi.c \
    ../QMA2/unzip/unzip.c \
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

INCLUDEPATH += ../QMA2/common ../QMA2
HEADERS  += \
    ../QMA2/common/SceneWidget.h \
    ../QMA2/common/Handles.h \
    ../QMA2/common/util.h \
    ../QMA2/common/World.h \
    ../QMA2/common/SceneLoader.h \
    ../QMA2/common/Grid.h \
    ../QMA2/common/Application.h \
    ../QMA2/common/VPDFile.h \
    ../QMA2/common/InfoPanel.h \
    ../QMA2/common/DebugDrawer.h \
    ../QMA2/common/LoggerWidget.h \
    ../QMA2/common/Archive.h \
    ../QMA2/unzip/ioapi.h \
    ../QMA2/unzip/unzip.h \
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

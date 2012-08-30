TEMPLATE = lib
LIBS += -framework QTKit -framework Foundation

OBJECTIVE_SOURCES += QTKitPlugin.mm
HEADERS += QTKitPlugin.h

macx {
  QMAKE_CXXFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS *= -mmacosx-version-min=10.5
  QMAKE_LFLAGS += -Wl,-rpath,@loader_path/../
  CONFIG(debug, debug|release) {
    TARGET = qtkitplugin_debug
    CONFIG += x86_64
  }
  CONFIG(release, debug|release) {
    TARGET = qtkitplugin
    CONFIG += x86 x86_64
  }
}

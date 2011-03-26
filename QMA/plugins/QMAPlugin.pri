include(../QMACommon.pri)

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../
DESTDIR = ../Plugins

# currently same as debug except building plugins
CONFIG(release, debug|release) {
    macx {
        CONFIG += x86 static
        DESTDIR = ../StaticPlugins
    }
}


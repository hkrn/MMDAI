include(../QMACommon.pri)

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../

# currently same as debug except building plugins
CONFIG(debug, debug|release) {
    DESTDIR = ../../debug/Plugins
    macx:DESTDIR = ../Plugins
}
CONFIG(release, debug|release) {
    DESTDIR = ../../release/Plugins
    macx {
        CONFIG += static
        DESTDIR = ../StaticPlugins
    }
}

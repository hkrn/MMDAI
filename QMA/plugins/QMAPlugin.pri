include(../QMACommon.pri)

TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../

# currently same as debug except building plugins
CONFIG(debug, debug|release) {
    DESTDIR = ../../debug/Plugins
    macx:DESTDIR = ../Plugins
    macx:CONFIG += x86 x86_64
}
CONFIG(release, debug|release) {
    DESTDIR = ../../release/Plugins
    macx {
        CONFIG += static x86 x86_64
        DESTDIR = ../StaticPlugins
    }
}

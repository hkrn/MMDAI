#ifndef COMMON_H
#define COMMON_H

#include <QtCore>
#include <QtGui>
#include <QOpenGLContext>

static inline void prepareRegal()
{
    static const QByteArray kRegalEnableVariables[] = {
        "REGAL_EMULATION",
        0
    };
    static const QByteArray kRegalDisableVariables[] = {
        "REGAL_NO_EMULATION",
        0
    };
    for (int i = 0; !kRegalEnableVariables[i].isNull(); i++) {
        qputenv(kRegalEnableVariables[i], "1");
    }
    for (int i = 0; !kRegalDisableVariables[i].isNull(); i++) {
        qputenv(kRegalEnableVariables[i], "0");
    }
}

#endif // COMMON_H

#ifndef COMMON_H
#define COMMON_H

#include <QtCore>
#include <QtGui>
#include <QApplication>
#include <QOpenGLContext>
#include <QQuickWindow>

static inline void setApplicationDescription(const QString &name, QApplication &application)
{
    application.setApplicationDisplayName(name);
    application.setApplicationName(name);
    application.setApplicationVersion("0.33.4");
    application.setOrganizationName("MMDAI Project");
    application.setOrganizationDomain("mmdai.github.com");
}

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

static inline void displayApplicationWindow(QObject *object, int samples)
{
    QQuickWindow *window = qobject_cast<QQuickWindow *>(object);
    Q_ASSERT(window);
    QSurfaceFormat format = window->format();
    format.setSamples(samples);
#if 0
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
#endif
    window->setFormat(format);
#ifdef Q_OS_MACX
    window->setFlags(window->flags() | Qt::WindowFullscreenButtonHint);
#endif
    window->show();
}

#endif // COMMON_H

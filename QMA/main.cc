/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QLibrary>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QPluginLoader>
#include <QtCore/QTranslator>
#include <QtCore/QtPlugin>
#include <QtGui/QApplication>

#include "MMDME/MMDME.h"
#include "QMALogger.h"
#include "QMAPlugin.h"
#include "QMAWindow.h"

#ifdef QMA_BUNDLE_AQUESTALK2_PLUGIN
Q_IMPORT_PLUGIN(qma_aquestalk2_plugin)
#endif
#ifdef QMA_BUNDLE_PLUGINS
Q_IMPORT_PLUGIN(qma_audio_plugin)
Q_IMPORT_PLUGIN(qma_julius_plugin)
Q_IMPORT_PLUGIN(qma_lookat_plugin)
Q_IMPORT_PLUGIN(qma_openjtalk_plugin)
Q_IMPORT_PLUGIN(qma_vimanager_plugin)
#endif

static void QMASetSearchPath(const QCoreApplication &app)
{
    QStringList paths;
    QDir appDir(app.applicationDirPath()), appBaseDir = appDir;
#if defined(Q_OS_WIN)
    app.addLibraryPath(appDir.absoluteFilePath("Plugins"));
#elif defined(Q_OS_MAC)
    if (appDir.dirName() == "MacOS") {
        appDir.cdUp();
        appDir.cdUp();
        appDir.cdUp();
    }
#endif
    const QString applicationPath(appDir.absolutePath());

    /* set path to find configurations (e.g. MMDAI.fst) */
#ifdef QMA_CONFIG_PATH
    const QString configPath(QMA_CONFIG_PATH);
#else
    const QString configPath(applicationPath);
#endif
    paths.clear();
    paths.append(configPath);
    QDir::setSearchPaths("MMDAIUserData", paths);
    MMDAILogInfo("MMDAIUserData: %s", configPath.toUtf8().constData());

    /* set path to find plugins */
#ifdef QMA_PLUGIN_PATH
    const QString pluginPath(QMA_PLUGIN_PATH);
#else
    const QString pluginPath(applicationPath + "/Plugins");
#endif
    paths.clear();
    paths.append(pluginPath);
    QDir::setSearchPaths("MMDAIPlugins", paths);
    MMDAILogInfo("MMDAIPlugins: %s", pluginPath.toUtf8().constData());

    /* set path to find resources such as model, motion etc. */
#ifdef QMA_RESOURCE_PATH
    const QString resourcePath(QMA_RESOURCE_PATH);
#else
    const QString resourcePath(applicationPath);
#endif
    paths.clear();
    paths.append(resourcePath);
    QDir::setSearchPaths("MMDAIResources", paths);
    MMDAILogInfo("MMDAIResources: %s", resourcePath.toUtf8().constData());

    /* load translation files from Qt's system path and resource path */
#ifdef QMA_TRANSLATION_PATH
    const QString translationPath(QMA_TRANSLATION_PATH);
#else
#ifdef Q_OS_MAC
    const QString translationPath(QDir::cleanPath(appBaseDir.absoluteFilePath("../Resources")));
#else
    const QString translationPath(resourcePath + "/Locales");
#endif
#endif
    paths.clear();
    paths.append(translationPath);
    QDir::setSearchPaths("MMDAITranslations", paths);
    MMDAILogInfo("MMDAITranslations: %s", translationPath.toUtf8().constData());
}

static void QMALoadTranslations(QCoreApplication &app, QTranslator &appTr, QTranslator &qtTr)
{
    const QString locale = QLocale::system().name();
    qtTr.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    appTr.load("QMA_" + locale, QDir("MMDAITranslations:/").absolutePath());
    app.installTranslator(&qtTr);
    app.installTranslator(&appTr);
}

int main(int argc, char *argv[])
{
    QMAApplication app(argc, argv);
    QTranslator appTranslator, qtTranslator;
    QMALogger::initialize();

    app.setOrganizationDomain("hkrn.github.com");
    app.setOrganizationName("MMDAI Project");
    app.setApplicationName("MMDAI");
    app.setApplicationVersion("1.732");
    QMASetSearchPath(app);
    QMALoadTranslations(app, appTranslator, qtTranslator);

    /* invoke QMAWindow */
    MMDAILogInfo("argc: %d", app.arguments().count());
    for (int i = 0; i < app.arguments().count(); i++) {
        MMDAILogInfo("%d: %s", i, app.arguments().at(i).toUtf8().constData());
    }
    QMAWindow window;
    QObject::connect(&app, SIGNAL(fileFound(QString)), &window, SLOT(reload(QString)));
    window.initialize();
    window.show();
    window.loadPlugins();
    window.start();
    return app.exec();
}

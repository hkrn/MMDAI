/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn (libMMDAI)                         */
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

#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtCore/QtPlugin>
#include <QtGui/QApplication>

#include "MMDME/MMDME.h"
#include "QMALogger.h"
#include "QMAWindow.h"

#ifdef QMA_BUNDLE_AQUESTALK2_PLUGIN
Q_IMPORT_PLUGIN(qma_aquestalk2_plugin);
#endif
#ifdef QMA_BUNDLE_PLUGINS
Q_IMPORT_PLUGIN(qma_audio_plugin);
Q_IMPORT_PLUGIN(qma_julius_plugin);
Q_IMPORT_PLUGIN(qma_lookat_plugin);
Q_IMPORT_PLUGIN(qma_openjtalk_plugin);
Q_IMPORT_PLUGIN(qma_vimanager_plugin);
#endif

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMALogger::initialize();
    QTranslator appTranslator, qtTranslator;
    const QString locale = QLocale::system().name();

    app.setOrganizationDomain("hkrn.github.com");
    app.setOrganizationName("MMDAI Project");
    app.setApplicationName("QtMMDAI");
    app.setApplicationVersion("0.61");

    QDir appDir = QDir(app.applicationDirPath());
#if defined(Q_OS_WIN)
    app.addLibraryPath(appDir.absoluteFilePath("Plugins"));
#elif defined(Q_OS_MAC)
    if (appDir.dirName() == "MacOS") {
        appDir.cdUp();
        appDir.cdUp();
        appDir.cdUp();
    }
#endif

    const QString applicationPath = appDir.absolutePath();
    QStringList paths;

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
    const QString translationPath = QDir(app.applicationDirPath()).absoluteFilePath("../Resources");
#else
    const QString translationPath = resourcePath + "/Locales";
#endif
#endif
    paths.clear();
    paths.append(translationPath);
    QDir::setSearchPaths("MMDAITranslations", paths);

    qtTranslator.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    appTranslator.load("QMA_" + locale, translationPath);
    app.installTranslator(&qtTranslator);
    app.installTranslator(&appTranslator);

    /* invoke QMAWindow */
    QMAWindow window;
    window.show();
    return app.exec();
}

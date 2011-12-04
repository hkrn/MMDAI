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

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "Application.h"
#include "MainWindow.h"

static void SetSearchPaths(const QCoreApplication &app)
{
    QStringList paths;
    QDir appDir(app.applicationDirPath()), appBaseDir = appDir;
#if defined(Q_OS_WIN)
    app.addLibraryPath(appDir.absoluteFilePath("Plugins"));
#elif defined(Q_OS_MAC)
    if (appDir.dirName() == "MacOS") {
        appDir.cdUp();
        // macdeployqt deploys on "@executable_path/../Plugins/"
        // It should be loaded Qt's plugins.
        app.addLibraryPath(appDir.absoluteFilePath("PlugIns"));
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

    /* set path to find resources such as model, motion etc. */
#ifdef QMA_RESOURCE_PATH
    const QString resourcePath(QMA_RESOURCE_PATH);
#else
    const QString resourcePath(applicationPath);
#endif
    paths.clear();
    paths.append(resourcePath);
    QDir::setSearchPaths("MMDAIResources", paths);

    /* load translation files from Qt's system path and resource path */
#ifdef QMA_TRANSLATION_PATH
    const QString translationPath(QMA_TRANSLATION_PATH);
#else
#ifdef Q_OS_MAC
    const QString translationPath(QDir::cleanPath(appBaseDir.absoluteFilePath("../Resources")));
#else
    const QString translationPath(resourcePath + "/locales");
#endif
#endif
    paths.clear();
    paths.append(translationPath);
    QDir::setSearchPaths("MMDAITranslations", paths);
}

static void LoadTranslations(QCoreApplication &app, QTranslator &appTr, QTranslator &qtTr)
{
    const QString locale = QLocale::system().name();
    qtTr.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    appTr.load("MMDAI1_" + locale, QDir("MMDAITranslations:/").absolutePath());
    app.installTranslator(&qtTr);
    app.installTranslator(&appTr);
}

int main(int argc, char *argv[])
{
    Application a(argc, argv);
    QTranslator appTranslator, qtTranslator;
    a.setApplicationName("MMDAI");
    a.setApplicationVersion("2.0.0");
    a.setOrganizationDomain("mmdai.github.com");
    a.setOrganizationName("MMDAI");
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    SetSearchPaths(a);
    LoadTranslations(a, appTranslator, qtTranslator);

    MainWindow w;
    if (w.validateLibraryVersion()) {
        w.show();
        return a.exec();
    }
    else {
        return -1;
    }
}

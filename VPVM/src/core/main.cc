/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "Application.h"
#include "LoggerWidget.h"
#include "MainWindow.h"
#include "version.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/AudioSource.h>
#include <vpvl2/extensions/icu4c/Encoding.h>
#include <vpvl2/qt/Util.h>
#include <unicode/udata.h>

#include <QtGui/QtGui>
#include <libxml/xmlwriter.h>

namespace {

using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::qt;

static void SetSearchPaths(const QCoreApplication &app)
{
    QStringList paths;
    QDir appDir(app.applicationDirPath()), appBaseDir = appDir;
#if defined(Q_OS_MAC)
    if (appDir.dirName() == "MacOS") {
        appDir.cdUp();
        // macdeployqt deploys on "@executable_path/../Plugins/"
        // It should be loaded Qt's plugins.
        app.addLibraryPath(appDir.absoluteFilePath("PlugIns"));
        appDir.cdUp();
        appDir.cdUp();
    }
#else
    app.addLibraryPath(appDir.absoluteFilePath("plugins"));
#endif
    const QString applicationPath(appDir.absolutePath());
    /* set path to find configurations (e.g. MMDAI.fst) */
    const QString configPath(applicationPath);
    paths.clear();
    paths.append(configPath);
    QDir::setSearchPaths("plugins", paths);

    /* set path to find resources such as model, motion etc. */
#if defined(Q_OS_MAC)
    const QString resourcePath(QDir::cleanPath(appBaseDir.absoluteFilePath("../Resources")));
#else
    const QString resourcePath(applicationPath + "/resources");
#endif
    paths.clear();
    paths.append(resourcePath);
    QDir::setSearchPaths("resources", paths);

    /* load translation files from Qt's system path and resource path */
#if defined(Q_OS_MAC)
    const QString translationPath(QDir::cleanPath(appBaseDir.absoluteFilePath("../Resources")));
#else
    const QString translationPath(applicationPath + "/resources");
#endif
    paths.clear();
    paths.append(translationPath);
    QDir::setSearchPaths("translations", paths);
}

typedef QSharedPointer<QTranslator> QTranslatorPtr;

static void LoadTranslations(QCoreApplication &app, QList<QTranslatorPtr> &translators)
{
    const QString &dir = QDir("translations:/").absolutePath();
    const QString &locale = QLocale::system().name();
    QTranslator *translator = new QTranslator();
    translator->load("qt_" + locale, dir);
    app.installTranslator(translator);
    translators.append(QTranslatorPtr(translator));
    translator = new QTranslator();
    translator->load("VPVM_" + locale, dir);
    app.installTranslator(translator);
    translators.append(QTranslatorPtr(translator));
}

struct Initializer {
    Initializer() {
        LIBXML_TEST_VERSION;
        xmlInitParser();
        extensions::AudioSource::initialize();
        qt::Util::initializeResources();
    }
    ~Initializer() {
        xmlCleanupParser();
        extensions::AudioSource::terminate();
        qt::Util::cleanupResources();
    }
};

}

int main(int argc, char *argv[])
{
    vpvm::Application a(argc, argv);
    Initializer initializer; Q_UNUSED(initializer);
    QList<QTranslatorPtr> translators;
    a.setApplicationName(VPVM_APPLICATION_NAME);
    a.setApplicationVersion(VPVM_VERSION_STRING);
    a.setOrganizationDomain(VPVM_ORGANIZATION_DOMAIN);
    a.setOrganizationName(VPVM_ORGANIZATION_NAME);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif
    SetSearchPaths(a);
    LoadTranslations(a, translators);

    int result = -1;
    if (!vpvl2::isLibraryVersionCorrect(VPVL2_VERSION)) {
        QWidget fake;
        Util::warning(&fake,
                      QApplication::tr("libvpvl2 version mismatch"),
                      QApplication::tr("libvpvl2's version is incorrect (expected: %1 actual: %2).\n"
                                       "Please replace libvpvl to correct version or reinstall MMDAI.")
                      .arg(VPVL2_VERSION_STRING).arg(vpvl2::libraryVersionString()));
        return result;
    }

    try {
        Encoding::Dictionary dictionary;
        Util::loadDictionary(&dictionary);
        vpvm::LoggerWidget::quietLogMessages(true);
        vpvm::MainWindow w(&dictionary);
        w.show();
        result = a.exec();
    } catch (std::exception &e) {
        QWidget fake;
        Util::warning(&fake,
                      QApplication::tr("Exception caught"),
                      QApplication::tr("Exception caught: %1").arg(e.what()));
    }

    return result;
}

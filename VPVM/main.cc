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

#include "common/Application.h"
#include "common/LoggerWidget.h"
#include "MainWindow.h"

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
#elif defined(Q_OS_MAC)
    const QString resourcePath(QDir::cleanPath(appBaseDir.absoluteFilePath("../Resources")));
#else
    const QString resourcePath(applicationPath + "/resources");
#endif
    paths.clear();
    paths.append(resourcePath);
    QDir::setSearchPaths("MMDAIResources", paths);

    /* load translation files from Qt's system path and resource path */
#ifdef QMA_TRANSLATION_PATH
    const QString translationPath(QMA_TRANSLATION_PATH);
#elif defined(Q_OS_MAC)
    const QString translationPath(QDir::cleanPath(appBaseDir.absoluteFilePath("../Resources")));
#else
    const QString translationPath(applicationPath + "/locales");
#endif
    paths.clear();
    paths.append(translationPath);
    QDir::setSearchPaths("MMDAITranslations", paths);
}

typedef QSharedPointer<QTranslator> QTranslatorPtr;

static void LoadTranslations(QCoreApplication &app, QList<QTranslatorPtr> &translators)
{
    const QString &dir = QDir("MMDAITranslations:/").absolutePath();
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

struct Deleter {
    static void cleanup(Encoding::Dictionary *dictionary) {
        dictionary->releaseAll();
    }
};

}

int main(int argc, char *argv[])
{
    LIBXML_TEST_VERSION;
    xmlInitParser();

    extensions::AudioSource::initialize();
    qt::Util::initializeResources();
    vpvm::Application a(argc, argv);
    vpvm::LoggerWidget::quietLogMessages(true);
    QList<QTranslatorPtr> translators;
    a.setApplicationName("MMDAI2");
    a.setApplicationVersion("0.27.11");
    a.setOrganizationDomain("mmdai.github.com");
    a.setOrganizationName("MMDAI");
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

    // TODO: make external
    QScopedPointer<Encoding::Dictionary, Deleter> dictionary(new Encoding::Dictionary());
    try {
        struct Pair {
            IEncoding::ConstantType type;
            String *value;
        } pairs[] = {
        { IEncoding::kArm, new String("腕") },
        { IEncoding::kAsterisk, new String("*") },
        { IEncoding::kCenter, new String("センター") },
        { IEncoding::kElbow, new String("ひじ") },
        { IEncoding::kFinger, new String("指")} ,
        { IEncoding::kLeft, new String("左") },
        { IEncoding::kLeftKnee, new String("左ひざ") },
        { IEncoding::kRight, new String("右") },
        { IEncoding::kRightKnee, new String("右ひざ") },
        { IEncoding::kSPAExtension, new String(".spa") },
        { IEncoding::kSPHExtension, new String(".sph") },
        { IEncoding::kWrist, new String("手首") },
        { IEncoding::kRootBone, new String("全ての親") },
        { IEncoding::kScaleBoneAsset, new String("拡大率") },
        { IEncoding::kOpacityMorphAsset, new String("不透明度") }
    };
        const int nconstants = sizeof(pairs) / sizeof(pairs[0]);
        for (int i = 0; i < nconstants; i++) {
            Pair &pair = pairs[i];
            dictionary->insert(pair.type, pair.value);
        }
        vpvm::MainWindow w(dictionary.data());
        w.show();
        result = a.exec();
    } catch (std::exception &e) {
        QWidget fake;
        Util::warning(&fake,
                      QApplication::tr("Exception caught"),
                      QApplication::tr("Exception caught: %1").arg(e.what()));
    }
    xmlCleanupParser();
    extensions::AudioSource::terminate();
    qt::Util::cleanupResources();

    return result;
}

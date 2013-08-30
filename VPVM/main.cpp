/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/extensions/BaseApplicationContext.h"

#include <QtGui>
#include <QtQuick>
#include <QApplication>

#include "BoneKeyframeRefObject.h"
#include "BoneMotionTrack.h"
#include "BoneRefObject.h"
#include "CameraKeyframeRefObject.h"
#include "CameraMotionTrack.h"
#include "CameraRefObject.h"
#include "LabelRefObject.h"
#include "LightKeyframeRefObject.h"
#include "LightMotionTrack.h"
#include "LightRefObject.h"
#include "ModelProxy.h"
#include "MorphKeyframeRefObject.h"
#include "MorphMotionTrack.h"
#include "MorphRefObject.h"
#include "MotionProxy.h"
#include "Preference.h"
#include "RenderTarget.h"
#include "ProjectProxy.h"
#include "UIAuxHelper.h"

using namespace vpvl2::extensions;

namespace {

static QString adjustPath(const QString &path)
{
#if defined(Q_OS_MAC)
    if (!QDir::isAbsolutePath(path)) {
        return QString::fromLatin1("%1/../Resources/%2")
                .arg(QCoreApplication::applicationDirPath(), path);
    }
#elif defined(Q_OS_QNX)
    if (!QDir::isAbsolutePath(path)) {
        return QString::fromLatin1("app/native/%1").arg(path);
    }
#elif defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
    const QString pathInInstallDir =
            QString::fromLatin1("%1/../%2").arg(QCoreApplication::applicationDirPath(), path);
    if (QFileInfo(pathInInstallDir).exists()) {
        return pathInInstallDir;
    }
#endif
    return path;
}

static void prepareRegal()
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

static QObject *createUIAuxHelper(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);
    QObject *value = new UIAuxHelper();
    return value;
}

void registerQmlTypes()
{
    qmlRegisterUncreatableType<BaseKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "BaseKeyframe", "");
    qmlRegisterUncreatableType<BaseMotionTrack>("com.github.mmdai.VPVM", 1, 0, "BaseMotionTrack", "");
    qmlRegisterUncreatableType<BoneKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "BoneKeyframe", "");
    qmlRegisterUncreatableType<BoneMotionTrack>("com.github.mmdai.VPVM", 1, 0, "BoneMotionTrack", "");
    qmlRegisterUncreatableType<BoneRefObject>("com.github.mmdai.VPVM", 1, 0, "Bone", "");
    qmlRegisterUncreatableType<CameraKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "CameraKeyframe", "");
    qmlRegisterUncreatableType<CameraMotionTrack>("com.github.mmdai.VPVM", 1, 0, "CameraMotionTrack", "");
    qmlRegisterUncreatableType<CameraRefObject>("com.github.mmdai.VPVM", 1, 0, "Camera", "");
    qmlRegisterUncreatableType<LabelRefObject>("com.github.mmdai.VPVM", 1, 0, "Label", "");
    qmlRegisterUncreatableType<LightKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "LightKeyframe", "");
    qmlRegisterUncreatableType<LightMotionTrack>("com.github.mmdai.VPVM", 1, 0, "LightMotionTrack", "");
    qmlRegisterUncreatableType<LightRefObject>("com.github.mmdai.VPVM", 1, 0, "Light", "");
    qmlRegisterUncreatableType<ModelProxy>("com.github.mmdai.VPVM", 1, 0, "Model", "");
    qmlRegisterUncreatableType<MorphKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "MorphKeyframe", "");
    qmlRegisterUncreatableType<MorphMotionTrack>("com.github.mmdai.VPVM", 1, 0, "MorphMotionTrack", "");
    qmlRegisterUncreatableType<MorphRefObject>("com.github.mmdai.VPVM", 1, 0, "Morph", "");
    qmlRegisterUncreatableType<MotionProxy>("com.github.mmdai.VPVM", 1, 0, "Motion", "");
    qmlRegisterUncreatableType<Preference>("com.github.mmdai.VPVM", 1, 0, "Preference", "");
    qmlRegisterSingletonType<UIAuxHelper>("com.github.mmdai.VPVM", 1, 0, "UIAuxHelper", createUIAuxHelper);
    qmlRegisterType<RenderTarget>("com.github.mmdai.VPVM", 1, 0, "RenderTarget");
    qmlRegisterType<ProjectProxy>("com.github.mmdai.VPVM", 1, 0, "Project");
}

}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    BaseApplicationContext::initializeOnce(argv[0]);

    app.setApplicationDisplayName("VPVM");
    app.setApplicationName("VPVM");
    app.setApplicationVersion("0.31.0");
    app.setOrganizationName("MMDAI Project");
    app.setOrganizationDomain("mmdai.github.com");
    QTranslator translator;
    translator.load(QLocale::system(), "VPVM", ".", adjustPath("translations"), ".qm");
    app.installTranslator(&translator);

    prepareRegal();
    registerQmlTypes();

    QQmlApplicationEngine engine;
    Preference applicationPreference;
    engine.rootContext()->setContextProperty("applicationPreference", &applicationPreference);
#ifdef QT_NO_DEBUG
    engine.setImportPathList(QStringList() << adjustPath("qml"));
    engine.setPluginPathList(QStringList() << adjustPath("qml/plugins"));
    engine.load(QUrl("qrc:///qml/VPVM/main.qml"));
#else
    engine.load(adjustPath("qml/VPVM/main.qml"));
#endif

    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
    Q_ASSERT(window);
    QSurfaceFormat format = window->format();
    format.setSamples(applicationPreference.samples());
#if 0
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
#endif
    window->setFormat(format);
#ifdef Q_OS_MACX
    window->setFlags(window->flags() | Qt::WindowFullscreenButtonHint);
#endif
    window->show();

    int result = app.exec();
    BaseApplicationContext::terminate();

    return result;
}

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

/* hack for qopengl.h compilation errors */
#define GL_ARB_shader_objects
#define GL_KHR_debug

#include <QtCore>
#include <QtGui>
#include <QtQuick>
#include <QApplication>

#include "ALAudioContext.h"
#include "ALAudioEngine.h"
#include "BoneKeyframeRefObject.h"
#include "BoneMotionTrack.h"
#include "BoneRefObject.h"
#include "CameraKeyframeRefObject.h"
#include "CameraMotionTrack.h"
#include "CameraRefObject.h"
#include "GraphicsDevice.h"
#include "Grid.h"
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
#include "Util.h"
#include "WorldProxy.h"

using namespace vpvl2::extensions;

namespace {

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

static QObject *createALAudioContext(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine);
    QObject *value = new ALAudioContext(engine);
    return value;
}

static QObject *createUIAuxHelper(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine);
    QObject *value = new UIAuxHelper(engine);
    return value;
}

void registerQmlTypes()
{
    qmlRegisterSingletonType<ALAudioContext>("com.github.mmdai.VPVM", 1, 0, "ALAudioContext", createALAudioContext);
    qmlRegisterType<ALAudioEngine>("com.github.mmdai.VPVM", 1, 0, "ALAudioEngine");
    qmlRegisterUncreatableType<BaseKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "BaseKeyframe", "");
    qmlRegisterUncreatableType<BaseMotionTrack>("com.github.mmdai.VPVM", 1, 0, "BaseMotionTrack", "");
    qmlRegisterUncreatableType<BoneKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "BoneKeyframe", "");
    qmlRegisterUncreatableType<BoneMotionTrack>("com.github.mmdai.VPVM", 1, 0, "BoneMotionTrack", "");
    qmlRegisterUncreatableType<BoneRefObject>("com.github.mmdai.VPVM", 1, 0, "Bone", "");
    qmlRegisterUncreatableType<CameraKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "CameraKeyframe", "");
    qmlRegisterUncreatableType<CameraMotionTrack>("com.github.mmdai.VPVM", 1, 0, "CameraMotionTrack", "");
    qmlRegisterUncreatableType<CameraRefObject>("com.github.mmdai.VPVM", 1, 0, "Camera", "");
    qmlRegisterUncreatableType<GraphicsDevice>("com.github.mmdai.VPVM", 1, 0, "GraphicsDevice", "");
    qmlRegisterUncreatableType<Grid>("com.github.mmdai.VPVM", 1, 0, "Grid", "");
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
    qmlRegisterUncreatableType<WorldProxy>("com.github.mmdai.VPVM", 1, 0, "World", "");
    qmlRegisterSingletonType<UIAuxHelper>("com.github.mmdai.VPVM", 1, 0, "UIAuxHelper", createUIAuxHelper);
    qmlRegisterType<RenderTarget>("com.github.mmdai.VPVM", 1, 0, "RenderTarget");
    qmlRegisterType<ProjectProxy>("com.github.mmdai.VPVM", 1, 0, "Project");
}

}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Preference applicationPreference;
    const QString &loggingDirectory = applicationPreference.initializeLoggingDirectory();
    int verboseLogLevel = applicationPreference.verboseLogLevel();
    BaseApplicationContext::initializeOnce(argv[0],  qPrintable(loggingDirectory), verboseLogLevel);

    app.setApplicationDisplayName("VPVM");
    app.setApplicationName("VPVM");
    app.setApplicationVersion("0.31.3");
    app.setOrganizationName("MMDAI Project");
    app.setOrganizationDomain("mmdai.github.com");
    if (applicationPreference.isFontFamilyToGUIShared()) {
        app.setFont(applicationPreference.fontFamily());
    }
    QTranslator translator;
    translator.load(QLocale::system(), "VPVM", ".", Util::resourcePath("translations"), ".qm");
    app.installTranslator(&translator);

    prepareRegal();
    registerQmlTypes();

    QQuickWindow::setDefaultAlphaBuffer(applicationPreference.isTransparentWindowEnabled());
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("applicationPreference", &applicationPreference);
#ifdef QT_NO_DEBUG
    engine.setImportPathList(QStringList() << Util::resourcePath("qml"));
    engine.setPluginPathList(QStringList() << Util::resourcePath("qml/plugins"));
    engine.load(QUrl("qrc:///qml/VPVM/main.qml"));
#else
    engine.load(Util::resourcePath("qml/VPVM/main.qml"));
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
    QThreadPool *threadPool = QThreadPool::globalInstance();
    if (threadPool->activeThreadCount() > 0) {
        threadPool->waitForDone();
    }
    BaseApplicationContext::terminate();

    return result;
}

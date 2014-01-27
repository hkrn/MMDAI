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

#include <QtQuick>
#include <QCommandLineParser>

#include "Common.h"
#include "Application.h"
#include "BoneKeyframeRefObject.h"
#include "BoneMotionTrack.h"
#include "BoneRefObject.h"
#include "CameraKeyframeRefObject.h"
#include "CameraMotionTrack.h"
#include "CameraRefObject.h"
#include "GraphicsDevice.h"
#include "Grid.h"
#include "JointRefObject.h"
#include "LabelRefObject.h"
#include "LightKeyframeRefObject.h"
#include "LightMotionTrack.h"
#include "LightRefObject.h"
#include "LoggingThread.h"
#include "MaterialRefObject.h"
#include "ModelProxy.h"
#include "MorphKeyframeRefObject.h"
#include "MorphMotionTrack.h"
#include "MorphRefObject.h"
#include "MotionProxy.h"
#include "Preference.h"
#include "ProjectProxy.h"
#include "RenderTarget.h"
#include "RigidBodyRefObject.h"
#include "Util.h"
#include "Vector3RefObject.h"
#include "VertexRefObject.h"
#include "WorldProxy.h"

using namespace vpvl2::extensions;

namespace {

static void registerQmlTypes()
{
    qmlRegisterUncreatableType<BaseKeyframeRefObject>("com.github.mmdai.VPMM", 1, 0, "BaseKeyframe", "");
    qmlRegisterUncreatableType<BaseMotionTrack>("com.github.mmdai.VPMM", 1, 0, "BaseMotionTrack", "");
    qmlRegisterUncreatableType<BoneKeyframeRefObject>("com.github.mmdai.VPMM", 1, 0, "BoneKeyframe", "");
    qmlRegisterUncreatableType<BoneMotionTrack>("com.github.mmdai.VPMM", 1, 0, "BoneMotionTrack", "");
    qmlRegisterUncreatableType<BoneRefObject>("com.github.mmdai.VPMM", 1, 0, "Bone", "");
    qmlRegisterUncreatableType<CameraKeyframeRefObject>("com.github.mmdai.VPMM", 1, 0, "CameraKeyframe", "");
    qmlRegisterUncreatableType<CameraMotionTrack>("com.github.mmdai.VPMM", 1, 0, "CameraMotionTrack", "");
    qmlRegisterUncreatableType<CameraRefObject>("com.github.mmdai.VPMM", 1, 0, "Camera", "");
    qmlRegisterUncreatableType<ChildBoneMorphRefObject>("com.github.mmdai.VPMM", 1, 0, "ChildBoneMorph", "");
    qmlRegisterUncreatableType<ChildFlipMorphRefObject>("com.github.mmdai.VPMM", 1, 0, "ChildFlipMorph", "");
    qmlRegisterUncreatableType<ChildGroupMorphRefObject>("com.github.mmdai.VPMM", 1, 0, "ChildGroupMorph", "");
    qmlRegisterUncreatableType<ChildImpulseMorphRefObject>("com.github.mmdai.VPMM", 1, 0, "ChildImpulseMorph", "");
    qmlRegisterUncreatableType<ChildMaterialMorphRefObject>("com.github.mmdai.VPMM", 1, 0, "ChildMaterialMorph", "");
    qmlRegisterUncreatableType<ChildUVMorphRefObject>("com.github.mmdai.VPMM", 1, 0, "ChildUVMorph", "");
    qmlRegisterUncreatableType<ChildVertexMorphRefObject>("com.github.mmdai.VPMM", 1, 0, "ChildVertexMorph", "");
    qmlRegisterUncreatableType<GraphicsDevice>("com.github.mmdai.VPMM", 1, 0, "GraphicsDevice", "");
    qmlRegisterUncreatableType<Grid>("com.github.mmdai.VPMM", 1, 0, "Grid", "");
    qmlRegisterUncreatableType<JointRefObject>("com.github.mmdai.VPMM", 1, 0, "Joint", "");
    qmlRegisterUncreatableType<LabelRefObject>("com.github.mmdai.VPMM", 1, 0, "Label", "");
    qmlRegisterUncreatableType<LightKeyframeRefObject>("com.github.mmdai.VPMM", 1, 0, "LightKeyframe", "");
    qmlRegisterUncreatableType<LightMotionTrack>("com.github.mmdai.VPMM", 1, 0, "LightMotionTrack", "");
    qmlRegisterUncreatableType<LightRefObject>("com.github.mmdai.VPMM", 1, 0, "Light", "");
    qmlRegisterUncreatableType<MaterialRefObject>("com.github.mmdai.VPMM", 1, 0, "Material", "");
    qmlRegisterUncreatableType<ModelProxy>("com.github.mmdai.VPMM", 1, 0, "Model", "");
    qmlRegisterUncreatableType<MorphKeyframeRefObject>("com.github.mmdai.VPMM", 1, 0, "MorphKeyframe", "");
    qmlRegisterUncreatableType<MorphMotionTrack>("com.github.mmdai.VPMM", 1, 0, "MorphMotionTrack", "");
    qmlRegisterUncreatableType<MorphRefObject>("com.github.mmdai.VPMM", 1, 0, "Morph", "");
    qmlRegisterUncreatableType<MotionProxy>("com.github.mmdai.VPMM", 1, 0, "Motion", "");
    qmlRegisterUncreatableType<Preference>("com.github.mmdai.VPMM", 1, 0, "Preference", "");
    qmlRegisterType<ProjectProxy>("com.github.mmdai.VPMM", 1, 0, "Project");
    qmlRegisterType<RenderTarget>("com.github.mmdai.VPMM", 1, 0, "RenderTarget");
    qmlRegisterUncreatableType<RigidBodyRefObject>("com.github.mmdai.VPMM", 1, 0, "RigidBody", "");
    qmlRegisterType<Vector3RefObject>("com.github.mmdai.VPMM", 1, 0, "Vector3");
    qmlRegisterUncreatableType<VertexRefObject>("com.github.mmdai.VPMM", 1, 0, "Vertex", "");
    qmlRegisterUncreatableType<WorldProxy>("com.github.mmdai.VPMM", 1, 0, "World", "");
}

}

int main(int argc, char *argv[])
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr("VPMM (a.k.a MMDAI2) is an application to import/edit model like PMXEditor (PMXE)"));
    Application application(&parser, argc, argv);
    QTranslator translator;
    translator.load(QLocale::system(), "VPMM", ".", Util::resourcePath("translations"), ".qm");
    application.installTranslator(&translator);

    Preference applicationPreference;
    const QString &loggingDirectory = applicationPreference.initializeLoggingDirectory();
    int verboseLogLevel = applicationPreference.verboseLogLevel();
    BaseApplicationContext::initializeOnce(argv[0],  qPrintable(loggingDirectory), verboseLogLevel);
    if (applicationPreference.isFontFamilyToGUIShared()) {
        application.setFont(applicationPreference.fontFamily());
    }
    prepareRegal();
    registerQmlTypes();

    QQuickWindow::setDefaultAlphaBuffer(applicationPreference.isTransparentWindowEnabled());
    QQmlApplicationEngine engine;
    QQmlContext *rootContext = engine.rootContext();
    rootContext->setContextProperty("applicationPreference", &applicationPreference);
    rootContext->setContextProperty("applicationBootstrapOption", &application);
    g_loggingThread.setDirectory(loggingDirectory);
    QThreadPool::globalInstance()->start(&g_loggingThread);
#ifdef QT_NO_DEBUG
    engine.load(QUrl("qrc:///qml/VPVM/main.qml"));
#else
    engine.load(Util::resourcePath("qml/main.qml"));
#endif
    displayApplicationWindow(engine.rootObjects().value(0), applicationPreference.samples());

    int result = application.exec();
    g_loggingThread.stop();
    BaseApplicationContext::terminate();

    return result;
}

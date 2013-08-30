/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#include "BackgroundImage.h"
#include "ExtendedSceneWidget.h"
#include "SceneLoader.h"
#include "Script.h"
#include "TiledStage.h"
#include "common/util.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;
using namespace internal;

ExtendedSceneWidget::ExtendedSceneWidget(vpvl2::IEncoding *encoding,
                                         vpvl2::Factory *factory,
                                         QSettings *settings,
                                         QWidget *parent)
    : SceneWidget(encoding, factory, settings, parent),
      m_script(0),
      m_tiledStage(0),
      m_enableTransparent(false)
{
}

ExtendedSceneWidget::~ExtendedSceneWidget()
{
    delete m_script;
    m_script = 0;
    delete m_tiledStage;
    m_tiledStage = 0;
}

void ExtendedSceneWidget::clear()
{
    stop();
    m_loader->release();
}

void ExtendedSceneWidget::loadScript()
{
    loadScript(internal::openFileDialog("sceneWidget/lastScriptDirectory",
                                        tr("Open script file"),
                                        tr("Script file (*.fst)"),
                                        m_settings));
}

void ExtendedSceneWidget::loadScript(const QString &filename)
{
    QFile file(filename);
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        stop();
        clear();
        delete m_script;
        m_script = new Script(this);
        m_script->setDir(QFileInfo(file).absoluteDir());
        m_script->load(stream);
        emit scriptDidLoaded(filename);
        const QFileInfo info(file);
        const QDir &dir = info.dir();
        m_script->loadSpeechEngine(dir, info.baseName());
        m_script->loadSpeechRecognitionEngine(dir, info.baseName());
        m_script->start();
        play();
    }
    else {
        qWarning("%s", qPrintable(tr("Cannot load script %1: %2").arg(filename).arg(file.errorString())));
    }
}

void ExtendedSceneWidget::dropEvent(QDropEvent *event)
{
    SceneWidget::dropEvent(event);
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        foreach (const QUrl url, mimeData->urls()) {
            QString path = url.toLocalFile();
            if (path.endsWith(".fst"))
                loadScript(path);
            qDebug() << "Proceeded a dropped file:" << path;
        }
    }
}

void ExtendedSceneWidget::initializeGL()
{
    SceneWidget::initializeGL();
    QStringList arguments = qApp->arguments();
    m_tiledStage = new TiledStage(m_loader->world());
    m_loader->setPhysicsEnabled(true);
    m_loader->startPhysicsSimulation();
    connect(m_loader, SIGNAL(modelDidAdd(vpvl2::IModel*,QUuid)), SLOT(setDefaultModelShadowSetting(vpvl2::IModel*)));
    setShowModelDialog(false);
    if (arguments.count() == 2)
        loadScript(arguments[1]);
    else
        play();
    /* vpvl::Scene の初期値を変更したため、互換性のために視点を変更する */
    Scene::ICamera *camera = m_loader->scene()->camera();
    camera->setPosition(Vector3(0, 13, 0));
    camera->setFov(16);
    camera->setDistance(100);
    emit cameraPerspectiveDidSet(camera);
}

#ifdef Q_OS_MAC
extern void UISetGLContextTransparent(bool value);
#else
#define UISetGLContextTransparent(value) (void)0
#endif

void ExtendedSceneWidget::paintGL()
{
    Scene *scene = m_loader->scene();
    QMatrix4x4 shadowMatrix;
    m_loader->setLightViewProjectionMatrix(shadowMatrix);
    m_loader->bindDepthTexture();
    m_loader->renderZPlot();
    m_loader->releaseDepthTexture();
    m_loader->setLightViewProjectionTextureMatrix(shadowMatrix);
    scene->light()->setToonEnable(true);
    /* 通常のレンダリングを行うよう切り替えてレンダリングする */
    UISetGLContextTransparent(m_enableTransparent);
    qglClearColor(m_enableTransparent ? Qt::transparent : Qt::darkBlue);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    m_background->draw();
    const QMatrix4x4 &projection = m_loader->projectionMatrix();
    m_tiledStage->renderBackground(scene, projection);
    m_tiledStage->renderFloor(scene, projection);
    m_loader->renderModels();
    if (m_script) {
        const QList<IMotion *> &motions = m_script->stoppedMotions();
        if (!motions.isEmpty())
            emit motionDidFinished(motions);
    }
}

void ExtendedSceneWidget::setDefaultModelShadowSetting(IModel *model)
{
    //m_loader->setSelfShadowEnable(model, true);
    m_loader->setProjectiveShadowEnable(model, false);
}

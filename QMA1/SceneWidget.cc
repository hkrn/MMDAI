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

#include "SceneWidget.h"

#include "Application.h"
#include "Delegate.h"
#include "SceneLoader.h"
#include "Script.h"
#include "TiledStage.h"
#include "World.h"

#include <QtGui/QtGui>
#include <btBulletDynamicsCommon.h>
#include <vpvl/vpvl.h>
#include <vpvl/gl/Renderer.h>
#include "util.h"

SceneWidget::SceneWidget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_bone(0),
    m_delegate(0),
    m_player(0),
    m_loader(0),
    m_script(0),
    m_world(0),
    m_settings(0),
    m_frameCount(0),
    m_currentFPS(0),
    m_defaultFPS(60),
    m_interval(1000.0f / m_defaultFPS),
    m_internalTimerID(0),
    m_visibleBones(false),
    m_playing(true)
{
    m_delegate = new Delegate(this);
    m_world = new World(m_defaultFPS);
    m_tiledStage = new TiledStage(m_delegate, m_world);
    setAcceptDrops(true);
    setAutoFillBackground(false);
    setMinimumSize(540, 480);
    connect(static_cast<Application *>(qApp), SIGNAL(fileDidRequest(QString)), this, SLOT(loadScript(QString)));
}

SceneWidget::~SceneWidget()
{
    delete m_script;
    m_script = 0;
    delete m_tiledStage;
    m_tiledStage = 0;
    delete m_renderer;
    m_renderer = 0;
    delete m_delegate;
    m_delegate = 0;
    delete m_world;
    m_world = 0;
}

void SceneWidget::play()
{
    m_playing = true;
    emit sceneDidPlay();
}

void SceneWidget::pause()
{
    m_playing = false;
    emit sceneDidPause();
}

void SceneWidget::stop()
{
    m_playing = false;
    m_renderer->scene()->resetMotion();
    emit sceneDidStop();
}

void SceneWidget::clear()
{
    stop();
    m_loader->release();
}

vpvl::PMDModel *SceneWidget::findModel(const QString &name)
{
    return m_loader->findModel(name);
}

void SceneWidget::setCurrentFPS(int value)
{
    if (value > 0) {
        m_defaultFPS = value;
        m_world->setCurrentFPS(value);
        m_renderer->scene()->setPreferredFPS(value);
    }
}

vpvl::PMDModel *SceneWidget::selectedModel() const
{
    return m_renderer->selectedModel();
}

void SceneWidget::setSelectedModel(vpvl::PMDModel *value)
{
    m_renderer->setSelectedModel(value);
    emit modelDidSelect(value);
}

void SceneWidget::addModel()
{
    vpvl::PMDModel *model = addModel(openFileDialog("sceneWidget/lastPMDDirectory",
                                                    tr("Open PMD file"),
                                                    tr("PMD file (*.pmd)")));
    if (model && !m_playing)
        model->updateImmediate();
}

vpvl::PMDModel *SceneWidget::addModel(const QString &path)
{
    QFileInfo fi(path);
    vpvl::PMDModel *model = 0;
    if (fi.exists()) {
        QProgressDialog *progress = getProgressDialog("Loading the model...", 0);
        model = m_loader->loadModel(fi.fileName(), fi.dir());
        if (model) {
            emit modelDidAdd(model);
            setSelectedModel(model);
        }
        else {
            QMessageBox::warning(this, tr("Loading model error"),
                                 tr("%1 cannot be loaded").arg(fi.fileName()));
        }
        delete progress;
    }
    return model;
}

void SceneWidget::insertMotionToAllModels()
{
    vpvl::VMDMotion *motion = insertMotionToAllModels(openFileDialog("sceneWidget/lastVMDDirectory",
                                                                     tr("Open VMD (for model) file"),
                                                                     tr("VMD file (*.vmd)")));
    if (motion)
        selectedModel()->updateImmediate();
}

vpvl::VMDMotion *SceneWidget::insertMotionToAllModels(const QString &path)
{
    vpvl::VMDMotion *motion = 0;
    if (QFile::exists(path)) {
        QList<vpvl::PMDModel *> models;
        motion = m_loader->loadModelMotion(path, models);
        if (motion) {
            foreach (vpvl::PMDModel *model, models)
                emit motionDidAdd(motion, model);
        }
        else {
            QMessageBox::warning(this, tr("Loading model motion error"),
                                 tr("%1 cannot be loaded").arg(QFileInfo(path).fileName()));
        }
    }
    return motion;
}

void SceneWidget::insertMotionToSelectedModel()
{
    vpvl::VMDMotion *motion = insertMotionToSelectedModel(openFileDialog("sceneWidget/lastVMDDirectory",
                                                                         tr("Open VMD (for model) file"),
                                                                         tr("VMD file (*.vmd)")));
    if (motion)
        advanceMotion(0.0f);
}

vpvl::VMDMotion *SceneWidget::insertMotionToSelectedModel(const QString &path)
{
    return insertMotionToModel(path, m_renderer->selectedModel());
}

vpvl::VMDMotion *SceneWidget::insertMotionToModel(const QString &path, vpvl::PMDModel *model)
{
    vpvl::VMDMotion *motion = 0;
    if (model) {
        if (QFile::exists(path)) {
            motion = m_loader->loadModelMotion(path, model);
            if (motion)
                emit motionDidAdd(motion, model);
            else
                QMessageBox::warning(this, tr("Loading model motion error"),
                                     tr("%1 cannot be loaded").arg(QFileInfo(path).fileName()));
        }
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."),
                             tr("Select a model to insert the motion"));
    }
    return motion;
}

vpvl::VMDMotion *SceneWidget::insertMotionToModel(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (motion && model) {
        m_loader->setModelMotion(motion, model);
        return motion;
    }
    return 0;
}

void SceneWidget::addAsset()
{
    addAsset(openFileDialog("sceneWidget/lastAssetDirectory",
                            tr("Open X file"),
                            tr("DirectX mesh file (*.x)")));
}

vpvl::Asset *SceneWidget::addAsset(const QString &path)
{
    QFileInfo fi(path);
    vpvl::Asset *asset = 0;
    if (fi.exists()) {
        QProgressDialog *progress = getProgressDialog("Loading the stage...", 0);
        asset = m_loader->loadAsset(fi.fileName(), fi.dir());
        if (asset)
            emit assetDidAdd(asset);
        else
            QMessageBox::warning(this, tr("Loading asset error"),
                                 tr("%1 cannot be loaded").arg(fi.fileName()));
        delete progress;
    }
    return asset;
}

void SceneWidget::advanceMotion(float frameIndex)
{
    vpvl::Scene *scene = m_renderer->scene();
    scene->updateModelView(0);
    scene->updateProjection(0);
    scene->advanceMotion(frameIndex);
    updateGL();
}

void SceneWidget::seekMotion(float frameIndex)
{
    vpvl::Scene *scene = m_renderer->scene();
    scene->updateModelView(0);
    scene->updateProjection(0);
    scene->seekMotion(frameIndex);
    updateGL();
}

void SceneWidget::setCamera()
{
    vpvl::VMDMotion *motion = setCamera(openFileDialog("sceneWidget/lastCameraDirectory",
                                                       tr("Open VMD (for camera) file"),
                                                       tr("VMD file (*.vmd)")));
    if (motion)
        advanceMotion(0.0f);
}

vpvl::VMDMotion *SceneWidget::setCamera(const QString &path)
{
    vpvl::VMDMotion *motion = 0;
    if (QFile::exists(path)) {
        motion = m_loader->loadCameraMotion(path);
        if (motion)
            emit cameraMotionDidSet(motion);
        else
            QMessageBox::warning(this, tr("Loading camera motion error"),
                                 tr("%1 cannot be loaded").arg(QFileInfo(path).fileName()));
    }
    return motion;
}

void SceneWidget::deleteSelectedModel()
{
    vpvl::PMDModel *selected = m_renderer->selectedModel();
    emit modelWillDelete(selected);
    if (m_loader->deleteModel(selected)) {
        emit modelDidSelect(0);
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected or exist."),
                             tr("Select a model to delete"));
    }
}

void SceneWidget::deleteModel(vpvl::PMDModel *model)
{
    m_loader->deleteModel(model);
}

void SceneWidget::deleteMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    m_loader->deleteModelMotion(motion, model);
}

void SceneWidget::resetCamera()
{
    vpvl::Scene *scene = m_renderer->scene();
    scene->resetCamera();
    emit cameraPerspectiveDidSet(scene->position(), scene->angle(), scene->fovy(), scene->distance());
}

void SceneWidget::setLightColor(const btVector4 &color)
{
    vpvl::Scene *scene = m_renderer->scene();
    scene->setLightSource(color, scene->lightPosition());
    emit lightColorDidSet(color);
}

void SceneWidget::setLightPosition(const btVector3 &position)
{
    vpvl::Scene *scene = m_renderer->scene();
    scene->setLightSource(scene->lightColor(), position);
    m_tiledStage->updateShadowMatrix(position);
    emit lightPositionDidSet(position);
}

void SceneWidget::setCameraPerspective(btVector3 *pos, btVector3 *angle, float *fovy, float *distance)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 posValue, angleValue;
    float fovyValue, distanceValue;
    posValue = !pos ? scene->position() : *pos;
    angleValue = !angle ? scene->angle() : *angle;
    fovyValue = !fovy ? scene->fovy() : *fovy;
    distanceValue = !distance ? scene->distance() : *distance;
    scene->setCameraPerspective(posValue, angleValue, fovyValue, distanceValue);
    emit cameraPerspectiveDidSet(posValue, angleValue, fovyValue, distanceValue);
}

void SceneWidget::loadScript()
{
    loadScript(openFileDialog("sceneWidget/lastScriptDirectory",
                              tr("Open script file"),
                              tr("Script file (*.fst)")));
}

void SceneWidget::loadScript(const QString &filename)
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

void SceneWidget::setBones(const QList<vpvl::Bone *> &bones)
{
    m_bone = bones.isEmpty() ? 0 : bones.last();
}

void SceneWidget::rotate(float x, float y)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    angle.setValue(angle.x() + x, angle.y() + y, angle.z());
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::translate(float x, float y)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    pos.setValue(pos.x() + x, pos.y() + y, pos.z());
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::zoom(bool up, const Qt::KeyboardModifiers &modifiers)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    float fovyStep = 1.0f, distanceStep = 4.0f;
    if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
        fovy = up ? fovy - fovyStep : fovy + fovyStep;
    }
    else {
        if (modifiers & Qt::ControlModifier)
            distanceStep *= 5.0f;
        else if (modifiers & Qt::ShiftModifier)
            distanceStep *= 0.2f;
        if (distanceStep != 0.0f)
            distance = up ? distance - distanceStep : distance + distanceStep;
    }
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::closeEvent(QCloseEvent *event)
{
    killTimer(m_internalTimerID);
    event->accept();
}

void SceneWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void SceneWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void SceneWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void SceneWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        const QList<QUrl> urls = mimeData->urls();
        vpvl::PMDModel *model = m_renderer->selectedModel();
        foreach (const QUrl url, urls) {
            QString path = url.toLocalFile();
            if (path.endsWith(".pmd", Qt::CaseInsensitive)) {
                QFileInfo modelPath(path);
                vpvl::PMDModel *model = m_loader->loadModel(modelPath.baseName(), modelPath.dir());
                if (model) {
                    emit modelDidAdd(model);
                    setSelectedModel(model);
                }
            }
            else if (path.endsWith(".vmd") && model) {
                vpvl::VMDMotion *motion = m_loader->loadModelMotion(path, model);
                if (motion)
                    emit motionDidAdd(motion, model);
            }
            else if (path.endsWith(".fst")) {
                loadScript(path);
            }
            qDebug() << "Proceeded a dropped file:" << path;
        }
    }
}

void SceneWidget::initializeGL()
{
    GLenum err;
    if (!vpvl::gl::Renderer::initializeGLEW(err))
        qFatal("Cannot initialize GLEW: %s", glewGetErrorString(err));
    else
        qDebug("GLEW version: %s", glewGetString(GLEW_VERSION));
    qDebug("VPVL version: %s (%d)", VPVL_VERSION_STRING, VPVL_VERSION);
    qDebug("GL_VERSION: %s", glGetString(GL_VERSION));
    qDebug("GL_VENDOR: %s", glGetString(GL_VENDOR));
    qDebug("GL_RENDERER: %s", glGetString(GL_RENDERER));
    m_renderer = new vpvl::gl::Renderer(m_delegate, width(), height(), m_defaultFPS);
    m_loader = new SceneLoader(m_renderer);
    m_renderer->setDebugDrawer(m_world->mutableWorld());
    vpvl::Scene *scene = m_renderer->scene();
    scene->setViewMove(0);
    scene->setWorld(m_world->mutableWorld());
    m_timer.start();
    m_internalTimerID = startTimer(m_interval);
    QStringList arguments = qApp->arguments();
    if (arguments.count() == 2)
        loadScript(arguments[1]);
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
    m_prevPos = event->pos();
}

void SceneWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        QPoint diff = event->pos() - m_prevPos;
        if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            vpvl::Scene *scene = m_renderer->scene();
            btVector3 position = scene->lightPosition();
            btQuaternion rx(0.0f, diff.y() * vpvl::radian(0.1f), 0.0f),
                    ry(0.0f, diff.x() * vpvl::radian(0.1f), 0.0f);
            position = position * btMatrix3x3(rx * ry);
            scene->setLightSource(scene->lightColor(), position);
        }
        else if (modifiers & Qt::ShiftModifier) {
            translate(diff.x() * -0.1f, diff.y() * 0.1f);
        }
        else {
            rotate(diff.y() * 0.5f, diff.x() * 0.5f);
        }
        m_prevPos = event->pos();
    }
}

void SceneWidget::paintGL()
{
    qreal matrix[16];
    qglClearColor(Qt::darkBlue);
    m_tiledStage->updateShadowMatrix(m_renderer->scene()->lightPosition());
    m_renderer->initializeSurface();
    m_renderer->clearSurface();
    m_tiledStage->renderBackground();
    // pre shadow
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    m_tiledStage->renderFloor();
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    m_tiledStage->shadowMatrix().copyDataTo(matrix);
    glMultMatrixd(matrix);
    // draw shadows
    m_renderer->drawShadow();
    // post shadow
    glPopMatrix();
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    glStencilFunc(GL_EQUAL, 2, ~0);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    m_tiledStage->renderFloor();
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    // draw all assets and models
    m_renderer->drawAssets();
    m_renderer->drawModels();
    drawBones();
    emit motionDidFinished(m_loader->stoppedMotions());
}

void SceneWidget::resizeGL(int w, int h)
{
    m_renderer->resize(w, h);
}

void SceneWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_internalTimerID) {
        vpvl::Scene *scene = m_renderer->scene();
        scene->updateModelView(0);
        scene->updateProjection(0);
        if (m_playing) {
            scene->advanceMotion(0.5f);
            updateFPS();
        }
        updateGL();
    }
}

void SceneWidget::wheelEvent(QWheelEvent *event)
{
    zoom(event->delta() > 0, event->modifiers());
}

void SceneWidget::drawBones()
{
    if (m_visibleBones)
        m_renderer->drawModelBones(true, true);
    m_renderer->drawBoneTransform(m_bone);
}

void SceneWidget::updateFPS()
{
    if (m_timer.elapsed() > 1000) {
        m_currentFPS = m_frameCount;
        m_frameCount = 0;
        m_timer.restart();
        emit fpsDidUpdate(m_currentFPS);
    }
    m_frameCount++;
}

QProgressDialog *SceneWidget::getProgressDialog(const QString &label, int max)
{
    QProgressDialog *progress = new QProgressDialog(label, tr("Cancel"), 0, max, this);
    progress->setMinimumDuration(0);
    progress->setValue(0);
    progress->setWindowModality(Qt::WindowModal);
    return progress;
}

const QString SceneWidget::openFileDialog(const QString &name, const QString &desc, const QString &exts)
{
    const QString path = m_settings->value(name).toString();
    const QString fileName = QFileDialog::getOpenFileName(this, desc, path, exts);
    if (!fileName.isEmpty()) {
        QDir dir(fileName);
        dir.cdUp();
        m_settings->setValue(name, dir.absolutePath());
    }
    return fileName;
}

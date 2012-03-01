/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

/* for GLEW limitation, include vpvl.h first to define VPVL_LINK_GLEW except Darwin */
#include <vpvl/vpvl.h>

#include "SceneWidget.h"

#include "Application.h"
#include "DebugDrawer.h"
#include "Grid.h"
#include "Handles.h"
#include "InfoPanel.h"
#include "SceneLoader.h"
#include "World.h"
#include "util.h"

#include <QtGui/QtGui>
#include <btBulletDynamicsCommon.h>

#ifdef Q_OS_DARWIN
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

using namespace vpvl;

#ifdef VPVL_ENABLE_GLSL
#include <vpvl/gl2/Renderer.h>
using namespace vpvl::gl2;
#else
#include <vpvl/gl/Renderer.h>
using namespace vpvl::gl;
#endif /* VPVL_ENABLE_GLSL */
using namespace internal;

namespace {

#ifdef GL_MULTISAMPLE
static inline void UIEnableMultisample()
{
    glEnable(GL_MULTISAMPLE);
}
#else
#define UIEnableMultisample() (void) 0
#endif

}

SceneWidget::SceneWidget(QSettings *settings, QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_world(0),
    m_loader(0),
    m_debugDrawer(0),
    m_grid(0),
    m_info(0),
    m_handles(0),
    m_settings(settings),
    m_editMode(kSelect),
    m_lastDistance(0.0f),
    m_prevElapsed(0.0f),
    m_frameIndex(0.0f),
    m_frameCount(0),
    m_currentFPS(0),
    m_interval(1000.0f / Scene::kFPS),
    m_internalTimerID(0),
    m_handleFlags(0),
    m_playing(false),
    m_enableBoneMove(false),
    m_enableBoneRotate(false),
    m_showModelDialog(false),
    m_lockTouchEvent(false),
    m_enableMoveGesture(false),
    m_enableRotateGesture(false),
    m_enableScaleGesture(false),
    m_enableUndoGesture(false)
{
    m_grid = new Grid();
    m_world = new World(Scene::kFPS);
    connect(static_cast<Application *>(qApp), SIGNAL(fileDidRequest(QString)), this, SLOT(loadFile(QString)));
    setShowModelDialog(m_settings->value("sceneWidget/showModelDialog", true).toBool());
    setMoveGestureEnable(m_settings->value("sceneWidget/enableMoveGesture", false).toBool());
    setRotateGestureEnable(m_settings->value("sceneWidget/enableRotateGesture", true).toBool());
    setScaleGestureEnable(m_settings->value("sceneWidget/enableScaleGesture", true).toBool());
    setUndoGestureEnable(m_settings->value("sceneWidget/enableUndoGesture", true).toBool());
    setAcceptDrops(true);
    setAutoFillBackground(false);
    setMinimumSize(540, 480);
    /* 通常はマウスを動かしても mouseMove が呼ばれないため、マウスが動いたら常時 mouseEvent を呼ぶようにする */
    setMouseTracking(true);
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
}

SceneWidget::~SceneWidget()
{
    delete m_handles;
    m_handles = 0;
    delete m_info;
    m_info = 0;
    delete m_grid;
    m_grid = 0;
    delete m_world;
    m_world = 0;
}

SceneLoader *SceneWidget::sceneLoader() const
{
    return m_loader;
}

void SceneWidget::play()
{
    m_playing = true;
    m_timer.restart();
    emit sceneDidPlay();
}

void SceneWidget::pause()
{
    m_playing = false;
    m_info->setFPS(0);
    m_info->update();
    emit sceneDidPause();
}

void SceneWidget::stop()
{
    m_playing = false;
    m_info->setFPS(0);
    m_info->update();
    m_loader->renderEngine()->scene()->resetMotion();
    emit sceneDidStop();
}

void SceneWidget::clear()
{
    stop();
    setSelectedModel(0);
    m_loader->release();
    m_loader->createProject();
}

void SceneWidget::startAutomaticRendering()
{
    m_internalTimerID = startTimer(m_interval);
}

void SceneWidget::stopAutomaticRendering()
{
    killTimer(m_internalTimerID);
    m_internalTimerID = 0;
}

void SceneWidget::startPhysicsSimulation()
{
    /* 物理暴走を防ぐために少し進めてから開始する */
    if (m_loader && m_loader->isPhysicsEnabled()) {
        btDiscreteDynamicsWorld *world = m_world->mutableWorld();
        m_loader->renderEngine()->scene()->setWorld(world);
        world->stepSimulation(1, 60);
    }
}

void SceneWidget::stopPhysicsSimulation()
{
    Scene *scene = m_loader->renderEngine()->scene();
    scene->setWorld(0);
    const Array<PMDModel *> &models = scene->getRenderingOrder();
    const int nmodels = models.count();
    for (int i = 0; i < nmodels; i++) {
        PMDModel *model = models[i];
        model->resetAllBones();
    }
    updateMotion();
}

void SceneWidget::loadProject(const QString &filename)
{
    QProgressDialog *dialog = new QProgressDialog();
    connect(m_loader, SIGNAL(projectDidLoad(bool)), dialog, SLOT(close()));
    connect(m_loader, SIGNAL(projectDidCount(int)), dialog, SLOT(setMaximum(int)));
    connect(m_loader, SIGNAL(projectDidProceed(int)), dialog, SLOT(setValue(int)));
    dialog->setLabelText(tr("Loading a project %1...").arg(QFileInfo(filename).fileName()));
    dialog->setWindowModality(Qt::WindowModal);
    dialog->setCancelButton(0);
    m_loader->loadProject(filename);
    m_world->setGravity(m_loader->worldGravity());
    delete dialog;
}

void SceneWidget::setWorldGravity(const vpvl::Vector3 &value)
{
    m_world->setGravity(value);
    m_loader->setWorldGravity(value);
}

void SceneWidget::saveProject(const QString &filename)
{
    m_loader->setWorldGravity(m_world->gravity());
    m_loader->saveProject(filename);
}

void SceneWidget::setPreferredFPS(int value)
{
    /* 一旦前のタイマーを止めてから新しい FPS に基づく間隔でタイマーを開始する */
    if (value > 0) {
        m_interval = 1000.0f / value;
        m_world->setPreferredFPS(value);
        m_loader->renderEngine()->scene()->setPreferredFPS(value);
        if (m_internalTimerID) {
            stopAutomaticRendering();
            startAutomaticRendering();
        }
    }
}

void SceneWidget::setSelectedModel(PMDModel *value)
{
    /* 情報パネルに選択されたモデルの名前を更新する */
    m_loader->setSelectedModel(value);
    m_info->setModel(value);
    m_info->update();
}

void SceneWidget::setModelEdgeOffset(double value)
{
    if (PMDModel *model = m_loader->selectedModel())
        m_loader->setModelEdgeOffset(model, static_cast<float>(value));
    updateMotion();
}

void SceneWidget::setModelEdgeColor(const QColor &color)
{
    if (PMDModel *model = m_loader->selectedModel())
        m_loader->setModelEdgeColor(model, color);
    updateMotion();
}

void SceneWidget::setModelProjectiveShadowEnable(bool value)
{
    if (PMDModel *model = m_loader->selectedModel())
        m_loader->setProjectiveShadowEnable(model, value);
    updateMotion();
}

void SceneWidget::setHandlesVisible(bool value)
{
    m_handles->setVisible(value);
}

void SceneWidget::setInfoPanelVisible(bool value)
{
    m_info->setVisible(value);
}

void SceneWidget::setBoneWireFramesVisible(bool value)
{
    m_debugDrawer->setVisible(value);
}

void SceneWidget::addModel()
{
    /* モデル追加と共に空のモーションを作成する */
    PMDModel *model = addModel(openFileDialog("sceneWidget/lastModelDirectory",
                                              tr("Open PMD file"),
                                              tr("PMD file (*.pmd)"),
                                              m_settings));
    if (model && !m_playing) {
        setEmptyMotion(model);
        model->advanceMotion(0.0f);
        emit newMotionDidSet(model);
    }
}

PMDModel *SceneWidget::addModel(const QString &path, bool skipDialog)
{
    QFileInfo fi(path);
    PMDModel *model = 0;
    if (fi.exists()) {
        const QDir &dir = fi.dir();
        const QString &base = fi.fileName();
        model = m_loader->loadModel(base, dir);
        if (model) {
            if (skipDialog || (!m_showModelDialog || acceptAddingModel(model))) {
                QUuid uuid;
                m_loader->addModel(model, base, dir, uuid);
                emit fileDidLoad(path);
            }
            else {
                delete model;
                model = 0;
            }
        }
        else {
            QMessageBox::warning(this, tr("Loading model error"),
                                 tr("%1 cannot be loaded").arg(fi.fileName()));
        }
    }
    return model;
}

void SceneWidget::insertMotionToAllModels()
{
    /* モーションを追加したら即座に反映させるために advanceMotion(0.0f) を呼んでおく */
    VMDMotion *motion = insertMotionToAllModels(openFileDialog("sceneWidget/lastModelMotionDirectory",
                                                               tr("Open VMD (for model) file"),
                                                               tr("VMD file (*.vmd)"),
                                                               m_settings));
    PMDModel *selected = m_loader->selectedModel();
    if (motion && selected)
        selected->advanceMotion(0.0f);
}

VMDMotion *SceneWidget::insertMotionToAllModels(const QString &path)
{
    VMDMotion *motion = 0;
    if (QFile::exists(path)) {
        QList<PMDModel *> models;
        motion = m_loader->loadModelMotion(path, models);
        if (motion) {
            emit fileDidLoad(path);
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
    PMDModel *model = m_loader->selectedModel();
    if (model) {
        VMDMotion *motion = insertMotionToSelectedModel(openFileDialog("sceneWidget/lastModelMotionDirectory",
                                                                       tr("Open VMD (for model) file"),
                                                                       tr("VMD file (*.vmd)"),
                                                                       m_settings));
        if (motion)
            advanceMotion(0.0f);
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."),
                             tr("Select a model to insert the motion (\"Model\" > \"Select model\")"));
    }
}

VMDMotion *SceneWidget::insertMotionToSelectedModel(const QString &path)
{
    return insertMotionToModel(path, m_loader->selectedModel());
}

VMDMotion *SceneWidget::insertMotionToModel(const QString &path, PMDModel *model)
{
    VMDMotion *motion = 0;
    if (model) {
        if (QFile::exists(path)) {
            motion = m_loader->loadModelMotion(path, model);
            if (motion) {
                emit fileDidLoad(path);
            }
            else {
                QMessageBox::warning(this, tr("Loading model motion error"),
                                     tr("%1 cannot be loaded").arg(QFileInfo(path).fileName()));
            }
        }
    }
    return motion;
}

void SceneWidget::setEmptyMotion()
{
    setEmptyMotion(m_loader->selectedModel());
}

void SceneWidget::setEmptyMotion(PMDModel *model)
{
    if (model) {
        VMDMotion *modelMotion = m_loader->newModelMotion(model);
        m_loader->setModelMotion(modelMotion, model);
        VMDMotion *cameraMotion = m_loader->newCameraMotion();
        m_loader->setCameraMotion(cameraMotion);
    }
    else
        QMessageBox::warning(this,
                             tr("The model is not selected."),
                             tr("Select a model to insert the motion (\"Model\" > \"Select model\")"));
}

void SceneWidget::addAsset()
{
    addAsset(openFileDialog("sceneWidget/lastAssetDirectory",
                            tr("Open X file"),
                            tr("DirectX mesh file (*.x)"),
                            m_settings));
}

Asset *SceneWidget::addAsset(const QString &path)
{
    QFileInfo fi(path);
    Asset *asset = 0;
    if (fi.exists()) {
        QUuid uuid;
        asset = m_loader->loadAsset(fi.fileName(), fi.dir(), uuid);
        if (asset) {
            emit fileDidLoad(path);
        }
        else {
            QMessageBox::warning(this, tr("Loading asset error"),
                                 tr("%1 cannot be loaded").arg(fi.fileName()));
        }
    }
    return asset;
}


void SceneWidget::addAssetFromMetadata()
{
    addAssetFromMetadata(openFileDialog("sceneWidget/lastAssetDirectory",
                                        tr("Open VAC file"),
                                        tr("MMD accessory metadata (*.vac)"),
                                        m_settings));
}

Asset *SceneWidget::addAssetFromMetadata(const QString &path)
{
    QFileInfo fi(path);
    Asset *asset = 0;
    if (fi.exists()) {
        QUuid uuid;
        asset = m_loader->loadAssetFromMetadata(fi.fileName(), fi.dir(), uuid);
        if (asset) {
            setFocus();
        }
        else {
            QMessageBox::warning(this, tr("Loading asset error"),
                                 tr("%1 cannot be loaded").arg(fi.fileName()));
        }
    }
    return asset;
}

void SceneWidget::saveMetadataFromAsset(Asset *asset)
{
    if (asset) {
        QString filename = QFileDialog::getSaveFileName(this, tr("Save %1 as VAC file")
                                                        .arg(internal::toQString(asset)), "",
                                                        tr("MMD accessory metadata (*.vac)"));
        m_loader->saveMetadataFromAsset(filename, asset);
    }
}

void SceneWidget::insertPoseToSelectedModel()
{
    PMDModel *model = m_loader->selectedModel();
    VPDFile *pose = insertPoseToSelectedModel(openFileDialog("sceneWidget/lastPoseDirectory",
                                                             tr("Open VPD file"),
                                                             tr("VPD file (*.vpd)"),
                                                             m_settings),
                                              model);
    if (pose && model)
        model->updateImmediate();
}

VPDFile *SceneWidget::insertPoseToSelectedModel(const QString &filename, PMDModel *model)
{
    VPDFile *pose = 0;
    if (model) {
        if (QFile::exists(filename)) {
            pose = m_loader->loadModelPose(filename, model);
            if (!pose) {
                QMessageBox::warning(this, tr("Loading model pose error"),
                                     tr("%1 cannot be loaded").arg(QFileInfo(filename).fileName()));
            }
        }
    }
    else
        QMessageBox::warning(this,
                             tr("The model is not selected."),
                             tr("Select a model to set the pose (\"Model\" > \"Select model\")"));
    return pose;
}

void SceneWidget::advanceMotion(float frameIndex)
{
    if (frameIndex <= 0)
        return;
    Scene *scene = m_loader->renderEngine()->scene();
    scene->updateModelView();
    scene->updateProjection();
    scene->advanceMotion(frameIndex);
    m_loader->renderEngine()->updateAllModel();
    updateGL();
    emit cameraPerspectiveDidSet(scene->cameraPosition(), scene->cameraAngle(), scene->fovy(), scene->cameraDistance());
}

void SceneWidget::seekMotion(float frameIndex, bool force)
{
    /* advanceMotion に似ているが、前のフレームインデックスを利用することがあるので、保存しておく必要がある */
    Renderer *renderEngine = m_loader->renderEngine();
    Scene *scene = renderEngine->scene();
    scene->updateModelView();
    scene->updateProjection();
    /* 同じフレームインデックスにシークする場合はカメラと照明は動かさないようにする。force で強制的に動かすことが出来る */
    if (m_frameIndex == frameIndex && !force) {
        VMDMotion *cameraMotion = scene->cameraMotion();
        VMDMotion *lightMotion = scene->lightMotion();
        scene->setCameraMotion(0);
        scene->setLightMotion(0);
        scene->seekMotion(frameIndex);
        scene->setCameraMotion(cameraMotion);
        scene->setLightMotion(lightMotion);
    }
    else {
        scene->seekMotion(frameIndex);
        m_frameIndex = frameIndex;
    }
    renderEngine->updateAllModel();
    updateGL();
    emit cameraPerspectiveDidSet(scene->cameraPosition(), scene->cameraAngle(), scene->fovy(), scene->cameraDistance());
    emit motionDidSeek(frameIndex);
}

void SceneWidget::resetMotion()
{
    /* resetMotion のラッパー */
    Renderer *renderEngine = m_loader->renderEngine();
    Scene *scene = renderEngine->scene();
    scene->resetMotion();
    scene->updateModelView();
    scene->updateProjection();
    renderEngine->updateAllModel();
    m_frameIndex = 0;
    updateGL();
    emit cameraPerspectiveDidSet(scene->cameraPosition(), scene->cameraAngle(), scene->fovy(), scene->cameraDistance());
    emit motionDidSeek(0.0f);
}

void SceneWidget::setCamera()
{
    VMDMotion *motion = setCamera(openFileDialog("sceneWidget/lastCameraMotionDirectory",
                                                 tr("Open VMD (for camera) file"),
                                                 tr("VMD file (*.vmd)"),
                                                 m_settings));
    if (motion)
        advanceMotion(0.0f);
}

VMDMotion *SceneWidget::setCamera(const QString &path)
{
    VMDMotion *motion = 0;
    if (QFile::exists(path)) {
        motion = m_loader->loadCameraMotion(path);
        if (motion) {
            emit fileDidLoad(path);
        }
        else {
            QMessageBox::warning(this, tr("Loading camera motion error"),
                                 tr("%1 cannot be loaded").arg(QFileInfo(path).fileName()));
        }
    }
    return motion;
}

void SceneWidget::deleteSelectedModel()
{
    PMDModel *selected = m_loader->selectedModel();
    if (selected) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this,
                                   qAppName(),
                                   tr("Do you want to delete the model \"%1\"? This cannot undo.")
                                   .arg(internal::toQString(selected)),
                                   QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes)
            m_loader->deleteModel(selected);
    }
}

void SceneWidget::resetCamera()
{
    Scene *scene = m_loader->renderEngine()->scene();
    scene->resetCamera();
    emit cameraPerspectiveDidSet(scene->cameraPosition(), scene->cameraAngle(), scene->fovy(), scene->cameraDistance());
}

void SceneWidget::setLightColor(const Color &color)
{
    Scene *scene = m_loader->renderEngine()->scene();
    scene->setLightSource(color, scene->lightPosition());
    emit lightColorDidSet(color);
}

void SceneWidget::setLightPosition(const Vector3 &position)
{
    Scene *scene = m_loader->renderEngine()->scene();
    scene->setLightSource(scene->lightColor(), position);
    emit lightPositionDidSet(position);
}

void SceneWidget::setCameraPerspective(Vector3 *pos, Vector3 *angle, float *fovy, float *distance)
{
    /* 変更しないことを示す NULL かどうかを判定するために引数をポインタに設定している */
    Scene *scene = m_loader->renderEngine()->scene();
    Vector3 posValue, angleValue;
    float fovyValue, distanceValue;
    posValue = !pos ? scene->cameraPosition() : *pos;
    angleValue = !angle ? scene->cameraAngle() : *angle;
    fovyValue = !fovy ? scene->fovy() : *fovy;
    distanceValue = !distance ? scene->cameraDistance() : *distance;
    scene->setCameraPerspective(posValue, angleValue, fovyValue, distanceValue);
    emit cameraPerspectiveDidSet(posValue, angleValue, fovyValue, distanceValue);
}

void SceneWidget::makeRay(const QPointF &input, Vector3 &rayFrom, Vector3 &rayTo) const
{
    // This implementation based on the below page.
    // http://softwareprodigy.blogspot.com/2009/08/gluunproject-for-iphone-opengl-es.html
    Scene *scene = m_loader->renderEngine()->scene();
    float modelviewMatrixf[16], projectionMatrixf[16];
    GLdouble modelviewMatrixd[16], projectionMatrixd[16];
    const GLint viewport[4] = { 0, 0, width(), height() };
    scene->getModelViewMatrix(modelviewMatrixf);
    scene->getProjectionMatrix(projectionMatrixf);
    for (int i = 0; i < 16; i++) {
        modelviewMatrixd[i] = modelviewMatrixf[i];
        projectionMatrixd[i] = projectionMatrixf[i];
    }
    GLdouble wx = input.x(), wy = height() - input.y(), cx, cy, cz, fx, fy, fz;
    gluUnProject(wx, wy, 0, modelviewMatrixd, projectionMatrixd, viewport, &cx, &cy, &cz);
    gluUnProject(wx, wy, 1, modelviewMatrixd, projectionMatrixd, viewport, &fx, &fy, &fz);
    rayFrom.setValue(cx, cy, cz);
    rayTo.setValue(fx, fy, fz);
}

void SceneWidget::selectBones(const QList<Bone *> &bones)
{
    m_info->setBones(bones, tr("(multiple)"));
    m_info->update();
    m_handles->setBone(bones.isEmpty() ? 0 : bones.first());
    m_bones = bones;
}

void SceneWidget::rotateScene(const Vector3 &delta)
{
    Scene *scene = m_loader->renderEngine()->scene();
    Vector3 pos = scene->cameraPosition(), angle = scene->cameraAngle();
    float fovy = scene->fovy(), distance = scene->cameraDistance();
    angle += delta;
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::rotateModel(const Quaternion &delta)
{
    rotateModel(m_loader->selectedModel(), delta);
}

void SceneWidget::rotateModel(PMDModel *model, const Quaternion &delta)
{
    if (model) {
        const Quaternion &rotation = model->rotationOffset();
        model->setRotationOffset(rotation * delta);
        model->updateImmediate();
        emit modelDidRotate(rotation);
    }
}

void SceneWidget::translateScene(const Vector3 &delta)
{
    // FIXME: direction
    Scene *scene = m_loader->renderEngine()->scene();
    Vector3 pos = scene->cameraPosition(), angle = scene->cameraAngle();
    float fovy = scene->fovy(), distance = scene->cameraDistance();
    pos += delta;
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::translateModel(const Vector3 &delta)
{
    translateModel(m_loader->selectedModel(), delta);
}

void SceneWidget::translateModel(PMDModel *model, const Vector3 &delta)
{
    // FIXME: direction
    if (model) {
        const Vector3 &position = model->positionOffset();
        model->setPositionOffset(position + delta);
        model->updateImmediate();
        emit modelDidMove(position);
    }
}

void SceneWidget::resetModelPosition()
{
    PMDModel *model = m_loader->selectedModel();
    if (model) {
        const Vector3 &position = model->positionOffset();
        model->setPositionOffset(Vector3(0.0f, 0.0f, 0.0f));
        model->updateImmediate();
        emit modelDidMove(position);
    }
}

void SceneWidget::loadFile(const QString &file)
{
    /* モデルファイル */
    if (file.endsWith(".pmd", Qt::CaseInsensitive)) {
        PMDModel *model = addModel(file);
        if (model && !m_playing) {
            setEmptyMotion(model);
            model->advanceMotion(0.0f);
            emit newMotionDidSet(model);
        }
    }
    /* モーションファイル */
    else if (file.endsWith(".vmd", Qt::CaseInsensitive)) {
        VMDMotion *motion = insertMotionToModel(file, m_loader->selectedModel());
        if (motion)
            advanceMotion(0.0f);
    }
    /* アクセサリファイル */
    else if (file.endsWith(".x", Qt::CaseInsensitive)) {
        addAsset(file);
    }
    /* ポーズファイル */
    else if (file.endsWith(".vpd", Qt::CaseInsensitive)) {
        PMDModel *model = m_loader->selectedModel();
        VPDFile *pose = insertPoseToSelectedModel(file, model);
        if (pose && model)
            model->updateImmediate();
    }
    /* アクセサリ情報ファイル */
    else if (file.endsWith(".vac", Qt::CaseInsensitive)) {
        addAssetFromMetadata(file);
    }
}

void SceneWidget::setEditMode(SceneWidget::EditMode value)
{
    switch (value) {
    case kRotate:
        m_handles->setVisibilityFlags(Handles::kVisibleRotate);
        break;
    case kMove:
        m_handles->setVisibilityFlags(Handles::kVisibleMove);
        break;
    default:
        m_handles->setVisibilityFlags(Handles::kNone);
        break;
    }
    m_editMode = value;
}

void SceneWidget::zoom(bool up, const Qt::KeyboardModifiers &modifiers)
{
    Scene *scene = m_loader->renderEngine()->scene();
    Vector3 pos = scene->cameraPosition(), angle = scene->cameraAngle();
    float fovy = scene->fovy(), distance = scene->cameraDistance();
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

bool SceneWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture && !m_lockTouchEvent)
        return gestureEvent(static_cast<QGestureEvent *>(event));
    return QGLWidget::event(event);
}

void SceneWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("sceneWidget/showModelDialog", showModelDialog());
    m_settings->setValue("sceneWidget/enableMoveGesture", isMoveGestureEnabled());
    m_settings->setValue("sceneWidget/enableRotateGesture", isRotateGestureEnabled());
    m_settings->setValue("sceneWidget/enableScaleGesture", isScaleGestureEnabled());
    m_settings->setValue("sceneWidget/enableUndoGesture", isUndoGestureEnabled());
    stopAutomaticRendering();
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
        const QList<QUrl> &urls = mimeData->urls();
        foreach (const QUrl url, urls) {
            const QString &file = url.toLocalFile();
            loadFile(file);
            qDebug() << "Proceeded a dropped file:" << file;
        }
    }
}

void SceneWidget::initializeGL()
{
    initializeGLFunctions(context());
    qDebug("VPVL version: %s (%d)", VPVL_VERSION_STRING, VPVL_VERSION);
    qDebug("GL_VERSION: %s", glGetString(GL_VERSION));
    qDebug("GL_VENDOR: %s", glGetString(GL_VENDOR));
    qDebug("GL_RENDERER: %s", glGetString(GL_RENDERER));
    UIEnableMultisample();
    /* OpenGL の初期化が最低条件なため、Renderer はここでインスタンスを作成する */
    const QSize &s = size();
    m_loader = new SceneLoader(s.width(), s.height(), Scene::kFPS);
    m_handles = new Handles(m_loader, s);
    m_info = new InfoPanel(s);
    m_debugDrawer = new DebugDrawer(m_loader->renderEngine()->scene());
    m_debugDrawer->setWorld(m_world->mutableWorld());
    /* OpenGL を利用するため、格子状フィールドの初期化もここで行う */
    m_grid->load();
    Scene *scene = m_loader->renderEngine()->scene();
    /* 物理演算に必要な World が initializeGL でインスタンスを生成するため、setPhysicsEnable はここで有効にする */
    //if (m_playing || m_enablePhysics)
    //    setPhysicsEnable(true);
    /* テクスチャ情報を必要とするため、ハンドルのリソースの読み込みはここで行う */
    m_handles->load();
    /* 動的なテクスチャ作成を行うため、情報パネルのリソースの読み込みも個々で行った上で初期設定を行う */
    m_info->load();
    m_info->setModel(0);
    m_info->setBones(QList<Bone *>(), "");
    m_info->setFPS(0.0f);
    m_info->update();
    m_debugDrawer->initialize();
    m_loader->renderEngine()->initializeSurface();
    m_timer.start();
    startAutomaticRendering();
    emit cameraPerspectiveDidSet(scene->cameraPosition(), scene->cameraAngle(), scene->fovy(), scene->cameraDistance());
    emit initailizeGLContextDidDone();
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
    const QPointF &pos = event->posF();
    QRectF rect;
    int flags;
    m_lockTouchEvent = true;
    m_handles->setPoint2D(pos);
    /* モデルのハンドルと重なるケースを考慮して右下のハンドルを優先的に処理する */
    bool movable = false, rotateable = false;
    if (m_bones.count() == 1) {
        vpvl::Bone *bone = m_bones.first();
        movable = bone->isMovable();
        rotateable = bone->isRotateable();
    }
    if (m_handles->testHitImage(pos, movable, rotateable, flags, rect)) {
        switch (flags) {
        /*
         * ローカルとグローバルの切り替えなので、値を反転して設定する必要がある
         * また、ローカルとグローバル、移動回転ハンドルはそれぞれフラグ値は排他的
         */
        case Handles::kLocal:
            m_handles->setLocal(false);
            break;
        case Handles::kGlobal:
            m_handles->setLocal(true);
            break;
        default:
            setCursor(Qt::SizeVerCursor);
            break;
        }
        m_handleFlags = flags;
        emit handleDidGrab();
        return;
    }
    if (PMDModel *model = m_loader->selectedModel()) {
        if (m_editMode == kSelect) {
            static const Vector3 size(0.1f, 0.1f, 0.1f);
            const QPointF &pos = event->posF();
            Vector3 znear, zfar, normal;
            makeRay(pos, znear, zfar);
            const BoneList &bones = model->bones();
            const int nbones = bones.count();
            Bone *nearestBone = 0;
            Scalar hitLambda = 1.0f;
            for (int i = 0; i < nbones; i++) {
                Bone *bone = bones[i];
                const Vector3 &o = bone->localTransform().getOrigin(),
                        min = o - size, max = o + size;
                if (btRayAabb(znear, zfar, min, max, hitLambda, normal)) {
                    nearestBone = bone;
                    break;
                }
            }
            if (nearestBone && (nearestBone->isMovable() || nearestBone->isRotateable())) {
                QList<Bone *> bones;
                bones.append(nearestBone);
                emit boneDidSelect(bones);
            }
        }
        else if (m_editMode == kRotate || m_editMode == kMove) {
            Vector3 rayFrom, rayTo, pick;
            makeRay(pos, rayFrom, rayTo);
            if (m_handles->testHitModel(rayFrom, rayTo, false, flags, pick)) {
                m_handleFlags = flags;
                setCursor(Qt::ClosedHandCursor);
                emit handleDidGrab();
            }
        }
    }
}

void SceneWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF &pos = event->posF();
    if (event->buttons() & Qt::LeftButton) {
        const Qt::KeyboardModifiers modifiers = event->modifiers();
        const QPointF &diff = m_handles->diffPoint2D(pos);
        /* モデルのハンドルがクリックされた */
        if (m_handleFlags & Handles::kView)
            grabModelHandleByRaycast(pos, diff, m_handleFlags);
        /* 有効な右下のハンドルがクリックされた */
        else if (m_handleFlags & Handles::kEnable)
            grabImageHandle(diff);
        /* 光源移動 */
        else if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            Scene *scene = m_loader->renderEngine()->scene();
            Vector3 position = scene->lightPosition();
            Quaternion rx(0.0f, diff.y() * radian(0.1f), 0.0f),
                    ry(0.0f, diff.x() * radian(0.1f), 0.0f);
            position = position * btMatrix3x3(rx * ry);
            scene->setLightSource(scene->lightColor(), position);
        }
        /* 場面の移動 */
        else if (modifiers & Qt::ShiftModifier) {
            translateScene(Vector3(diff.x() * -0.1f, diff.y() * 0.1f, 0.0f));
        }
        /* 場面の回転 (X と Y が逆転している点に注意) */
        else {
            rotateScene(Vector3(diff.y() * 0.5f, diff.x() * 0.5f, 0.0f));
        }
        m_handles->setPoint2D(event->posF());
        return;
    }
    changeCursorIfHandlesHit(pos);
}

void SceneWidget::mouseReleaseEvent(QMouseEvent *event)
{
    changeCursorIfHandlesHit(event->posF());
    setEditMode(m_editMode);
    m_handleFlags = Handles::kNone;
    m_handles->setAngle(0.0f);
    m_handles->setPoint3D(Vector3(0.0f, 0.0f, 0.0f));
    m_lockTouchEvent = false;
    emit handleDidRelease();
}

void SceneWidget::paintGL()
{
    qglClearColor(m_loader->isBlackBackgroundEnabled() ? Qt::black : Qt::white);
    UIEnableMultisample();
    Renderer *renderEngine = m_loader->renderEngine();
    renderEngine->clear();
    {
        glCullFace(GL_FRONT);
        Renderer *renderEngine = m_loader->renderEngine();
        const Array<PMDModel *> &models = renderEngine->scene()->getRenderingOrder();
        const int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            PMDModel *model = models[i];
            if (m_loader->isProjectiveShadowEnabled(model))
                renderEngine->renderModelShadow(model);
        }
        glCullFace(GL_BACK);
    }
    renderEngine->renderAllModels();
    renderEngine->renderAllAssets();
    /* FIXME: rendering order should be drawn after drawing handles */
    vpvl::Bone *bone = 0;
    if (m_bones.count() == 1)
        bone = m_bones.first();
    m_grid->draw(renderEngine->scene(), m_loader->isGridVisible());
    if (bone)
        m_handles->drawImageHandles(bone->isMovable(), bone->isRotateable());
    else
        m_handles->drawImageHandles(false, false);
    m_info->draw();
    switch (m_editMode) {
    case kSelect:
        m_debugDrawer->drawModelBones(m_loader->selectedModel(), selectedBones());
        m_debugDrawer->drawBoneTransform(bone);
        break;
    case kRotate:
        m_handles->drawRotationHandle();
        break;
    case kMove:
        m_handles->drawMoveHandle();
        break;
    }
}

void SceneWidget::resizeGL(int w, int h)
{
    m_loader->renderEngine()->resize(w, h);
    m_handles->resize(w, h);
    m_info->resize(w, h);
}

void SceneWidget::timerEvent(QTimerEvent *event)
{
    /* タイマーが生きている => 描写命令を出す */
    if (event->timerId() == m_internalTimerID) {
        Renderer *renderEngine = m_loader->renderEngine();
        Scene *scene = renderEngine->scene();
        scene->updateModelView();
        scene->updateProjection();
        renderEngine->updateAllModel();
        if (m_playing) {
            /* タイマーの仕様上一定ではないため、差分をここで吸収する */
            float elapsed = m_timer.elapsed() / static_cast<float>(Scene::kFPS);
            float diff = elapsed - m_prevElapsed;
            m_prevElapsed = elapsed;
            if (diff < 0)
                diff = elapsed;
            scene->advanceMotion(diff);
            updateFPS();
        }
        updateGL();
    }
}

void SceneWidget::wheelEvent(QWheelEvent *event)
{
    zoom(event->delta() > 0, event->modifiers());
}

bool SceneWidget::gestureEvent(QGestureEvent *event)
{
    if (QGesture *swipe = event->gesture(Qt::SwipeGesture))
        swipeTriggered(static_cast<QSwipeGesture *>(swipe));
    else if (QGesture *pan = event->gesture(Qt::PanGesture))
        panTriggered(static_cast<QPanGesture *>(pan));
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
    return true;
}

void SceneWidget::panTriggered(QPanGesture *event)
{
    if (!m_enableMoveGesture)
        return;
    const Qt::GestureState state = event->state();
    switch (state) {
    case Qt::GestureStarted:
    case Qt::GestureUpdated:
        setCursor(Qt::SizeAllCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
    const QPointF &delta = event->delta();
    const Vector3 newDelta(delta.x() * -0.25, delta.y() * 0.25, 0.0f);
    if (!m_bones.isEmpty()) {
        vpvl::Bone *bone = m_bones.last();
        switch (state) {
        case Qt::GestureStarted:
            emit handleDidGrab();
            break;
        case Qt::GestureUpdated:
            emit handleDidMove(newDelta, bone, 'V');
            break;
        case Qt::GestureFinished:
            emit handleDidRelease();
            break;
        default:
            break;
        }
    }
    else if (PMDModel *model = m_loader->selectedModel())
        translateModel(model, newDelta);
    else
        translateScene(newDelta);
}

void SceneWidget::pinchTriggered(QPinchGesture *event)
{
    const Qt::GestureState state = event->state();
    QPinchGesture::ChangeFlags flags = event->changeFlags();
    Scene *scene = m_loader->renderEngine()->scene();
    const Vector3 &pos = scene->cameraPosition(), &angle = scene->cameraAngle();
    float distance = scene->cameraDistance(), fovy = scene->fovy();
    if (m_enableRotateGesture && flags & QPinchGesture::RotationAngleChanged) {
        qreal value = event->rotationAngle() - event->lastRotationAngle();
        Scalar radian = vpvl::radian(value);
        Quaternion rotation(0.0f, 0.0f, 0.0f, 1.0f);
        /* 四元数を使う場合回転が時計回りになるよう符号を反転させる必要がある */
        if (!m_bones.isEmpty()) {
            vpvl::Bone *bone = m_bones.last();
            rotation.setEulerZYX(0.0f, -radian, 0.0f);
            switch (state) {
            case Qt::GestureStarted:
                emit handleDidGrab();
                break;
            case Qt::GestureUpdated:
                emit handleDidRotate(rotation, bone, 'V', float(value));
                break;
            case Qt::GestureFinished:
                emit handleDidRelease();
                break;
            default:
                break;
            }
        }
        else if (PMDModel *model = m_loader->selectedModel()) {
            rotation.setEulerZYX(0.0f, -radian, 0.0f);
            rotateModel(model, rotation);
        }
        else {
            rotateScene(Vector3(0.0f, value, 0.0f));
        }
    }
    qreal scaleFactor = 1.0;
    if (state == Qt::GestureStarted)
        m_lastDistance = distance;
    if (m_enableScaleGesture && flags & QPinchGesture::ScaleFactorChanged) {
        scaleFactor = event->scaleFactor();
        distance = m_lastDistance * scaleFactor;
    }
    if (scaleFactor != 1.0) {
        scene->setCameraPerspective(pos, angle, fovy, distance);
        emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
    }
}

void SceneWidget::swipeTriggered(QSwipeGesture *event)
{
    if (m_enableUndoGesture && event->state() == Qt::GestureFinished) {
        const QSwipeGesture::SwipeDirection hdir = event->horizontalDirection();
        const QSwipeGesture::SwipeDirection vdir = event->verticalDirection();
        if (hdir == QSwipeGesture::Left || vdir == QSwipeGesture::Up) {
            emit undoDidRequest();
        }
        else if (hdir == QSwipeGesture::Right || vdir == QSwipeGesture::Down) {
            emit redoDidRequest();
        }
    }
}

bool SceneWidget::acceptAddingModel(PMDModel *model)
{
    /* モデルを追加する前にモデルの名前とコメントを出すダイアログを表示 */
    QMessageBox mbox;
    QString comment = internal::toQString(model->comment());
    mbox.setText(tr("Model Information of \"%1\"").arg(internal::toQString(model->name())));
    mbox.setInformativeText(comment.replace("\n", "<br>"));
    mbox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    return mbox.exec() == QMessageBox::Ok;
}

void SceneWidget::updateFPS()
{
    /* 1秒ごとの FPS はここで計算しておく。1秒過ぎたら updateFPS を呼んだ回数を求め、タイマーを再起動させる */
    if (m_timer.elapsed() > 1000) {
        m_currentFPS = m_frameCount;
        m_frameCount = 0;
        m_timer.restart();
        m_info->setFPS(m_currentFPS);
        m_info->update();
        emit fpsDidUpdate(m_currentFPS);
    }
    m_frameCount++;
}

void SceneWidget::changeCursorIfHandlesHit(const QPointF &pos)
{
    int flags;
    QRectF rect;
    /* ハンドルアイコンの中に入っているか? */
    bool movable = false, rotateable = false;
    if (m_bones.count() == 1) {
        vpvl::Bone *bone = m_bones.first();
        movable = bone->isMovable();
        rotateable = bone->isRotateable();
    }
    if (m_handles->testHitImage(pos, movable, rotateable, flags, rect) && flags & Handles::kEnable) {
        setCursor(Qt::SizeVerCursor);
    }
    /* 回転モードの場合は回転ハンドルに入っているか? */
    else if (m_editMode == kRotate || m_editMode == kMove) {
        Vector3 rayFrom, rayTo, pick;
        makeRay(pos, rayFrom, rayTo);
        if (m_handles->testHitModel(rayFrom, rayTo, true, flags, pick))
            setCursor(Qt::OpenHandCursor);
        else
            unsetCursor();
    }
    else {
        unsetCursor();
    }
}

void SceneWidget::grabImageHandle(const QPointF &diff)
{
    int flags = m_handleFlags;
    int mode = m_handles->isLocal() ? 'L' : 'G';
    /* 意図する向きと実際の値が逆なので、反転させる */
    float diffValue = -diff.y();
    /* 移動ハンドルである */
    if (flags & Handles::kMove) {
        const float &value = diffValue * 0.1f;
        Vector3 delta(0.0f, 0.0f, 0.0f);
        if (flags & Handles::kX)
            delta.setX(value);
        else if (flags & Handles::kY)
            delta.setY(value);
        else if (flags & Handles::kZ)
            delta.setZ(value);
        emit handleDidMove(delta, 0, mode);
    }
    /* 回転ハンドルである */
    else if (flags & Handles::kRotate) {
        const float &value = radian(diffValue) * 0.1f;
        Quaternion delta(0.0f, 0.0f, 0.0f, 1.0f);
        int axis = 0;
        if (flags & Handles::kX) {
            delta.setX(value);
            axis = 'X' << 8;
        }
        else if (flags & Handles::kY) {
            delta.setY(value);
            axis = 'Y' << 8;
        }
        else if (flags & Handles::kZ) {
            delta.setZ(-value);
            axis = 'Z' << 8;
        }
        emit handleDidRotate(delta, 0, mode | axis, diffValue);
    }
}

void SceneWidget::grabModelHandleByRaycast(const QPointF &pos, const QPointF &diff, int flags)
{
    int mode = 'V';
    Vector3 rayFrom, rayTo, pick, delta;
    /* モデルのハンドルに当たっている場合のみモデルを動かす */
    if (flags & Handles::kMove) {
        const QPointF &diff2 = diff * 0.1;
        Bone *bone = m_handles->currentBone();
        const Transform &transform = bone->localTransform();
        if (flags & Handles::kX) {
            delta = transform.getBasis() * Vector3(diff2.x(), 0, 0);
        }
        else if (flags & Handles::kY) {
            delta = transform.getBasis() * Vector3(0, diff2.y(), 0);
        }
        else if (flags & Handles::kZ) {
            delta = transform.getBasis() * Vector3(0, 0, diff2.y());
        }
        emit handleDidMove(delta, 0, mode);
    }
    else if (m_handles->testHitModel(rayFrom, rayTo, false, flags, pick)) {
        mode = 'L';
        /* 移動ハンドルである(矢印の先端) */
        /* 回転ハンドルである(ドーナツ) */
        if (flags & Handles::kRotate) {
            const btScalar &angle = m_handles->angle(pick);
            Quaternion delta(0.0f, 0.0f, 0.0f, 1.0f);
            if (!m_handles->isAngleZero()) {
                float diff = m_handles->diffAngle(angle);
                if (flags & Handles::kX)
                    delta.setEulerZYX(0.0f, 0.0f, btFabs(diff));
                else if (flags & Handles::kY)
                    delta.setEulerZYX(0.0f, btFabs(diff), 0.0f);
                else if (flags & Handles::kZ)
                    delta.setEulerZYX(btFabs(diff), 0.0f, 0.0f);
                emit handleDidRotate(delta, 0, mode, diff);
            }
            m_handles->setAngle(angle);
        }
        m_handles->setPoint3D(pick);
    }
}

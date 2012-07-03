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
#include <vpvl2/vpvl2.h>

#include "SceneWidget.h"

#include "Application.h"
#include "BackgroundImage.h"
#include "DebugDrawer.h"
#include "Grid.h"
#include "Handles.h"
#include "InfoPanel.h"
#include "SceneLoader.h"
#include "TextureDrawHelper.h"
#include "World.h"
#include "util.h"

#include <QtGui/QtGui>
#include <btBulletDynamicsCommon.h>

#ifdef Q_OS_DARWIN
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

using namespace vpvl2;
using namespace internal;


class SceneWidget::PlaneWorld {
public:
    PlaneWorld()
        : m_dispatcher(&m_config),
          m_broadphase(-internal::kWorldAabbSize, internal::kWorldAabbSize),
          m_world(&m_dispatcher, &m_broadphase, &m_solver, &m_config),
          m_state(0),
          m_shape(0),
          m_body(0)
    {
        m_state = new btDefaultMotionState();
        m_shape = new btBoxShape(btVector3(internal::kWorldAabbSize.x(), internal::kWorldAabbSize.y(), 0.01));
        btRigidBody::btRigidBodyConstructionInfo info(0, m_state, m_shape);
        m_body = new btRigidBody(info);
        m_world.addRigidBody(m_body);
    }
    ~PlaneWorld()
    {
        m_world.removeRigidBody(m_body);
        delete m_body;
        delete m_shape;
        delete m_state;
    }

    void updateTransform(const Transform &value) {
        m_body->setWorldTransform(value);
        m_world.stepSimulation(1);
    }
    void draw(const SceneLoader *loader, internal::DebugDrawer *drawer) {
        const btTransform &worldTransform = m_body->getWorldTransform();
        m_world.setDebugDrawer(drawer);
        drawer->drawShape(&m_world, m_shape, loader, worldTransform, btVector3(1, 0, 0));
        m_world.setDebugDrawer(0);
    }
    bool test(const Vector3 &from, const Vector3 &to, Vector3 &hit) {
        btCollisionWorld::ClosestRayResultCallback callback(from, to);
        m_world.rayTest(from, to, callback);
        hit.setZero();
        if (callback.hasHit()) {
            hit = callback.m_hitPointWorld;
            return true;
        }
        return false;
    }

private:
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
    btDiscreteDynamicsWorld m_world;
    btDefaultMotionState *m_state;
    btBoxShape *m_shape;
    btRigidBody *m_body;
};

SceneWidget::SceneWidget(IEncoding *encoding, Factory *factory, QSettings *settings, QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_loader(0),
    m_settings(settings),
    m_background(0),
    m_encoding(encoding),
    m_factory(factory),
    m_currentSelectedBone(0),
    m_lastBonePosition(kZeroV3),
    m_totalDelta(0.0f),
    m_debugDrawer(0),
    m_grid(0),
    m_info(0),
    m_plane(0),
    m_handles(0),
    m_editMode(kSelect),
    m_lastDistance(0.0f),
    m_prevElapsed(0.0f),
    m_timeIndex(0.0f),
    m_frameCount(0),
    m_currentFPS(0),
    m_interval(1000.0f / Scene::defaultFPS()),
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
    m_enableUndoGesture(false),
    m_enableUpdateGL(true),
    m_isImageHandleRectIntersect(false)
{
    m_grid = new Grid();
    m_plane = new PlaneWorld();
    connect(static_cast<Application *>(qApp), SIGNAL(fileDidRequest(QString)), this, SLOT(loadFile(QString)));
    connect(this, SIGNAL(cameraPerspectiveDidSet(const vpvl2::Scene::ICamera*)),
            this, SLOT(updatePlaneWorld(const vpvl2::Scene::ICamera*)));
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
#ifdef QMA2_ENABLE_GESTURE
    /* ジェスチャを有効にすると突然死が発生しやすいことを確認しているため無効化 */
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::SwipeGesture);
#endif
}

SceneWidget::~SceneWidget()
{
    delete m_handles;
    m_handles = 0;
    delete m_info;
    m_info = 0;
    delete m_grid;
    m_grid = 0;
    delete m_plane;
    m_plane = 0;
    delete m_background;
    m_background = 0;
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
    resetMotion();
    emit sceneDidStop();
}

void SceneWidget::clear()
{
    stop();
    clearSelectedBones();
    setSelectedModel(0);
    m_loader->release();
    m_loader->createProject();
}

void SceneWidget::startAutomaticRendering()
{
    if (!m_internalTimerID)
        m_internalTimerID = startTimer(m_interval);
}

void SceneWidget::stopAutomaticRendering()
{
    if (m_internalTimerID) {
        killTimer(m_internalTimerID);
        m_internalTimerID = 0;
    }
}

void SceneWidget::loadProject(const QString &filename)
{
    QScopedPointer<QProgressDialog> dialog(new QProgressDialog());
    QProgressDialog *ptr = dialog.data();
    /* プロジェクトの読み込みが完了したらダイアログを閉じるようにする */
    connect(m_loader, SIGNAL(projectDidLoad(bool)), ptr, SLOT(close()));
    connect(m_loader, SIGNAL(projectDidCount(int)), ptr, SLOT(setMaximum(int)));
    connect(m_loader, SIGNAL(projectDidProceed(int)), ptr, SLOT(setValue(int)));
    dialog->setLabelText(tr("Loading a project %1...").arg(QFileInfo(filename).fileName()));
    dialog->setWindowModality(Qt::WindowModal);
    dialog->setCancelButton(0);
    /* ぶら下がりポインタを残さないために選択状態のボーンを全てクリアする */
    clearSelectedBones();
    stopAutomaticRendering();
    /* プロジェクトの読み込みが完了するまで描画を一時的に停止する */
    m_enableUpdateGL = false;
    /* プロジェクト読み込み */
    m_loader->loadProject(filename);
    /* 背景画像読み込み */
    m_background->setImage(m_loader->backgroundImage());
    m_background->setImagePosition(m_loader->backgroundImagePosition());
    m_background->setUniformEnable(m_loader->isBackgroundImageUniformEnabled());
    m_enableUpdateGL = true;
    /* 0フレーム目で読み込んだ時シークされないため、強制シークを行うために m_frameIndex を -1 にする */
    m_timeIndex = -1;
    seekMotion(0, true);
    startAutomaticRendering();
    QApplication::alert(this);
}

void SceneWidget::saveProject(const QString &filename)
{
    /* 背景画像設定をプロジェクトに保存する */
    m_loader->setBackgroundImagePath(m_background->imageFilename());
    m_loader->setBackgroundImagePosition(m_background->imagePosition());
    m_loader->setBackgroundImageUniformEnable(m_background->isUniformEnabled());
    m_loader->saveProject(filename);
}

void SceneWidget::setPreferredFPS(int value)
{
    /* 一旦前のタイマーを止めてから新しい FPS に基づく間隔でタイマーを開始する */
    if (value > 0) {
        m_interval = 1000.0f / value;
        m_loader->setPreferredFPS(value);
        if (m_internalTimerID) {
            stopAutomaticRendering();
            startAutomaticRendering();
        }
    }
}

void SceneWidget::setSelectedModel(IModel *value)
{
    /* 情報パネルに選択されたモデルの名前を更新する */
    m_loader->setSelectedModel(value);
    m_info->setModel(value);
    m_info->update();
    m_editMode = value ? kSelect : kNone;
}

void SceneWidget::setBackgroundImage(const QString &filename)
{
    m_background->setImage(filename);
}

void SceneWidget::setModelEdgeOffset(double value)
{
    if (IModel *model = m_loader->selectedModel())
        m_loader->setModelEdgeOffset(model, static_cast<float>(value));
    refreshMotions();
}

void SceneWidget::setModelOpacity(const Scalar &value)
{
    if (IModel *model = m_loader->selectedModel())
        m_loader->setModelOpacity(model, value);
    refreshMotions();
}

void SceneWidget::setModelEdgeColor(const QColor &color)
{
    if (IModel *model = m_loader->selectedModel())
        m_loader->setModelEdgeColor(model, color);
    refreshMotions();
}

void SceneWidget::setModelPositionOffset(const Vector3 &value)
{
    if (IModel *model = m_loader->selectedModel())
        m_loader->setModelPosition(model, value);
    refreshMotions();
}

void SceneWidget::setModelRotationOffset(const Vector3 &value)
{
    if (IModel *model = m_loader->selectedModel())
        m_loader->setModelRotation(model, value);
    refreshMotions();
}

void SceneWidget::setModelProjectiveShadowEnable(bool value)
{
    if (IModel *model = m_loader->selectedModel())
        m_loader->setProjectiveShadowEnable(model, value);
    refreshMotions();
}

void SceneWidget::setModelSelfShadowEnable(bool value)
{
    if (IModel *model = m_loader->selectedModel())
        m_loader->setSelfShadowEnable(model, value);
    refreshMotions();
}

void SceneWidget::setModelOpenSkinningEnable(bool value)
{
    if (IModel *model = m_loader->selectedModel())
        m_loader->setSelfShadowEnable(model, value);
    refreshMotions();
}

void SceneWidget::setModelVertexShaderSkinningType1Enable(bool value)
{
    if (IModel *model = m_loader->selectedModel())
        m_loader->setSelfShadowEnable(model, value);
    refreshMotions();
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
    IModel *model = addModel(openFileDialog("sceneWidget/lastModelDirectory",
                                            tr("Open PMD/PMX file"),
                                            tr("PMD/PMX file (*.pmd *.pmx *.zip)"),
                                            m_settings));
    if (model && !m_playing) {
        setEmptyMotion(model);
        emit newMotionDidSet(model);
    }
}

IModel *SceneWidget::addModel(const QString &path, bool skipDialog)
{
    QFileInfo fi(path);
    IModel *model = 0;
    if (fi.exists()) {
        if (m_loader->loadModel(path, model)) {
            if (skipDialog || (!m_showModelDialog || acceptAddingModel(model))) {
                QUuid uuid;
                m_loader->addModel(model, fi.fileName(), fi.dir(), uuid);
                emit fileDidLoad(path);
            }
            else {
                delete model;
                model = 0;
            }
        }
        else {
            internal::warning(this, tr("Loading model error"),
                              tr("%1 cannot be loaded").arg(fi.fileName()));
        }
    }
    return model;
}

void SceneWidget::insertMotionToAllModels()
{
    /* モーションを追加したら即座に反映させるために updateMotion を呼んでおく */
    IMotion *motion = insertMotionToAllModels(openFileDialog("sceneWidget/lastModelMotionDirectory",
                                                             tr("Open VMD (for model) file"),
                                                             tr("VMD file (*.vmd)"),
                                                             m_settings));
    IModel *selected = m_loader->selectedModel();
    if (motion && selected)
        refreshMotions();
}

IMotion *SceneWidget::insertMotionToAllModels(const QString &path)
{
    IMotion *motion = 0;
    if (QFile::exists(path)) {
        QList<IModel *> models;
        motion = m_loader->loadModelMotion(path, models);
        if (motion) {
            m_timeIndex = -1;
            seekMotion(0, false);
            emit fileDidLoad(path);
        }
        else {
            internal::warning(this, tr("Loading model motion error"),
                              tr("%1 cannot be loaded").arg(QFileInfo(path).fileName()));
        }
    }
    return motion;
}

void SceneWidget::insertMotionToSelectedModel()
{
    IModel *model = m_loader->selectedModel();
    if (model) {
        IMotion *motion = insertMotionToSelectedModel(openFileDialog("sceneWidget/lastModelMotionDirectory",
                                                                     tr("Open VMD (for model) file"),
                                                                     tr("VMD file (*.vmd)"),
                                                                     m_settings));
        if (motion)
            refreshMotions();
    }
    else {
        internal::warning(this, tr("The model is not selected."),
                          tr("Select a model to insert the motion (\"Model\" > \"Select model\")"));
    }
}

IMotion *SceneWidget::insertMotionToSelectedModel(const QString &path)
{
    return insertMotionToModel(path, m_loader->selectedModel());
}

IMotion *SceneWidget::insertMotionToModel(const QString &path, IModel *model)
{
    IMotion *motion = 0;
    if (model) {
        if (QFile::exists(path)) {
            motion = m_loader->loadModelMotion(path);
            if (motion) {
                /* 違うモデルに適用しようとしているかどうかの確認 */
                if (!model->name()->equals(motion->name())) {
                    int ret = internal::warning(0,
                                                tr("Applying this motion to the different model"),
                                                tr("This motion is created for %1. Do you apply this motion to %2?")
                                                .arg(internal::toQStringFromMotion(motion))
                                                .arg(internal::toQStringFromModel(model)),
                                                "",
                                                QMessageBox::Yes|QMessageBox::No);
                    if (ret == QMessageBox::Yes) {
                        m_loader->setModelMotion(motion, model);
                    }
                    else {
                        delete motion;
                        motion = 0;
                        return motion;
                    }
                }
                else {
                    m_loader->setModelMotion(motion, model);
                }
                m_timeIndex = -1;
                seekMotion(0, false);
                emit fileDidLoad(path);
            }
            else {
                internal::warning(this, tr("Loading model motion error"),
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

void SceneWidget::setEmptyMotion(IModel *model)
{
    if (model) {
        IMotion *modelMotion = m_loader->newModelMotion(model);
        m_loader->setModelMotion(modelMotion, model);
    }
    else
        internal::warning(this,
                          tr("The model is not selected."),
                          tr("Select a model to insert the motion (\"Model\" > \"Select model\")"));
}

void SceneWidget::addAsset()
{
    addAsset(openFileDialog("sceneWidget/lastAssetDirectory",
                            tr("Open X file"),
                            tr("DirectX mesh file (*.x *.zip)"),
                            m_settings));
}

IModel *SceneWidget::addAsset(const QString &path)
{
    IModel *asset = 0;
    QFileInfo fi(path);
    if (fi.exists()) {
        QUuid uuid;
        if (m_loader->loadAsset(path, uuid, asset)) {
            emit fileDidLoad(path);
        }
        else {
            internal::warning(this, tr("Loading asset error"),
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

IModel *SceneWidget::addAssetFromMetadata(const QString &path)
{
    QFileInfo fi(path);
    IModel *asset = 0;
    if (fi.exists()) {
        QUuid uuid;
        asset = m_loader->loadAssetFromMetadata(fi.fileName(), fi.dir(), uuid);
        if (asset) {
            setFocus();
        }
        else {
            internal::warning(this, tr("Loading asset error"),
                              tr("%1 cannot be loaded").arg(fi.fileName()));
        }
    }
    return asset;
}

void SceneWidget::saveMetadataFromAsset(IModel *asset)
{
    if (asset) {
        QString filename = QFileDialog::getSaveFileName(this, tr("Save %1 as VAC file")
                                                        .arg(internal::toQStringFromModel(asset)), "",
                                                        tr("MMD accessory metadata (*.vac)"));
        m_loader->saveMetadataFromAsset(filename, asset);
    }
}

void SceneWidget::insertPoseToSelectedModel()
{
    IModel *model = m_loader->selectedModel();
    VPDFilePtr ptr = insertPoseToSelectedModel(openFileDialog("sceneWidget/lastPoseDirectory",
                                                              tr("Open VPD file"),
                                                              tr("VPD file (*.vpd)"),
                                                              m_settings),
                                               model);
    if (!ptr.isNull() && model)
        model->performUpdate(m_loader->scene()->light()->direction());
}

void SceneWidget::setBackgroundImage()
{
    const QString &filename = openFileDialog("sceneWidget/lastBackgroundImageDirectory",
                                             tr("Open background image file"),
                                             tr("Image file (*.bmp *.jpg *.gif *.png *.tif);; Movie file (*.mng)"),
                                             m_settings);
    if (!filename.isEmpty())
        setBackgroundImage(filename);
}

void SceneWidget::setBackgroundPosition(const QPoint &value)
{
    m_background->setImagePosition(value);
    m_loader->setBackgroundImagePosition(value);
}

void SceneWidget::setBackgroundImageUniformEnable(bool value)
{
    m_background->setUniformEnable(value);
    m_background->resize(size());
    m_loader->setBackgroundImageUniformEnable(value);
}

void SceneWidget::clearBackgroundImage()
{
    setBackgroundImage("");
}

VPDFilePtr SceneWidget::insertPoseToSelectedModel(const QString &filename, IModel *model)
{
    VPDFilePtr ptr;
    if (model) {
        if (QFile::exists(filename)) {
            ptr = m_loader->loadModelPose(filename, model);
            if (ptr.isNull()) {
                internal::warning(this, tr("Loading model pose error"),
                                  tr("%1 cannot be loaded").arg(QFileInfo(filename).fileName()));
            }
        }
    }
    else
        internal::warning(this,
                          tr("The model is not selected."),
                          tr("Select a model to set the pose (\"Model\" > \"Select model\")"));
    return ptr;
}

void SceneWidget::advanceMotion(const IKeyframe::TimeIndex &delta)
{
    if (delta <= 0)
        return;
    Scene *scene = m_loader->scene();
    scene->advance(delta);
    if (m_loader->isPhysicsEnabled())
        m_loader->world()->stepSimulationDelta(delta);
    updateScene();
}

void SceneWidget::seekMotion(const IKeyframe::TimeIndex &timeIndex, bool forceCameraUpdate)
{
    /*
       渡された値が同じフレーム位置の場合は何もしない
       (シグナルスロット処理の関係でモーフスライダーが動かなくなってしまうため)
     */
    if (timeIndex == m_timeIndex)
        return;
    /*
       advanceMotion に似ているが、前のフレームインデックスを利用することがあるので、保存しておく必要がある。
       force でカメラと照明を強制的に動かすことが出来る(例として場面タブからシークした場合)。
     */
    Scene *scene = m_loader->scene();
    if (!forceCameraUpdate) {
        /* 一旦カメラと照明のモーションを外してモデルのモーションのみ動かすようにする */
        Scene::ICamera *camera = scene->camera();
        Scene::ILight *light = scene->light();
        IMotion *cameraMotion = camera->motion();
        IMotion *lightMotion = light->motion();
        camera->setMotion(0);
        light->setMotion(0);
        scene->seek(timeIndex);
        camera->setMotion(cameraMotion);
        light->setMotion(lightMotion);
    }
    else {
        scene->seek(timeIndex);
    }
    m_timeIndex = timeIndex;
    m_background->setFrameIndex(timeIndex);
    emit motionDidSeek(timeIndex);
    updateScene();
}

void SceneWidget::resetMotion()
{
    Scene *scene = m_loader->scene();
    const Array<IMotion *> &motions = scene->motions();
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        motion->reset();
    }
    m_timeIndex = 0;
    m_background->setFrameIndex(0);
    updateScene();
    emit motionDidSeek(0.0f);
}

void SceneWidget::setCamera()
{
    IMotion *motion = setCamera(openFileDialog("sceneWidget/lastCameraMotionDirectory",
                                               tr("Open VMD (for camera) file"),
                                               tr("VMD file (*.vmd)"),
                                               m_settings));
    if (motion)
        refreshScene();
}

IMotion *SceneWidget::setCamera(const QString &path)
{
    IMotion *motion = 0;
    if (QFile::exists(path)) {
        motion = m_loader->loadCameraMotion(path);
        if (motion) {
            emit fileDidLoad(path);
        }
        else {
            internal::warning(this, tr("Loading camera motion error"),
                              tr("%1 cannot be loaded").arg(QFileInfo(path).fileName()));
        }
    }
    return motion;
}

void SceneWidget::deleteSelectedModel()
{
    IModel *selected = m_loader->selectedModel();
    if (selected) {
        int ret = internal::warning(0,
                                    tr("Confirm"),
                                    tr("Do you want to delete the model \"%1\"? This cannot undo.")
                                    .arg(internal::toQStringFromModel(selected)),
                                    "",
                                    QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            /* モデルを削除しても IBone への参照が残ってしまうので中身を空にする */
            clearSelectedBones();
            m_loader->deleteModel(selected);
        }
    }
}

void SceneWidget::resetCamera()
{
    Scene::ICamera *camera = m_loader->scene()->camera();
    camera->resetDefault();
    emit cameraPerspectiveDidSet(camera);
}

void SceneWidget::setCameraPerspective(const QSharedPointer<Scene::ICamera> &camera)
{
    Scene::ICamera *c1 = camera.data(), *c2 = m_loader->scene()->camera();
    c2->copyFrom(c1);
    emit cameraPerspectiveDidSet(c2);
}

void SceneWidget::makeRay(const QPointF &input, Vector3 &rayFrom, Vector3 &rayTo) const
{
    // This implementation based on the below page.
    // http://softwareprodigy.blogspot.com/2009/08/gluunproject-for-iphone-opengl-es.html
    GLdouble modelviewMatrixd[16], projectionMatrixd[16];
    const GLint viewport[4] = { 0, 0, width(), height() };
    QMatrix4x4 view, projection;
    m_loader->getCameraMatrices(view, projection);
    for (int i = 0; i < 16; i++) {
        modelviewMatrixd[i] = view.constData()[i];
        projectionMatrixd[i] = projection.constData()[i];
    }
    GLdouble wx = input.x(), wy = height() - input.y(), cx, cy, cz, fx, fy, fz;
    gluUnProject(wx, wy, 0, modelviewMatrixd, projectionMatrixd, viewport, &cx, &cy, &cz);
    gluUnProject(wx, wy, 1, modelviewMatrixd, projectionMatrixd, viewport, &fx, &fy, &fz);
    rayFrom.setValue(cx, cy, cz);
    rayTo.setValue(fx, fy, fz);
}

void SceneWidget::selectBones(const QList<IBone *> &bones)
{
    /* signal/slot による循環参照防止 */
    if (m_selectedBones != bones) {
        m_info->setBones(bones, tr("(multiple)"));
        m_info->update();
        m_handles->setBone(bones.isEmpty() ? 0 : bones.first());
        m_selectedBones = bones;
        emit bonesDidSelect(bones);
    }
}

void SceneWidget::rotateScene(const Vector3 &delta)
{
    Scene::ICamera *camera = m_loader->scene()->camera();
    camera->setAngle(camera->angle() + delta);
    emit cameraPerspectiveDidSet(camera);
}

void SceneWidget::rotateModel(const Quaternion &delta)
{
    rotateModel(m_loader->selectedModel(), delta);
}

void SceneWidget::rotateModel(IModel *model, const Quaternion &delta)
{
    if (model) {
        const Quaternion &rotation = model->rotation();
        model->setRotation(rotation * delta);
        emit modelDidRotate(rotation);
    }
}

void SceneWidget::translateScene(const Vector3 &delta)
{
    Scene::ICamera *camera = m_loader->scene()->camera();
    camera->setPosition(camera->position() + camera->modelViewTransform().getBasis().inverse() * delta);
    emit cameraPerspectiveDidSet(camera);
}

void SceneWidget::translateModel(const Vector3 &delta)
{
    translateModel(m_loader->selectedModel(), delta);
}

void SceneWidget::translateModel(IModel *model, const Vector3 &delta)
{
    if (model) {
        const Vector3 &position = model->position();
        model->setPosition(position + delta);
        emit modelDidMove(position);
    }
}

void SceneWidget::resetModelPosition()
{
    IModel *model = m_loader->selectedModel();
    if (model) {
        const Vector3 &position = model->position();
        model->setPosition(kZeroV3);
        emit modelDidMove(position);
    }
}

void SceneWidget::updatePlaneWorld(const Scene::ICamera *camera)
{
    Transform transform = camera->modelViewTransform().inverse();
    transform.setOrigin(kZeroV3);
    m_plane->updateTransform(transform);
}

void SceneWidget::loadFile(const QString &file)
{
    /* モデルファイル */
    QFileInfo fileInfo(file);
    const QString &extension = fileInfo.suffix().toLower();
    if (extension == "pmd" || extension == "pmx" || extension == "zip") {
        IModel *model = addModel(file);
        if (model && !m_playing) {
            setEmptyMotion(model);
            emit newMotionDidSet(model);
        }
    }
    /* モーションファイル */
    else if (extension == "vmd") {
        IMotion *motion = insertMotionToModel(file, m_loader->selectedModel());
        if (motion)
            refreshMotions();
    }
    /* アクセサリファイル */
    else if (extension == "x") {
        addAsset(file);
    }
    /* ポーズファイル */
    else if (extension == "vpd") {
        IModel *model = m_loader->selectedModel();
        VPDFilePtr ptr = insertPoseToSelectedModel(file, model);
        if (!ptr.isNull() && model)
            model->performUpdate(m_loader->scene()->light()->direction());
    }
    /* アクセサリ情報ファイル */
    else if (extension == "vac") {
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
    Scene::ICamera *camera = m_loader->scene()->camera();
    Scalar fovyStep = 1.0f, distanceStep = 4.0f;
    if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
        Scalar fovy = camera->fov();
        camera->setFov(up ? fovy - fovyStep : fovy + fovyStep);
    }
    else {
        Scalar distance = camera->distance();
        if (modifiers & Qt::ControlModifier)
            distanceStep *= 5.0f;
        else if (modifiers & Qt::ShiftModifier)
            distanceStep *= 0.2f;
        if (distanceStep != 0.0f)
            camera->setDistance(up ? distance - distanceStep : distance + distanceStep);
    }
    emit cameraPerspectiveDidSet(camera);
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
    qDebug("VPVL2 version: %s (%d)", VPVL2_VERSION_STRING, VPVL2_VERSION);
    qDebug("GL_VERSION: %s", glGetString(GL_VERSION));
    qDebug("GL_VENDOR: %s", glGetString(GL_VENDOR));
    qDebug("GL_RENDERER: %s", glGetString(GL_RENDERER));
    /* アルファブレンドとカリングを初期状態から有効にする */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /* OpenGL の初期化が最低条件なため、Renderer はここでインスタンスを作成する */
    m_loader = new SceneLoader(m_encoding, m_factory, this);
    connect(m_loader, SIGNAL(projectDidLoad(bool)), SLOT(openErrorDialogIfFailed(bool)));
    const QSize &s = size();
    m_handles = new Handles(m_loader, s);
    m_info = new InfoPanel(s);
    m_debugDrawer = new DebugDrawer();
#ifdef IS_QMA2
    /* OpenGL を利用するため、格子状フィールドの初期化もここで行う */
    m_grid->load();
    /* テクスチャ情報を必要とするため、ハンドルのリソースの読み込みはここで行う */
    m_handles->load();
    /* 動的なテクスチャ作成を行うため、情報パネルのリソースの読み込みも個々で行った上で初期設定を行う */
    m_info->load();
    /* デバッグ表示のシェーダ読み込み(ハンドルと同じソースを使う) */
    m_debugDrawer->load();
#endif
    m_background = new BackgroundImage(s);
    m_loader->updateDepthBuffer(QSize());
    m_info->setModel(0);
    m_info->setBones(QList<IBone *>(), "");
    m_info->setFPS(0.0f);
    m_info->update();
    m_timer.start();
    startAutomaticRendering();
    emit initailizeGLContextDidDone();
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
    const QPointF &pos = event->posF();
    int flags;
    m_lockTouchEvent = true;
    m_clickOrigin = pos;
    m_handles->setPoint2D(pos);
    Vector3 znear, zfar, hit;
#ifdef IS_QMA2
    QRectF rect;
    makeRay(pos, znear, zfar);
    m_loader->setMousePosition(event, geometry());
    m_plane->test(znear, zfar, hit);
    /* 今は決め打ちの値にしている */
    const Scalar &delta = 0.0005 * m_loader->scene()->camera()->distance();
    m_delta.setX(delta);
    m_delta.setY(delta);
    /*
     * モデルのハンドルと重なるケースを考慮して右下のハンドルを優先的に処理する。
     * また、右下のハンドル処理はひとつ以上ボーンが選択されていなければならない。
     */
    bool movable = false, rotateable = false;
    if (!m_selectedBones.isEmpty()) {
        IBone *bone = m_selectedBones.last();
        movable = bone->isMovable();
        rotateable = bone->isRotateable();
        /* 右下のハンドルを掴まれた */
        if (m_handles->testHitImage(pos, movable, rotateable, flags, rect)) {
            /* 事前にカーソル消去用のビットマップを作成する */
            QPixmap npixmap(32, 32);
            npixmap.fill(Qt::transparent);
            /* kView => kLocal => kGlobal => kView の順番で切り替えを行う */
            switch (flags) {
            case Handles::kView:
                m_handles->setState(Handles::kLocal);
                break;
            case Handles::kLocal:
                m_handles->setState(Handles::kGlobal);
                break;
            case Handles::kGlobal:
                m_handles->setState(Handles::kView);
                break;
                /* ハンドルを掴まれたらカーソルを消去する */
            case Handles::kNone:
            case Handles::kEnable:
            case Handles::kDisable:
            case Handles::kMove:
            case Handles::kRotate:
            case Handles::kX:
            case Handles::kY:
            case Handles::kZ:
            case Handles::kModel:
            case Handles::kVisibleMove:
            case Handles::kVisibleRotate:
            case Handles::kVisibleAll:
            default:
                setCursor(QCursor(npixmap));
                m_handleFlags = flags;
                emit handleDidGrab();
                break;
            }
            return;
        }
    }
#else
    Q_UNUSED(hit)
#endif
    /* モデルが選択状態にある */
    if (IModel *model = m_loader->selectedModel()) {
        /* ボーン選択モードである */
        if (m_editMode == kSelect) {
            IBone *nearestBone = findNearestBone(model, znear, zfar, 0.1);
            /* 操作可能で最も近いボーンを選択状態にする */
            if (nearestBone) {
                QList<IBone *> selectedBones;
                /* CTRL が押されている場合は前回の選択状態を引き継ぐ */
                if (event->modifiers() & Qt::CTRL)
                    selectedBones.append(m_selectedBones);
                /* すでにボーンが選択済みの場合は選択状態を外す */
                if (selectedBones.contains(nearestBone))
                    selectedBones.removeOne(nearestBone);
                else
                    selectedBones.append(nearestBone);
                selectBones(selectedBones);
            }
        }
        /*
         * 回転モードまたは移動モードでかつモデルハンドルにカーソルがあっている場合 handleDidGrab を発行する。
         * 回転モードの場合は該当する部分 (例えば X なら X のサークル) のみを表示する。
         */
        else if (m_editMode == kRotate || m_editMode == kMove) {
            Vector3 rayFrom, rayTo, pick;
            makeRay(pos, rayFrom, rayTo);
            /* 移動ボーンでかつ範囲内にある */
            IBone *bone = m_handles->currentBone();
            if (bone->isMovable() && intersectsBone(bone, znear, zfar, 0.5)) {
                m_currentSelectedBone = bone;
                m_lastBonePosition = bone->worldTransform().getOrigin();
                setCursor(Qt::ClosedHandCursor);
                emit handleDidGrab();
                return;
            }
            /* モデルハンドルにカーソルが入ってる */
            if (m_handles->testHitModel(rayFrom, rayTo, false, flags, pick)) {
                m_handleFlags = flags;
                if (m_editMode == kRotate)
                    m_handles->setVisibilityFlags(flags);
                setCursor(Qt::ClosedHandCursor);
                emit handleDidGrab();
            }
        }
    }
}

void SceneWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QPointF &pos = event->posF();
    IBone *bone = m_handles->currentBone();
    m_isImageHandleRectIntersect = false;
    m_loader->setMousePosition(event, geometry());
    if (m_currentSelectedBone) {
        Vector3 znear, zfar, hit;
        makeRay(pos, znear, zfar);
        m_plane->test(znear, zfar, hit);
        const Vector3 &delta = hit - m_lastBonePosition;
        emit handleDidMoveAbsolute(delta, m_currentSelectedBone, 'G');
    }
    else if (event->buttons() & Qt::LeftButton) {
        const Qt::KeyboardModifiers modifiers = event->modifiers();
        const QPointF &diff = m_handles->diffPoint2D(pos);
        /* モデルのハンドルがクリックされた */
        if (m_handleFlags & Handles::kModel) {
            grabModelHandleByRaycast(pos, diff, m_handleFlags);
        }
        /* 有効な右下のハンドルがクリックされた (かつ操作切り替えボタンではないこと) */
        else if (m_handleFlags & Handles::kEnable && !Handles::isToggleButton(m_handleFlags)) {
            m_isImageHandleRectIntersect = true;
            m_totalDelta = m_totalDelta + (pos.y() - m_clickOrigin.y()) * m_delta.y();
            grabImageHandle(m_totalDelta);
            QCursor::setPos(mapToGlobal(m_clickOrigin.toPoint()));
        }
        /* 光源移動 */
        else if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            Scene::ILight *light = m_loader->scene()->light();
            const Vector3 &direction = light->direction();
            Quaternion rx(0.0f, diff.y() * radian(0.1f), 0.0f),
                    ry(0.0f, diff.x() * radian(0.1f), 0.0f);
            light->setDirection(direction * Matrix3x3(rx * ry));
        }
        /* 場面の移動 (X 方向だけ向きを逆にする) */
        else if (modifiers & Qt::ShiftModifier || event->buttons() & Qt::MiddleButton) {
            translateScene(Vector3(diff.x() * -m_delta.x(), diff.y() * m_delta.y(), 0.0f));
        }
        /* 場面の回転 (X と Y が逆転している点に注意) */
        else {
            rotateScene(Vector3(diff.y() * 0.5f, diff.x() * 0.5f, 0.0f));
        }
        m_handles->setPoint2D(pos);
    }
    else if (bone) {
        QRectF rect;
        int flags;
        bool movable = bone->isMovable(), rotateable = bone->isRotateable();
        Vector3 znear, zfar;
        makeRay(pos, znear, zfar);
        /* 選択モード状態ではなく、かつ移動可能ボーンに付随する水色の球状にカーソルがあたっているか？ */
        if (movable && m_editMode != kSelect && intersectsBone(bone, znear, zfar, 0.5)) {
            setCursor(Qt::OpenHandCursor);
            return;
        }
        /* 操作ハンドル(右下の画像)にマウスカーソルが入ってるか? */
        m_isImageHandleRectIntersect = m_handles->testHitImage(pos, movable, rotateable, flags, rect);
        /* ハンドル操作中ではない場合のみ (m_handleFlags は mousePressEvent で設定される) */
        if (m_handleFlags == Handles::kNone) {
            if (m_isImageHandleRectIntersect) {
                /* 切り替えボタン */
                if (Handles::isToggleButton(flags))
                    setCursor(Qt::PointingHandCursor);
                /* 有効な回転または移動ハンドル。無効の場合は何もしない */
                else if (flags & Handles::kEnable)
                    setCursor(Qt::SizeVerCursor);
            }
            /* 回転モードの場合は回転ハンドルに入っているか? */
            else if (testHitModelHandle(pos)) {
                setCursor(Qt::OpenHandCursor);
            }
            else {
                unsetCursor();
            }
        }
    }
}

void SceneWidget::mouseReleaseEvent(QMouseEvent *event)
{
    /* 回転モードの場合は回転ハンドルに入っているか? */
    if (testHitModelHandle(event->posF()))
        setCursor(Qt::OpenHandCursor);
    else
        unsetCursor();
    m_loader->setMousePosition(event, geometry());
    /* 状態をリセットする */
    m_totalDelta = 0.0f;
    /* handleDidRelease を発行するかどうかを判定するためフラグの状態を保存する */
    int flags = m_handleFlags;
    m_handleFlags = Handles::kNone;
    m_handles->setAngle(0.0f);
    m_handles->setPoint3D(Vector3(0.0f, 0.0f, 0.0f));
    m_lockTouchEvent = false;
    setEditMode(m_editMode);
    /*
     * ハンドルを使って操作した場合のみ handleDidRelease を発行する
     * (そうしないと commitTransform が呼ばれて余計な UndoCommand が追加されてしまうため)
     */
    bool isModelHandle = flags & Handles::kModel;
    bool isImageHandle = flags & Handles::kEnable && !Handles::isToggleButton(flags);
    if (isModelHandle || isImageHandle || m_currentSelectedBone)
        emit handleDidRelease();
    m_currentSelectedBone = 0;
    m_lastBonePosition.setZero();
}

void SceneWidget::paintGL()
{
#ifdef IS_QMA2
    Scene *scene = m_loader->scene();
    /* ボーン選択モード以外でのみ深度バッファのレンダリングを行う */
    if (m_editMode != kSelect) {
        m_loader->renderZPlotToTexture();
        scene->light()->setToonEnable(true);
    }
    else {
        scene->light()->setToonEnable(false);
    }
    /* 通常のレンダリングを行うよう切り替えてレンダリングする */
    qglClearColor(m_loader->screenColor());
    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_background->draw();
    m_grid->draw(m_loader, m_loader->isGridVisible());
    m_loader->renderModels();
    /* ボーン選択済みかどうか？ボーンが選択されていればハンドル描写を行う */
    IBone *bone = 0;
    if (!m_selectedBones.isEmpty())
        bone = m_selectedBones.first();
    switch (m_editMode) {
    case kSelect: /* ボーン選択モード */
        /* モデルのボーンの接続部分をレンダリング */
        if (!(m_handleFlags & Handles::kEnable)) {
            QSet<const IBone *> boneSet;
            foreach (const IBone *bone, m_selectedBones)
                boneSet.insert(bone);
            m_debugDrawer->drawModelBones(m_loader->selectedModel(), m_loader, boneSet);
        }
        /* 右下のハンドルが領域に入ってる場合は軸を表示する */
        if (m_isImageHandleRectIntersect)
            m_debugDrawer->drawBoneTransform(bone, m_loader, m_handles->modeFromConstraint());
        /*
         * 情報パネルと右下のハンドルを最後にレンダリング(表示上最上位に持っていく)
         * 右下の操作ハンドルはモデルが選択されていない場合は非表示にするように変更
         */
        if (m_loader->selectedModel())
            m_handles->drawImageHandles(bone);
        m_info->draw();
        break;
    case kRotate: /* 回転モード */
        /* kRotate と kMove の場合はレンダリングがうまくいかない関係でモデルのハンドルを最後に持ってく */
        m_debugDrawer->drawMovableBone(bone, m_loader);
        m_handles->drawImageHandles(bone);
        m_info->draw();
        m_handles->drawRotationHandle();
        break;
    case kMove: /* 移動モード */
        m_debugDrawer->drawMovableBone(bone, m_loader);
        m_handles->drawImageHandles(bone);
        m_info->draw();
        m_handles->drawMoveHandle();
        break;
    case kNone: /* モデル選択なし */
    default:
        m_info->draw();
        break;
    }
#endif
}

void SceneWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    const QSize s(w, h);
    m_handles->resize(s);
    m_info->resize(s);
    m_background->resize(s);
}

void SceneWidget::timerEvent(QTimerEvent *event)
{
    /* モーション再生のタイマーが生きている => 描写命令を出す */
    if (event->timerId() == m_internalTimerID) {
        if (m_playing) {
            /* タイマーの仕様上一定ではないため、差分をここで吸収する */
            Scalar elapsed = m_timer.elapsed() / Scene::defaultFPS();
            Scalar diff = elapsed - m_prevElapsed;
            m_prevElapsed = elapsed;
            if (diff < 0)
                diff = elapsed;
            advanceMotion(diff);
            updateFPS();
        }
        else {
            /* 非再生中(編集中)はモーションを一切動かさず、カメラの更新だけ行う */
            Scene *scene = m_loader->scene();
            scene->updateCamera();
            updateScene();
        }
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
    /* 移動のジェスチャー */
    const QPointF &delta = event->delta();
    const Vector3 newDelta(delta.x() * -0.25, delta.y() * 0.25, 0.0f);
    if (!m_selectedBones.isEmpty()) {
        IBone *bone = m_selectedBones.last();
        switch (state) {
        case Qt::GestureStarted:
            emit handleDidGrab();
            break;
        case Qt::GestureUpdated:
            emit handleDidMoveRelative(newDelta, bone, 'V');
            break;
        case Qt::GestureFinished:
            emit handleDidRelease();
            break;
        default:
            break;
        }
    }
    else if (IModel *model = m_loader->selectedModel())
        translateModel(model, newDelta);
    else
        translateScene(newDelta);
}

void SceneWidget::pinchTriggered(QPinchGesture *event)
{
    const Qt::GestureState state = event->state();
    QPinchGesture::ChangeFlags flags = event->changeFlags();
    Scene::ICamera *camera = m_loader->scene()->camera();
    /* 回転ジェスチャー */
    if (m_enableRotateGesture && flags & QPinchGesture::RotationAngleChanged) {
        qreal value = event->rotationAngle() - event->lastRotationAngle();
        const Scalar &radian = vpvl2::radian(value);
        /* ボーンが選択されている場合はボーンの回転 (現時点でY軸のみ) */
        if (!m_selectedBones.isEmpty()) {
            IBone *bone = m_selectedBones.last();
            int mode = m_handles->modeFromConstraint(), axis = 'Y' << 8;
            switch (state) {
            case Qt::GestureStarted:
                emit handleDidGrab();
                break;
            case Qt::GestureUpdated:
                emit handleDidRotate(event->rotationAngle(), bone, mode | axis);
                break;
            case Qt::GestureFinished:
                emit handleDidRelease();
                break;
            default:
                break;
            }
        }
        /* 四元数を使う場合回転が時計回りになるよう符号を反転させる必要がある */
        else if (IModel *model = m_loader->selectedModel()) {
            Quaternion rotation(0.0f, 0.0f, 0.0f, 1.0f);
            rotation.setEulerZYX(0.0f, -radian, 0.0f);
            rotateModel(model, rotation);
        }
        else {
            rotateScene(Vector3(0.0f, value, 0.0f));
        }
    }
    /* 拡大縮小のジェスチャー */
    Scalar distance = camera->distance();
    if (state == Qt::GestureStarted)
        m_lastDistance = distance;
    if (m_enableScaleGesture && flags & QPinchGesture::ScaleFactorChanged) {
        qreal scaleFactor = event->scaleFactor();
        distance = m_lastDistance * scaleFactor;
        if (qFuzzyCompare(scaleFactor, 1.0)) {
            camera->setDistance(distance);
            emit cameraPerspectiveDidSet(camera);
        }
    }
}

void SceneWidget::swipeTriggered(QSwipeGesture *event)
{
    /* 左か上の場合は巻き戻し、右か下の場合はやり直しを実行する */
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

void SceneWidget::openErrorDialogIfFailed(bool loadingProjectFailed)
{
    if (!loadingProjectFailed) {
        internal::warning(this,
                          tr("Failed loading the project"),
                          tr("Failed loading the project. The project contains duplicated UUID or corrupted."));
    }
}

bool SceneWidget::acceptAddingModel(IModel *model)
{
    /* モデルを追加する前にモデルの名前とコメントを出すダイアログを表示 */
    QMessageBox mbox;
    QString comment = internal::toQStringFromString(model->comment());
    mbox.setText(tr("Model Information of \"%1\"").arg(internal::toQStringFromModel(model)));
    mbox.setInformativeText(comment.replace("\n", "<br>"));
    mbox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    return mbox.exec() == QMessageBox::Ok;
}

bool SceneWidget::testHitModelHandle(const QPointF &pos)
{
    if (m_editMode == SceneWidget::kRotate || m_editMode == SceneWidget::kMove) {
        Vector3 rayFrom, rayTo, pick;
        int flags;
        makeRay(pos, rayFrom, rayTo);
        return m_handles->testHitModel(rayFrom, rayTo, true, flags, pick);
    }
    return false;
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

void SceneWidget::updateScene()
{
    m_loader->updateMatrices(QSizeF(size()));
    m_loader->scene()->updateRenderEngines();
    if (m_enableUpdateGL)
        updateGL();
    emit cameraPerspectiveDidSet(m_loader->scene()->camera());
}

void SceneWidget::clearSelectedBones()
{
    m_selectedBones.clear();
    m_info->setBones(m_selectedBones, "");
    m_handles->setBone(0);
}

void SceneWidget::grabImageHandle(const Scalar &deltaValue)
{
    int flags = m_handleFlags;
    int mode = m_handles->modeFromConstraint();
    /* 移動ハンドルである */
    if (flags & Handles::kMove) {
        /* 意図する向きと実際の値が逆なので、反転させる */
        Vector3 delta(0.0f, 0.0f, 0.0f);
        const Scalar &kDeltaBias = -0.2;
        if (flags & Handles::kX)
            delta.setX(deltaValue * kDeltaBias);
        else if (flags & Handles::kY)
            delta.setY(deltaValue * kDeltaBias);
        else if (flags & Handles::kZ)
            delta.setZ(deltaValue * kDeltaBias);
        emit handleDidMoveAbsolute(delta, 0, mode);
    }
    /* 回転ハンドルである */
    else if (flags & Handles::kRotate) {
        /* 上にいくとマイナスになるように変更する */
        const Scalar &angle = -deltaValue;
        int axis = 0;
        if (flags & Handles::kX) {
            axis = 'X' << 8;
        }
        else if (flags & Handles::kY) {
            axis = 'Y' << 8;
        }
        else if (flags & Handles::kZ) {
            axis = 'Z' << 8;
        }
        emit handleDidRotate(angle, 0, mode | axis);
    }
}

void SceneWidget::grabModelHandleByRaycast(const QPointF &pos, const QPointF &diff, int flags)
{
    int mode = m_handles->modeFromConstraint();
    Vector3 rayFrom, rayTo, pick, delta = kZeroV3;
    /* モデルのハンドルに当たっている場合のみモデルを動かす */
    if (flags & Handles::kMove) {
        const QPointF &diff2 = QPointF(diff.x() * m_delta.x(), diff.y() * m_delta.y());
        /* 移動ハンドルである(矢印の先端) */
        if (flags & Handles::kX) {
            delta.setValue(diff2.x(), 0, 0);
        }
        else if (flags & Handles::kY) {
            delta.setValue(0, -diff2.y(), 0);
        }
        else if (flags & Handles::kZ) {
            delta.setValue(0, 0, diff2.y());
        }
        emit handleDidMoveRelative(delta, 0, mode);
        return;
    }
    makeRay(pos, rayFrom, rayTo);
    if (m_handles->testHitModel(rayFrom, rayTo, false, flags, pick)) {
        /* 回転ハンドルである(ドーナツ) */
        if (flags & Handles::kRotate) {
            const Vector3 &origin = m_handles->currentBone()->worldTransform().getOrigin();
            const Vector3 &delta = (pick - origin).normalize();
            Scalar angle = 0.0;
            int axis = 0;
            if (flags & Handles::kX) {
                angle = btAtan2(delta.y(), delta.z());
                axis = 'X' << 8;
            }
            else if (flags & Handles::kY) {
                angle = -btAtan2(delta.x(), delta.z());
                axis = 'Y' << 8;
            }
            else if (flags & Handles::kZ) {
                angle = -btAtan2(delta.x(), delta.y());
                axis = 'Z' << 8;
            }
            if (m_handles->isAngleZero()) {
                m_handles->setAngle(angle);
                angle = 0.0;
            }
            else {
                angle = btDegrees(m_handles->diffAngle(angle));
            }
            emit handleDidRotate(angle, 0, mode | axis);
        }
        m_handles->setPoint3D(pick);
    }
}

IBone *SceneWidget::findNearestBone(const IModel *model,
                                    const Vector3 &znear,
                                    const Vector3 &zfar,
                                    const Scalar &threshold) const
{
    static const Vector3 size(threshold, threshold, threshold);
    Array<IBone *> bones;
    model->getBones(bones);
    const int nbones = bones.count();
    IBone *nearestBone = 0;
    Scalar hitLambda = 1.0f;
    Vector3 normal;
    /* 操作可能なボーンを探す */
    for (int i = 0; i < nbones; i++) {
        IBone *bone = bones[i];
        const Vector3 &o = bone->worldTransform().getOrigin(),
                min = o - size, max = o + size;
        if (btRayAabb(znear, zfar, min, max, hitLambda, normal) && bone->isInteractive()) {
            nearestBone = bone;
            break;
        }
    }
    return nearestBone;
}

bool SceneWidget::intersectsBone(const IBone *bone,
                                 const Vector3 &znear,
                                 const Vector3 &zfar,
                                 const Scalar &threshold) const
{
    static const Vector3 size(threshold, threshold, threshold);
    const Vector3 &o = bone->worldTransform().getOrigin(),
            min = o - size, max = o + size;
    Scalar hitLambda = 1.0f;
    Vector3 normal;
    return btRayAabb(znear, zfar, min, max, hitLambda, normal);
}

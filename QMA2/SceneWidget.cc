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
#include "DebugDrawer.h"
#include "Delegate.h"
#include "Grid.h"
#include "Handles.h"
#include "InfoPanel.h"
#include "SceneLoader.h"
#include "World.h"
#include "util.h"

#include <QtGui/QtGui>
#include <btBulletDynamicsCommon.h>
#include <vpvl/vpvl.h>

#ifdef VPVL_USE_GLSL
#include <vpvl/gl2/Renderer.h>
using namespace vpvl::gl2;
#else
#include <vpvl/gl/Renderer.h>
using namespace vpvl::gl;
#endif
using namespace internal;

SceneWidget::SceneWidget(QSettings *settings, QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_renderer(0),
    m_debugDrawer(0),
    m_delegate(0),
    m_grid(0),
    m_handles(0),
    m_info(0),
    m_world(0),
    m_bone(0),
    m_doubleClickedPos(0.0f, 0.0f, 0.0f),
    m_loader(0),
    m_settings(settings),
    m_prevElapsed(0.0f),
    m_selectedEdgeOffset(0.0f),
    m_frameIndex(0.0f),
    m_frameCount(0),
    m_currentFPS(0),
    m_defaultFPS(60),
    m_interval(1000.0f / m_defaultFPS),
    m_internalTimerID(0),
    m_handleFlags(0),
    m_visibleBones(false),
    m_playing(false),
    m_enableBoneMove(false),
    m_enableBoneRotate(false),
    m_enablePhysics(false)
{
    m_debugDrawer = new DebugDrawer();
    m_delegate = new Delegate(this);
    m_grid = new Grid();
    m_info = new InfoPanel(this);
    m_world = new World(m_defaultFPS);
    m_handles = new Handles(this);
    // must be delay to execute on initializeGL
    m_enablePhysics = m_settings->value("sceneWidget/isPhysicsEnabled", false).toBool();
    setBoneWireframeVisible(m_settings->value("sceneWidget/isBoneWireframeVisible", false).toBool());
    setGridVisible(m_settings->value("sceneWidget/isGridVisible", true).toBool());
    setAcceptDrops(true);
    setAutoFillBackground(false);
    setMinimumSize(540, 480);
}

SceneWidget::~SceneWidget()
{
    delete m_handles;
    m_handles = 0;
    delete m_info;
    m_info = 0;
    delete m_renderer;
    m_renderer = 0;
    delete m_grid;
    m_grid = 0;
    delete m_delegate;
    m_delegate = 0;
    delete m_world;
    m_world = 0;
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
    m_renderer->scene()->resetMotion();
    emit sceneDidStop();
}

void SceneWidget::clear()
{
    stop();
    m_loader->release();
}

const vpvl::Scene *SceneWidget::scene() const
{
    return m_renderer->scene();
}

vpvl::PMDModel *SceneWidget::findModel(const QString &name) const
{
    return m_loader->findModel(name);
}

void SceneWidget::setPreferredFPS(int value)
{
    if (value > 0) {
        m_defaultFPS = value;
        m_interval = 1000.0f / value;
        m_world->setPreferredFPS(value);
        m_renderer->scene()->setPreferredFPS(value);
        if (m_internalTimerID) {
            killTimer(m_internalTimerID);
            m_internalTimerID = startTimer(m_interval);
        }
    }
}

vpvl::PMDModel *SceneWidget::selectedModel() const
{
    return m_renderer->selectedModel();
}

void SceneWidget::setSelectedModel(vpvl::PMDModel *value)
{
    hideSelectedModelEdge();
    m_renderer->setSelectedModel(value);
    m_info->setModel(value);
    m_info->update();
    showSelectedModelEdge();
    emit modelDidSelect(value);
}

void SceneWidget::setHandlesVisible(bool value)
{
    m_handles->setVisible(value);
}

void SceneWidget::setInfoPanelVisible(bool value)
{
    m_info->setVisible(value);
}

void SceneWidget::showSelectedModelEdge()
{
    static const vpvl::Color red(1.0f, 0.0f, 0.0f, 1.0f);
    vpvl::PMDModel *model = m_renderer->selectedModel();
    if (model) {
        m_selectedEdgeOffset = model->edgeOffset();
        model->setEdgeOffset(1.0f);
        model->setEdgeColor(red);
        updateMotion();
    }
}

void SceneWidget::hideSelectedModelEdge()
{
    static const vpvl::Color black(0.0f, 0.0f, 0.0f, 1.0f);
    vpvl::PMDModel *model = m_renderer->selectedModel();
    if (model) {
        model->setEdgeOffset(m_selectedEdgeOffset);
        model->setEdgeColor(black);
        updateMotion();
    }
}

void SceneWidget::addModel()
{
    vpvl::PMDModel *model = addModel(openFileDialog("sceneWidget/lastPMDDirectory",
                                                    tr("Open PMD file"),
                                                    tr("PMD file (*.pmd)")));
    if (model && !m_playing) {
        setEmptyMotion(model);
        model->advanceMotion(0.0f);
        emit newMotionDidSet(model);
    }
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
    vpvl::PMDModel *selected = selectedModel();
    if (motion && selected)
        selected->advanceMotion(0.0f);
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
    return insertMotionToModel(path, selectedModel());
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

void SceneWidget::setEmptyMotion(vpvl::PMDModel *model)
{
    if (model) {
        vpvl::VMDMotion *motion = new vpvl::VMDMotion();
        const vpvl::BoneList &bones = model->bones();
        const int nbones = bones.count();
        vpvl::BoneAnimation *boneAnimation = motion->mutableBoneAnimation();
        for (int i = 0; i < nbones; i++) {
            vpvl::Bone *bone = bones[i];
            if (bone->isMovable() || bone->isRotateable()) {
                vpvl::BoneKeyFrame *frame = new vpvl::BoneKeyFrame();
                frame->setDefaultInterpolationParameter();
                frame->setName(bone->name());
                boneAnimation->addKeyFrame(frame);
            }
        }
        const vpvl::FaceList &faces = model->faces();
        const int nfaces = faces.count();
        vpvl::FaceAnimation *faceAnimation = motion->mutableFaceAnimation();
        for (int i = 0; i < nfaces; i++) {
            vpvl::Face *face = faces[i];
            vpvl::FaceKeyFrame *frame = new vpvl::FaceKeyFrame();
            frame->setName(face->name());
            faceAnimation->addKeyFrame(frame);
        }
        m_loader->setModelMotion(motion, model);
        emit motionDidAdd(motion, model);
    }
    else
        QMessageBox::warning(this, tr("The model is not selected."), tr("Select a model to insert the motion"));
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
        QProgressDialog *progress = getProgressDialog("Loading the asset...", 0);
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


void SceneWidget::addAssetFromMetadata()
{
    addAssetFromMetadata(openFileDialog("sceneWidget/lastAssetDirectory",
                                        tr("Open VAC file"),
                                        tr("MMD accessory metadata (*.vac)")));
}

vpvl::Asset *SceneWidget::addAssetFromMetadata(const QString &path)
{
    QFileInfo fi(path);
    vpvl::Asset *asset = 0;
    if (fi.exists()) {
        QProgressDialog *progress = getProgressDialog("Loading the asset...", 0);
        asset = m_loader->loadAssetFromMetadata(fi.fileName(), fi.dir());
        if (asset)
            emit assetDidAdd(asset);
        else
            QMessageBox::warning(this, tr("Loading asset error"),
                                 tr("%1 cannot be loaded").arg(fi.fileName()));
        delete progress;
    }
    return asset;
}

void SceneWidget::saveMetadataFromAsset(vpvl::Asset *asset)
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
    vpvl::PMDModel *model = selectedModel();
    VPDFile *pose = insertPoseToSelectedModel(openFileDialog("sceneWidget/lastVPDDirectory",
                                                             tr("Open VPD file"),
                                                             tr("VPD file (*.vpd)")),
                                              model);
    if (pose && model)
        model->updateImmediate();
}

VPDFile *SceneWidget::insertPoseToSelectedModel(const QString &filename, vpvl::PMDModel *model)
{
    VPDFile *pose = 0;
    if (model) {
        if (QFile::exists(filename)) {
            pose = m_loader->loadModelPose(filename, model);
            if (pose)
                emit modelDidMakePose(pose, model);
            else
                QMessageBox::warning(this, tr("Loading model pose error"),
                                     tr("%1 cannot be loaded").arg(QFileInfo(filename).fileName()));
        }
    }
    else
        QMessageBox::warning(this, tr("The model is not selected."), tr("Select a model to set the pose"));
    return pose;
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
    m_frameIndex = frameIndex;
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
    vpvl::PMDModel *selected = selectedModel();
    emit modelWillDelete(selected);
    if (m_loader->deleteModel(selected)) {
        setSelectedModel(0);
        emit modelDidSelect(0);
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected or exist."),
                             tr("Select a model to delete"));
    }
}

void SceneWidget::deleteAsset(vpvl::Asset *asset)
{
    m_loader->deleteAsset(asset);
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

void SceneWidget::setLightColor(const vpvl::Color &color)
{
    vpvl::Scene *scene = m_renderer->scene();
    scene->setLightSource(color, scene->lightPosition());
    emit lightColorDidSet(color);
}

void SceneWidget::setLightPosition(const vpvl::Vector3 &position)
{
    vpvl::Scene *scene = m_renderer->scene();
    scene->setLightSource(scene->lightColor(), position);
    emit lightPositionDidSet(position);
}

void SceneWidget::setCameraPerspective(vpvl::Vector3 *pos, vpvl::Vector3 *angle, float *fovy, float *distance)
{
    vpvl::Scene *scene = m_renderer->scene();
    vpvl::Vector3 posValue, angleValue;
    float fovyValue, distanceValue;
    posValue = !pos ? scene->position() : *pos;
    angleValue = !angle ? scene->angle() : *angle;
    fovyValue = !fovy ? scene->fovy() : *fovy;
    distanceValue = !distance ? scene->distance() : *distance;
    scene->setCameraPerspective(posValue, angleValue, fovyValue, distanceValue);
    emit cameraPerspectiveDidSet(posValue, angleValue, fovyValue, distanceValue);
}

bool SceneWidget::isGridVisible() const
{
    return m_grid->isEnabled();
}

void SceneWidget::setGridVisible(bool value)
{
    m_grid->setEnable(value);
}

void SceneWidget::setPhysicsEnable(bool value)
{
    m_renderer->scene()->setWorld(value ? m_world->mutableWorld() : 0);
    m_enablePhysics = value;
}

const QPointF SceneWidget::objectCoordinates(const QPointF &input) const
{
    // This implementation based on the below page.
    // http://softwareprodigy.blogspot.com/2009/08/gluunproject-for-iphone-opengl-es.html
    vpvl::Scene *scene = m_renderer->scene();
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
    vpvl::Vector3 camera(cx, cy, cz), zfar(fx, fy, fz), pointInPlane(0, 0, 0), planeNormal(0, 0, -1);
    zfar -= camera;
    zfar /= camera.length();
    pointInPlane -= camera;
    zfar *= planeNormal.dot(pointInPlane) / planeNormal.dot(zfar);
    QPointF output;
    output.setX(zfar.x() + camera.x());
    output.setY(zfar.y() + camera.y());
    return output;
}

void SceneWidget::setBones(const QList<vpvl::Bone *> &bones)
{
    vpvl::Bone *bone = bones.isEmpty() ? 0 : bones.last();
    m_info->setBone(bone);
    m_info->update();
    if (bone) {
        m_handles->setMovable(bone->isMovable());
        m_handles->setRotateable(bone->isRotateable());
    }
    else {
        m_handles->setMovable(false);
        m_handles->setRotateable(false);
    }
    m_bone = bone;
}

void SceneWidget::rotate(float x, float y)
{
    vpvl::Scene *scene = m_renderer->scene();
    vpvl::Vector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    angle.setValue(angle.x() + x, angle.y() + y, angle.z());
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::translate(float x, float y)
{
    vpvl::Scene *scene = m_renderer->scene();
    vpvl::Vector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    pos.setValue(pos.x() + x, pos.y() + y, pos.z());
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::translateModel(float x, float y)
{
    vpvl::PMDModel *model = selectedModel();
    if (model) {
        vpvl::Vector3 p = model->positionOffset();
        p.setValue(p.x() + x, p.y(), p.z() + y);
        model->setPositionOffset(p);
        model->updateImmediate();
    }
}

void SceneWidget::resetModelPosition()
{
    vpvl::PMDModel *model = selectedModel();
    if (model) {
        model->setPositionOffset(vpvl::Vector3(0.0f, 0.0f, 0.0f));
        model->updateImmediate();
    }
}

void SceneWidget::loadFile(const QString &file)
{
    if (file.endsWith(".pmd", Qt::CaseInsensitive)) {
        vpvl::PMDModel *model = addModel(file);
        if (model && !m_playing) {
            setEmptyMotion(model);
            model->advanceMotion(0.0f);
            emit newMotionDidSet(model);
        }
    }
    else if (file.endsWith(".vmd", Qt::CaseInsensitive)) {
        vpvl::VMDMotion *motion = insertMotionToModel(file, selectedModel());
        if (motion)
            advanceMotion(0.0f);
    }
    else if (file.endsWith(".x", Qt::CaseInsensitive)) {
        addAsset(file);
    }
    else if (file.endsWith(".vpd", Qt::CaseInsensitive)) {
        vpvl::PMDModel *model = selectedModel();
        VPDFile *pose = insertPoseToSelectedModel(file, model);
        if (pose && model)
            model->updateImmediate();
    }
    else if (file.endsWith(".vac", Qt::CaseInsensitive)) {
        addAssetFromMetadata(file);
    }
}

void SceneWidget::zoom(bool up, const Qt::KeyboardModifiers &modifiers)
{
    vpvl::Scene *scene = m_renderer->scene();
    vpvl::Vector3 pos = scene->position(), angle = scene->angle();
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
    m_settings->setValue("sceneWidget/isBoneWireframeVisible", m_visibleBones);
    m_settings->setValue("sceneWidget/isGridVisible", m_grid->isEnabled());
    m_settings->setValue("sceneWidget/isPhysicsEnabled", m_enablePhysics);
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
        foreach (const QUrl url, urls) {
            const QString &file = url.toLocalFile();
            loadFile(file);
            qDebug() << "Proceeded a dropped file:" << file;
        }
    }
}

void SceneWidget::initializeGL()
{
    GLenum err;
    if (!Renderer::initializeGLEW(err))
        qFatal("Cannot initialize GLEW: %s", glewGetErrorString(err));
    else
        qDebug("GLEW version: %s", glewGetString(GLEW_VERSION));
    qDebug("VPVL version: %s (%d)", VPVL_VERSION_STRING, VPVL_VERSION);
    qDebug("GL_VERSION: %s", glGetString(GL_VERSION));
    qDebug("GL_VENDOR: %s", glGetString(GL_VENDOR));
    qDebug("GL_RENDERER: %s", glGetString(GL_RENDERER));
    m_renderer = new Renderer(m_delegate, width(), height(), m_defaultFPS);
    m_loader = new SceneLoader(m_renderer);
    m_debugDrawer->setWorld(m_world->mutableWorld());
    m_grid->initialize();
#ifdef VPVL_USE_GLSL
    m_renderer->createPrograms();
#endif
    vpvl::Scene *scene = m_renderer->scene();
    scene->setViewMove(0);
    const vpvl::Color &color = scene->lightColor();
#if 0 // MMD like toon
    const vpvl::Scalar &intensity = 0.6f;
    const vpvl::Vector3 &a = color * intensity * 2.0f;
    const vpvl::Vector3 &d = color * 0.0f;
    const vpvl::Vector3 &s = color * intensity;
#else // no toon
    const vpvl::Vector3 &a = color;
    const vpvl::Vector3 &d = color;
    const vpvl::Vector3 &s = color;
#endif
    const vpvl::Color ambient(a.x(), a.y(), a.z(), 1.0f);
    const vpvl::Color diffuse(d.x(), d.y(), d.z(), 1.0f);
    const vpvl::Color specular(s.x(), s.y(), s.z(), 1.0f);
    scene->setLightComponent(ambient, diffuse, specular);
    if (m_playing || m_enablePhysics)
        setPhysicsEnable(true);
    m_timer.start();
    m_internalTimerID = startTimer(m_interval);
    m_handles->load();
    m_info->load();
    m_info->setModel(0);
    m_info->setBone(0);
    m_info->setFPS(0.0f);
    m_info->update();
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
    const QPoint &pos = event->pos();
    QRectF rect;
    m_prevPos = pos;
    int flag;
    if (m_handles->testHit(pos, flag, rect)) {
        switch (flag) {
        case Handles::kLocal:
            m_handles->setLocal(false);
            emit globalTransformDidSelect();
            break;
        case Handles::kGlobal:
            m_handles->setLocal(true);
            emit localTransformDidSelect();
            break;
        default:
            break;
        }
    }
    m_handleFlags = flag;
}

void SceneWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    vpvl::PMDModel *model = selectedModel();
    if (model) {
        const QPointF &pos = objectCoordinates(event->posF());
        const vpvl::BoneList &bones = model->bones();
        const int nbones = bones.count();
        vpvl::Vector3 origin(pos.x(), pos.y(), 0.0f);
        vpvl::Bone *nearestBone = 0;
        vpvl::Scalar nearestDistance = 10000.0f;
        for (int i = 0; i < nbones; i++) {
            vpvl::Bone *bone = bones[i];
            vpvl::Vector3 boneOrigin = bone->localTransform().getOrigin();
            boneOrigin.setZ(0.0f);
            vpvl::Scalar distance = boneOrigin.distance(origin);
            if (distance < nearestDistance) {
                nearestBone = bone;
                nearestDistance = distance;
            }
        }
        if (nearestBone && nearestBone->isVisible()) {
            const vpvl::Vector3 &origin = nearestBone->localTransform().getOrigin();
            qDebug() << "nearest:" << internal::toQString(nearestBone)
                     << "mouse:" << pos
                     << "origin:" << QPointF(origin.x(), origin.y());
            QList<vpvl::Bone *> bones;
            bones.append(nearestBone);
            emit boneDidSelect(bones);
        }
        m_doubleClickedPos = origin;
        event->ignore();
    }
}

void SceneWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        QPoint diff = event->pos() - m_prevPos;
        if (m_handleFlags & Handles::kEnable) {
            if (m_handleFlags & Handles::kMove) {
                const float value = diff.y() * 0.1f;
                if (m_handleFlags & Handles::kX)
                    emit handleDidMove('X', value);
                else if (m_handleFlags & Handles::kY)
                    emit handleDidMove('Y', value);
                else if (m_handleFlags & Handles::kZ)
                    emit handleDidMove('Z', value);
            }
            else if (m_handleFlags & Handles::kRotate) {
                const float value = vpvl::radian(diff.y() * 0.1f);
                if (m_handleFlags & Handles::kX)
                    emit handleDidRotate('X', value);
                else if (m_handleFlags & Handles::kY)
                    emit handleDidRotate('Y', value);
                else if (m_handleFlags & Handles::kZ)
                    emit handleDidRotate('Z', value);
            }
        }
        else if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            vpvl::Scene *scene = m_renderer->scene();
            vpvl::Vector3 position = scene->lightPosition();
            vpvl::Quaternion rx(0.0f, diff.y() * vpvl::radian(0.1f), 0.0f),
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

void SceneWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
}

void SceneWidget::paintGL()
{
    QPainter painter(this);
    qglClearColor(Qt::white);
    glEnable(GL_MULTISAMPLE);
    m_renderer->initializeSurface();
    m_renderer->clearSurface();
    m_renderer->drawSurface();
    m_grid->draw(m_renderer->scene());
    drawBones();
    painter.beginNativePainting();
    m_handles->draw();
    m_info->draw();
    painter.endNativePainting();
    emit motionDidFinished(m_loader->stoppedMotions());
}

void SceneWidget::resizeGL(int w, int h)
{
    m_renderer->resize(w, h);
    m_handles->resize(w, h);
    m_info->resize(w, h);
}

void SceneWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_internalTimerID) {
        vpvl::Scene *scene = m_renderer->scene();
        scene->updateModelView(0);
        scene->updateProjection(0);
        if (m_playing) {
            float elapsed = m_timer.elapsed() / static_cast<float>(vpvl::Scene::kFPS);
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

void SceneWidget::drawBones()
{
    if (m_visibleBones) {
        m_debugDrawer->drawModelBones(selectedModel(), true, true);
        m_debugDrawer->drawPosition(m_doubleClickedPos);
    }
    m_debugDrawer->drawBoneTransform(m_bone);
}

void SceneWidget::updateFPS()
{
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

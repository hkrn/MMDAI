/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2013  hkrn                                    */
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
#include <vpvl2/qt/CustomGLContext.h>
#include <vpvl2/qt/RenderContext.h>
#include <vpvl2/qt/TextureDrawHelper.h>
#include <vpvl2/qt/Util.h>

#include "SceneWidget.h"

#include "Application.h"
#include "BackgroundImage.h"
#include "DebugDrawer.h"
#include "Grid.h"
#include "Handles.h"
#include "InfoPanel.h"
#include "LoggerWidget.h"
#include "SceneLoader.h"

#include <QtGui/QtGui>
#include <btBulletDynamicsCommon.h>
#include <glm/gtc/matrix_transform.hpp>

#ifdef Q_OS_DARWIN
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif /* Q_OS_DARWIN */

namespace vpvm
{

using namespace vpvl2;

static void UIAlertMVDMotion(const IMotionSharedPtr motion, SceneWidget *parent)
{
    /* MVD ファイルを読み込んだ時プロジェクト保存が出来無いことを伝えるメッセージを出す */
    if (motion->type() == IMotion::kMVDMotion) {
        Util::warning(parent,
                      QApplication::tr("The MVD file cannot save to the project currently"),
                      QApplication::tr("The MVD file cannot save to the project. If you want to export the MVD file, "
                                       "You can use \"Save model motion.\" instead. Sorry for inconvenience."));
    }
}

class SceneWidget::PlaneWorld {
public:
    static const Vector3 kWorldAabbSize;

    PlaneWorld()
        : m_dispatcher(&m_config),
          m_broadphase(-kWorldAabbSize, kWorldAabbSize),
          m_world(&m_dispatcher, &m_broadphase, &m_solver, &m_config),
          m_state(new btDefaultMotionState()),
          m_shape(new btBoxShape(btVector3(kWorldAabbSize.x(), kWorldAabbSize.y(), 0.01))),
          m_body(new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, m_state.data(), m_shape.data())))
    {
        m_world.addRigidBody(m_body.data());
    }
    ~PlaneWorld()
    {
        m_world.removeRigidBody(m_body.data());
    }

    void updateTransform(const Transform &value) {
        m_body->setWorldTransform(value);
        m_world.stepSimulation(1);
    }
    void draw(const SceneLoader *loader, DebugDrawer *drawer) {
        const btTransform &worldTransform = m_body->getWorldTransform();
        m_world.setDebugDrawer(drawer);
        drawer->drawShape(&m_world, m_shape.data(), loader, worldTransform, btVector3(1, 0, 0));
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
    QScopedPointer<btDefaultMotionState> m_state;
    QScopedPointer<btBoxShape> m_shape;
    QScopedPointer<btRigidBody> m_body;
};

const Vector3 SceneWidget::PlaneWorld::kWorldAabbSize = Vector3(10000, 10000, 10000);

SceneWidget::SceneWidget(const QGLFormat format,
                         IEncoding *encoding,
                         Factory *factory,
                         QSettings *settings,
                         QWidget *parent)
    : QGLWidget(new qt::CustomGLContext(format), parent),
      m_settingsRef(settings),
      m_plane(new PlaneWorld()),
      m_encodingRef(encoding),
      m_factoryRef(factory),
      m_currentSelectedBoneRef(0),
      m_editMode(kSelect),
      m_lastBonePosition(kZeroV3),
      m_totalDelta(0),
      m_timeIndex(0),
      m_lastDistance(0),
      m_prevElapsed(0),
      m_frameCount(0),
      m_currentFPS(0),
      m_handleFlags(0),
      m_playing(false),
      m_enableBoneMove(false),
      m_enableBoneRotate(false),
      m_showModelDialog(false),
      m_lockTouchEvent(false),
      m_enableGestures(false),
      m_enableMoveGesture(false),
      m_enableRotateGesture(false),
      m_enableScaleGesture(false),
      m_enableUndoGesture(false),
      m_enableUpdateGL(true),
      m_isImageHandleRectIntersect(false)
{
    connect(static_cast<Application *>(qApp), SIGNAL(fileDidRequest(QString)), this, SLOT(loadFile(QString)));
    connect(this, SIGNAL(cameraPerspectiveDidSet(const ICamera*)),
            this, SLOT(updatePlaneWorld(const ICamera*)));
    setAcceptDrops(true);
    setAutoFillBackground(false);
    setShowModelDialog(m_settingsRef->value("sceneWidget/showModelDialog", true).toBool());
    /* 通常はマウスを動かしても mouseMove が呼ばれないため、マウスが動いたら常時 mouseEvent を呼ぶようにする */
    setMouseTracking(true);
    setGesturesEnable(m_settingsRef->value("sceneWidget/enableGestures", false).toBool());
}

SceneWidget::~SceneWidget()
{
}

SceneLoader *SceneWidget::sceneLoaderRef() const
{
    return m_loader.data();
}

void SceneWidget::play()
{
    m_playing = true;
    m_refreshTimer.restart();
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
    revertSelectedModel();
    m_loader->releaseProject();
    m_loader->newProject();
}

void SceneWidget::startAutomaticRendering()
{
    if (!m_updateTimer.isActive()) {
        m_updateTimer.start(int(1000.0 / Scene::defaultFPS()), this);
    }
}

void SceneWidget::stopAutomaticRendering()
{
    if (m_updateTimer.isActive()) {
        m_updateTimer.stop();
    }
}

void SceneWidget::loadProject(const QString &filename)
{
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
    /* ハンドルの遅延読み込み */
    m_handles->loadModelHandles();
    m_enableUpdateGL = true;
    seekMotion(0, true, true);
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
        m_loader->setPreferredFPS(value);
        if (m_updateTimer.isActive()) {
            stopAutomaticRendering();
            startAutomaticRendering();
        }
    }
}

void SceneWidget::setSelectedModel(IModelSharedPtr value, EditMode mode)
{
    if (!value) {
        clearSelectedBones();
        clearSelectedMorphs();
    }
    /* 情報パネルに選択されたモデルの名前を更新する */
    m_loader->setSelectedModel(value);
    m_info->setModel(value.data());
    m_info->update();
    setEditMode(mode);
}

void SceneWidget::setBackgroundImage(const QString &filename)
{
    m_background->setImage(filename);
}

void SceneWidget::setModelEdgeOffset(double value)
{
    IModelSharedPtr model = m_loader->selectedModelRef();
    if (model) {
        m_loader->setModelEdgeOffset(model, static_cast<float>(value));
        m_loader->sceneRef()->updateModel(model.data());
    }
    refreshMotions();
}

void SceneWidget::setModelOpacity(const Scalar &value)
{
    IModelSharedPtr model = m_loader->selectedModelRef();
    if (model) {
        m_loader->setModelOpacity(model, value);
    }
    refreshMotions();
}

void SceneWidget::setModelPositionOffset(const Vector3 &value)
{
    IModelSharedPtr model = m_loader->selectedModelRef();
    if (model) {
        m_loader->setModelPosition(model, value);
    }
    refreshMotions();
}

void SceneWidget::setModelRotationOffset(const Vector3 &value)
{
    IModelSharedPtr model = m_loader->selectedModelRef();
    if (model) {
        m_loader->setModelRotation(model, value);
    }
    refreshMotions();
}

void SceneWidget::setModelProjectiveShadowEnable(bool value)
{
    IModelSharedPtr model = m_loader->selectedModelRef();
    if (model) {
        m_loader->setProjectiveShadowEnable(model.data(), value);
    }
    refreshMotions();
}

void SceneWidget::setModelSelfShadowEnable(bool value)
{
    IModelSharedPtr model = m_loader->selectedModelRef();
    if (model) {
        m_loader->setSelfShadowEnable(model.data(), value);
    }
    refreshMotions();
}

void SceneWidget::setModelOpenSkinningEnable(bool value)
{
    IModelSharedPtr model = m_loader->selectedModelRef();
    if (model) {
        m_loader->setSelfShadowEnable(model.data(), value);
    }
    refreshMotions();
}

void SceneWidget::setModelVertexShaderSkinningType1Enable(bool value)
{
    IModelSharedPtr model = m_loader->selectedModelRef();
    if (model) {
        m_loader->setSelfShadowEnable(model.data(), value);
    }
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

void SceneWidget::addFile()
{
    loadFile(Util::openFileDialog("sceneWidget/lastFileDirectory",
                                  tr("Open a file"),
                                  tr("Model file (*.pmd *.pmx *.zip);;"
                                     "Accessory file (*.x);;"
                                     "Model motion file (*.vmd *.mvd);;"
                                     "Effect (*.cgfx);;"
                                     "Pose file (*.vpd);;"
                                     "Accessory metadata file (*.vac)"),
                                  m_settingsRef));
}

void SceneWidget::addModel()
{
    /* モデル追加と共に空のモーションを作成する */
    loadModel(Util::openFileDialog("sceneWidget/lastModelDirectory",
                                   tr("Open PMD/PMX file"),
                                   tr("Model file (*.pmd *.pmx *.zip)"),
                                   m_settingsRef));
}

void SceneWidget::loadModel(const QString &path, bool skipDialog)
{
    QFileInfo finfo(path);
    bool didLoad = true;
    if (finfo.exists()) {
        IModelSharedPtr modelPtr;
        ArchiveSmartPtr archive(new Archive(m_encodingRef));
        Archive::EntryNames allFilesInArchiveRaw;
        /* zip を解凍 */
        emit fileDidOpenProgress(tr("Loading %1").arg(finfo.baseName()), false);
        emit fileDidUpdateProgress(0, 0, tr("Loading %1...").arg(finfo.baseName()));
        const String archivePath(Util::fromQString(path));
        if (archive->open(&archivePath, allFilesInArchiveRaw)) {
            const QStringList &allFilesInArchive = SceneLoader::toStringList(allFilesInArchiveRaw);
            const QStringList &modelsInArchive = allFilesInArchive.filter(SceneLoader::kModelExtensions);
            m_renderContext->setArchive(archive.get());
            /* zip 内に pmd/pmx ファイルがあり、全てのファイルが解凍に成功した場合はそれらのモデルを全て読み出す処理に移動する */
            if (!modelsInArchive.isEmpty() && archive->uncompress(SceneLoader::toSet(allFilesInArchive.filter(SceneLoader::kModelLoadable)))) {
                QUuid uuid;
                IModel::Type type;
                QFileInfo modelFileInfoInArchive;
                int nmodels = modelsInArchive.size(), i = 0;
                emit fileDidUpdateProgress(0, nmodels, tr("Loading %1 (%2 of %3)...").arg(finfo.baseName()).arg(0).arg(nmodels));
                foreach (const QString &modelInArchive, modelsInArchive) {
                    if (const std::string *bytesRef = archive->data(Util::fromQString(modelInArchive))) {
                        modelFileInfoInArchive.setFile(modelInArchive);
                        /* zip 内のパスを zip までのファイルパスに置換する。これは qt::RenderContext で読み出せるようにするため */
                        archive->replaceFilePath(Util::fromQString(modelFileInfoInArchive.path()), Util::fromQString(finfo.path()) + "/");
                        type = modelFileInfoInArchive.suffix() == "pmx" ? IModel::kPMXModel : IModel::kPMDModel;
                        const QByteArray &bytes = QByteArray::fromRawData(bytesRef->data(), bytesRef->size());
                        if (m_loader->loadModel(bytes, type, modelPtr) &&
                                (skipDialog || (!m_showModelDialog || acceptAddingModel(modelPtr.data())))) {
                            /* ハンドルの遅延読み込み */
                            m_handles->loadModelHandles();
                            m_loader->addModel(modelPtr, finfo, modelFileInfoInArchive, uuid);
                            setEmptyMotion(modelPtr, false);
                        }
                    }
                    ++i;
                    emit fileDidUpdateProgress(i, nmodels, tr("Loading %1 (%2 of %3)...")
                                               .arg(modelFileInfoInArchive.baseName()).arg(i).arg(nmodels));
                    /* 元の zip 内のファイルパスに戻して再度置換できるようにする */
                    archive->restoreOriginalEntries();
                }
            }
            m_renderContext->setArchive(0);
        }
        /* 通常のモデル読み込み処理 */
        else if (m_loader->loadModel(path, modelPtr)) {
            if (skipDialog || (!m_showModelDialog || acceptAddingModel(modelPtr.data()))) {
                QUuid uuid;
                /* ハンドルの遅延読み込み */
                m_handles->loadModelHandles();
                m_loader->addModel(modelPtr, finfo, QFileInfo(), uuid);
                setEmptyMotion(modelPtr, false);
            }
        }
        else {
            didLoad = false;
        }
        emit fileDidLoad(path, didLoad);
        if (!didLoad) {
            Util::warning(this, tr("Loading model error"), tr("%1 cannot be loaded").arg(finfo.fileName()));
        }
    }
}

void SceneWidget::insertMotionToAllModels()
{
    /* モーションを追加したら即座に反映させるために updateMotion を呼んでおく */
    IMotionSharedPtr motionPtr;
    loadMotionToAllModels(Util::openFileDialog("sceneWidget/lastModelMotionDirectory",
                                               tr("Load model motion from a VMD/MVD file"),
                                               tr("Model motion file (*.vmd *.mvd)"),
                                               m_settingsRef),
                          motionPtr);
    if (motionPtr) {
        refreshMotions();
    }
}

void SceneWidget::loadMotionToAllModels(const QString &path, IMotionSharedPtr motionPtr)
{
    if (QFile::exists(path)) {
        QList<IModelSharedPtr> models;
        emit fileDidOpenProgress(tr("Loading %1").arg(path), false);
        emit fileDidUpdateProgress(0, 0, tr("Loading %1...").arg(path));
        m_loader->loadModelMotion(path, models, motionPtr);
        UIAlertMVDMotion(motionPtr, this);
        seekMotion(0, false, true);
        emit fileDidLoad(path, true);
    }
    else if (!path.isEmpty()) {
        Util::warning(this, tr("Loading model motion error"),
                      tr("%1 cannot be loaded").arg(QFileInfo(path).fileName()));
    }
}

void SceneWidget::insertMotionToSelectedModel()
{
    if (m_loader->selectedModelRef()) {
        IMotionSharedPtr motionPtr;
        loadMotionToSelectedModel(Util::openFileDialog("sceneWidget/lastModelMotionDirectory",
                                                       tr("Load model motion from a VMD/MVD file"),
                                                       tr("Model motion file (*.vmd *.mvd)"),
                                                       m_settingsRef),
                                  motionPtr);
        if (motionPtr) {
            UIAlertMVDMotion(motionPtr, this);
            refreshMotions();
        }
    }
    else {
        Util::warning(this, tr("The model is not selected."),
                      tr("Select a model to insert the motion (\"Model\" > \"Select model\")"));
    }
}

void SceneWidget::loadMotionToSelectedModel(const QString &path, IMotionSharedPtr motionPtr)
{
    loadMotionToModel(path, m_loader->selectedModelRef(), motionPtr);
}

void SceneWidget::loadMotionToModel(const QString &path, IModelSharedPtr model, IMotionSharedPtr motionPtr)
{
    if (model) {
        if (QFile::exists(path)) {
            if (m_loader->loadModelMotion(path, motionPtr)) {
                /* 違うモデルに適用しようとしているかどうかの確認 */
                if (!model->name()->equals(motionPtr->name())) {
                    int ret = Util::warning(0,
                                            tr("Applying this motion to the different model"),
                                            tr("This motion is created for %1. Do you apply this motion to %2?")
                                            .arg(Util::toQStringFromMotion(motionPtr.data()))
                                            .arg(Util::toQStringFromModel(model.data())),
                                            "",
                                            QMessageBox::Yes|QMessageBox::No);
                    if (ret == QMessageBox::Yes) {
                        loadModelMotion(motionPtr, path, model);
                    }
                }
                else {
                    loadModelMotion(motionPtr, path, model);
                }
                seekMotion(0, false, true);
                emit fileDidLoad(path, true);
            }
            else {
                Util::warning(this, tr("Loading model motion error"),
                              tr("%1 cannot be loaded").arg(QFileInfo(path).fileName()));
            }
        }
    }
}

void SceneWidget::setEmptyMotion()
{
    setEmptyMotion(m_loader->selectedModelRef(), false);
}

void SceneWidget::setEmptyMotion(IModelSharedPtr model, bool skipWarning)
{
    if (model && !m_playing) {
        IMotionSharedPtr motion;
        m_loader->newModelMotion(model, motion);
        m_loader->setModelMotion(motion, model);
        emit newMotionDidSet(model);
    }
    else if (!skipWarning) {
        Util::warning(this,
                      tr("The model is not selected."),
                      tr("Select a model to insert the motion (\"Model\" > \"Select model\")"));
    }
}

void SceneWidget::addAsset()
{
    loadAsset(Util::openFileDialog("sceneWidget/lastAssetDirectory",
                                   tr("Open accessory file"),
                                   tr("Accessory file (*.x *.zip)"),
                                   m_settingsRef));
}

void SceneWidget::loadAsset(const QString &path)
{
    QFileInfo finfo(path);
    bool didLoad = true;
    if (finfo.exists()) {
        ArchiveSmartPtr archive(new Archive(m_encodingRef));
        QUuid uuid;
        IModelSharedPtr assetPtr;
        Archive::EntryNames allFilesInArchiveRaw;
        /* zip を解凍 */
        emit fileDidOpenProgress(tr("Loading %1").arg(finfo.baseName()), false);
        emit fileDidUpdateProgress(0, 0, tr("Loading %1...").arg(finfo.baseName()));
        const String archivePath(Util::fromQString(path));
        if (archive->open(&archivePath, allFilesInArchiveRaw)) {
            const QStringList &allFilesInArchive = SceneLoader::toStringList(allFilesInArchiveRaw);
            const QStringList &assetsInArchive = allFilesInArchive.filter(SceneLoader::kAssetExtensions);
            /* zip 内に x ファイルがあり、全てのファイルが解凍に成功した場合はそれらのモデルを全て読み出す処理に移動する */
            if (!assetsInArchive.isEmpty() && archive->uncompress(SceneLoader::toSet(allFilesInArchive.filter(SceneLoader::kAssetLoadable)))) {
                QFileInfo assetFileInfoInArchive, archiveFileInfo(path);
                int nmodels = assetsInArchive.size(), i = 0;
                m_renderContext->setArchive(archive.get());
                emit fileDidUpdateProgress(0, nmodels, tr("Loading %1 (%2 of %3)...").arg(finfo.baseName()).arg(0).arg(nmodels));
                foreach (const QString &assetInArchive, assetsInArchive) {
                    if (const std::string *bytesRef = archive->data(Util::fromQString(assetInArchive))) {
                        const QByteArray &bytes = QByteArray::fromRawData(bytesRef->data(), bytesRef->size());
                        assetFileInfoInArchive.setFile(assetInArchive);
                        /* zip 内のパスを zip までのファイルパスに置換する。これは qt::RenderContext で読み出せるようにするため */
                        archive->replaceFilePath(Util::fromQString(assetFileInfoInArchive.path()), Util::fromQString(archiveFileInfo.path()) + "/");
                        if (m_loader->loadAsset(bytes, finfo, assetFileInfoInArchive, uuid, assetPtr)) {
                            /* ハンドルの遅延読み込み */
                            m_handles->loadModelHandles();
                            setEmptyMotion(assetPtr, false);
                        }
                    }
                    ++i;
                    emit fileDidUpdateProgress(i, nmodels, tr("Loading %1 (%2 of %3)...").arg(finfo.baseName()).arg(i).arg(nmodels));
                    archive->restoreOriginalEntries();
                }
                m_renderContext->setArchive(0);
            }
        }
        /* 通常のファイル読み込み */
        else if (m_loader->loadAsset(path, uuid, assetPtr)) {
            /* ハンドルの遅延読み込み */
            m_handles->loadModelHandles();
            setEmptyMotion(assetPtr, false);
        }
        /* 読み込み失敗 */
        else {
            didLoad = false;
        }
        emit fileDidLoad(path, didLoad);
        if (!didLoad) {
            Util::warning(this, tr("Loading asset error"), tr("%1 cannot be loaded").arg(finfo.fileName()));
        }
    }
}

void SceneWidget::addAssetFromMetadata()
{
    loadAssetFromMetadata(Util::openFileDialog("sceneWidget/lastAssetDirectory",
                                               tr("Open VAC file"),
                                               tr("MMD accessory metadata (*.vac)"),
                                               m_settingsRef));
}

void SceneWidget::loadAssetFromMetadata(const QString &path)
{
    QFileInfo fi(path);
    if (fi.exists()) {
        QUuid uuid;
        IModelSharedPtr asset;
        if (m_loader->loadAssetFromMetadata(fi.fileName(), fi.dir(), uuid, asset)) {
            /* ハンドルの遅延読み込み */
            m_handles->loadModelHandles();
            setFocus();
        }
        else {
            Util::warning(this, tr("Loading asset error"),
                          tr("%1 cannot be loaded").arg(fi.fileName()));
        }
    }
}

void SceneWidget::saveMetadataFromAsset(IModelSharedPtr asset)
{
    if (asset) {
        QString filename = QFileDialog::getSaveFileName(this, tr("Save %1 as VAC file")
                                                        .arg(Util::toQStringFromModel(asset.data())), "",
                                                        tr("MMD accessory metadata (*.vac)"));
        m_loader->saveMetadataFromAsset(filename, asset);
    }
}

void SceneWidget::insertPoseToSelectedModel()
{
    IModelSharedPtr model = m_loader->selectedModelRef();
    VPDFilePtr ptr = insertPoseToSelectedModel(Util::openFileDialog("sceneWidget/lastPoseDirectory",
                                                                    tr("Open VPD file"),
                                                                    tr("VPD file (*.vpd)"),
                                                                    m_settingsRef),
                                               model);
    if (!ptr.isNull())
        m_loader->sceneRef()->updateModel(model.data());
}

void SceneWidget::setBackgroundImage()
{

    const QString &filename = Util::openFileDialog("sceneWidget/lastBackgroundImageDirectory",
                                                   tr("Open background image file"),
                                               #ifdef Q_OS_MACX
                                                   tr("Image file (*.bmp *.jpg *.gif *.png *.tif);; Movie file (*.avi *.m4v *.mp4 *.mov *.mng)"),
                                               #else
                                                   tr("Image file (*.bmp *.jpg *.gif *.png *.tif);; Movie file (*.mng)"),
                                               #endif
                                                   m_settingsRef);
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

VPDFilePtr SceneWidget::insertPoseToSelectedModel(const QString &filename, IModelSharedPtr model)
{
    VPDFilePtr ptr;
    if (model) {
        if (QFile::exists(filename)) {
            ptr = m_loader->loadModelPose(filename, model);
            if (ptr.isNull()) {
                Util::warning(this, tr("Loading model pose error"),
                              tr("%1 cannot be loaded").arg(QFileInfo(filename).fileName()));
            }
        }
    }
    else {
        Util::warning(this,
                      tr("The model is not selected."),
                      tr("Select a model to set the pose (\"Model\" > \"Select model\")"));
    }
    return ptr;
}

void SceneWidget::advanceMotion(const IKeyframe::TimeIndex &delta)
{
    if (delta <= 0)
        return;
    Scene *scene = m_loader->sceneRef();
    scene->advance(delta, Scene::kUpdateAll);
    scene->update(Scene::kUpdateAll);
    if (m_loader->isPhysicsEnabled())
        m_loader->worldRef()->stepSimulation(delta);
    updateScene();
}

void SceneWidget::seekMotion(const IKeyframe::TimeIndex &timeIndex, bool forceCameraUpdate, bool forceEvenSame)
{
    /*
       渡された値が同じフレーム位置の場合は何もしない
       (シグナルスロット処理の関係でモーフスライダーが動かなくなってしまうため)
     */
    if (!forceCameraUpdate && !forceEvenSame && timeIndex == m_timeIndex)
        return;
    /*
       advanceMotion に似ているが、前のフレームインデックスを利用することがあるので、保存しておく必要がある。
       force でカメラと照明を強制的に動かすことが出来る(例として場面タブからシークした場合)。
     */
    Scene *scene = m_loader->sceneRef();
    int flags = forceCameraUpdate ? Scene::kUpdateAll : Scene::kUpdateModels | Scene::kUpdateRenderEngines;
    scene->seek(timeIndex, flags);
    scene->update(flags);
    m_timeIndex = timeIndex;
    m_background->setTimeIndex(timeIndex);
    emit motionDidSeek(timeIndex);
    updateScene();
}

void SceneWidget::resetMotion()
{
    Scene *scene = m_loader->sceneRef();
    Array<IMotion *> motions;
    scene->getMotionRefs(motions);
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        IMotion *motion = motions[i];
        motion->reset();
    }
    m_timeIndex = 0;
    m_background->setTimeIndex(0);
    scene->seek(0, Scene::kUpdateAll);
    scene->update(Scene::kUpdateAll);
    emit motionDidSeek(0.0f);
    updateScene();
}

void SceneWidget::setCamera()
{
    IMotionSharedPtr motion = setCamera(Util::openFileDialog("sceneWidget/lastCameraMotionDirectory",
                                                             tr("Load camera motion from a VMD/MVD file"),
                                                             tr("Camera motion file (*.vmd *.mvd)"),
                                                             m_settingsRef));
    if (motion) {
        UIAlertMVDMotion(motion, this);
        refreshScene();
    }
}

IMotionSharedPtr SceneWidget::setCamera(const QString &path)
{
    IMotionSharedPtr motionPtr;
    if (QFile::exists(path)) {
        if (m_loader->loadCameraMotion(path, motionPtr)) {
            emit fileDidLoad(path, true);
        }
        else {
            Util::warning(this, tr("Loading camera motion error"),
                          tr("%1 cannot be loaded").arg(QFileInfo(path).fileName()));
        }
    }
    return motionPtr;
}

void SceneWidget::deleteSelectedModel()
{
    IModelSharedPtr selected = m_loader->selectedModelRef();
    if (selected) {
        int ret = Util::warning(0,
                                tr("Confirm"),
                                tr("Do you want to delete the model \"%1\"? This cannot undo.")
                                .arg(Util::toQStringFromModel(selected.data())),
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
    ICamera *camera = m_loader->sceneRef()->camera();
    camera->resetDefault();
    emit cameraPerspectiveDidSet(camera);
}

void SceneWidget::setCameraPerspective(const QSharedPointer<ICamera> &camera)
{
    ICamera *c1 = camera.data(), *c2 = m_loader->sceneRef()->camera();
    c2->copyFrom(c1);
    emit cameraPerspectiveDidSet(c2);
}

void SceneWidget::makeRay(const QPointF &input, Vector3 &rayFrom, Vector3 &rayTo) const
{
    // This implementation based on the below page.
    // http://softwareprodigy.blogspot.com/2009/08/gluunproject-for-iphone-opengl-es.html
    glm::mat4 world, view, projection;
    glm::vec2 win(input.x(), height() - input.y());
    m_loader->getCameraMatrices(world, view, projection);
    const glm::vec4 viewport(0, 0, width(), height());
    ICamera *camera = m_loader->sceneRef()->camera();
    /* projection の値は無限望遠で rayTo の値が inf になってしまうので、上書きする */
    projection = glm::perspective(camera->fov(), width() / glm::mediump_float(height()), 0.1f, 10000.0f);
    const glm::vec3 &cnear = glm::unProject(glm::vec3(win, 0), view * world, projection, viewport);
    rayFrom.setValue(cnear.x, cnear.y, cnear.z);
    const glm::vec3 &cfar = glm::unProject(glm::vec3(win, 1), view * world, projection, viewport);
    rayTo.setValue(cfar.x, cfar.y, cfar.z);
}

void SceneWidget::selectBones(const QList<IBone *> &bones)
{
    /* signal/slot による循環参照防止 */
    if (!Util::compare(bones, m_selectedBoneRefs)) {
        m_info->setBones(bones, tr("(multiple)"));
        m_info->update();
        m_handles->setBone(bones.isEmpty() ? 0 : bones.first());
        m_selectedBoneRefs = bones;
        emit bonesDidSelect(bones);
    }
}

void SceneWidget::selectMorphs(const QList<IMorph *> &morphs)
{
    /* signal/slot による循環参照防止 */
    if (!Util::compare(morphs, m_selectedMorphRefs)) {
        m_info->setMorphs(morphs, tr("(multiple)"));
        m_info->update();
        m_selectedMorphRefs = morphs;
        emit morphsDidSelect(morphs);
    }
}

void SceneWidget::rotateScene(const Vector3 &delta)
{
    ICamera *camera = m_loader->sceneRef()->camera();
    camera->setAngle(camera->angle() + delta);
    emit cameraPerspectiveDidSet(camera);
}

void SceneWidget::rotateModel(const Quaternion &delta)
{
    rotateModel(m_loader->selectedModelRef(), delta);
}

void SceneWidget::rotateModel(IModelSharedPtr model, const Quaternion &delta)
{
    if (model) {
        const Quaternion &rotation = model->worldRotation();
        model->setWorldRotation(rotation * delta);
        emit modelDidRotate(rotation);
    }
}

void SceneWidget::translateScene(const Vector3 &delta)
{
    ICamera *camera = m_loader->sceneRef()->camera();
    camera->setLookAt(camera->lookAt() + camera->modelViewTransform().getBasis().inverse() * delta);
    emit cameraPerspectiveDidSet(camera);
}

void SceneWidget::translateModel(const Vector3 &delta)
{
    translateModel(m_loader->selectedModelRef(), delta);
}

void SceneWidget::translateModel(IModelSharedPtr model, const Vector3 &delta)
{
    if (model) {
        const Vector3 &position = model->worldPosition();
        model->setWorldPosition(position + delta);
        emit modelDidMove(position);
    }
}

void SceneWidget::resetModelPosition()
{
    IModelSharedPtr model = m_loader->selectedModelRef();
    if (model) {
        const Vector3 &position = model->worldPosition();
        model->setWorldPosition(kZeroV3);
        emit modelDidMove(position);
    }
}

void SceneWidget::updatePlaneWorld(const ICamera *camera)
{
    Transform transform = camera->modelViewTransform().inverse();
    transform.setOrigin(kZeroV3);
    m_plane->updateTransform(transform);
}

void SceneWidget::renderBackgroundObjects()
{
    /* 背景画像描写 */
    m_background->draw();
    /* グリッドの描写 */
    m_grid->draw(m_loader.data(), m_loader->isGridVisible());
}

void SceneWidget::setGesturesEnable(bool value)
{
    if (value) {
        grabGesture(Qt::PanGesture);
        grabGesture(Qt::PinchGesture);
        grabGesture(Qt::SwipeGesture);
        setMoveGestureEnable(m_settingsRef->value("sceneWidget/enableMoveGesture", false).toBool());
        setRotateGestureEnable(m_settingsRef->value("sceneWidget/enableRotateGesture", true).toBool());
        setScaleGestureEnable(m_settingsRef->value("sceneWidget/enableScaleGesture", true).toBool());
        setUndoGestureEnable(m_settingsRef->value("sceneWidget/enableUndoGesture", true).toBool());
        qDebug("Gestures enabled");
    }
    else {
        ungrabGesture(Qt::PanGesture);
        ungrabGesture(Qt::PinchGesture);
        ungrabGesture(Qt::SwipeGesture);
        qDebug("Gestures disabled");
    }
    m_enableGestures = value;
}

void SceneWidget::loadFile(const QString &path)
{
    /* モデルファイル */
    QFileInfo fileInfo(path);
    const QString &extension = fileInfo.suffix().toLower();
    if (extension == "pmd" || extension == "pmx" || extension == "zip") {
        IModelSharedPtr modelPtr;
        loadModel(path, modelPtr);
        setEmptyMotion(modelPtr, false);
    }
    /* モーションファイル */
    else if (extension == "vmd" || extension == "mvd") {
        IMotionSharedPtr motionPtr;
        loadMotionToModel(path, m_loader->selectedModelRef(), motionPtr);
        if (motionPtr)
            refreshMotions();
    }
    /* アクセサリファイル */
    else if (extension == "x") {
        loadAsset(path);
    }
    else if (extension == "cgfx") {
        IEffect *effectRef = 0; /* unused */
        m_loader->loadEffectRef(path, effectRef);
    }
    /* ポーズファイル */
    else if (extension == "vpd") {
        IModelSharedPtr model = m_loader->selectedModelRef();
        VPDFilePtr ptr = insertPoseToSelectedModel(path, model);
        if (!ptr.isNull())
            m_loader->sceneRef()->updateModel(model.data());
    }
    /* アクセサリ情報ファイル */
    else if (extension == "vac") {
        loadAssetFromMetadata(path);
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
    ICamera *camera = m_loader->sceneRef()->camera();
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
    m_settingsRef->setValue("sceneWidget/showModelDialog", showModelDialog());
    m_settingsRef->setValue("sceneWidget/enableGestures", isGesturesEnabled());
    m_settingsRef->setValue("sceneWidget/enableMoveGesture", isMoveGestureEnabled());
    m_settingsRef->setValue("sceneWidget/enableRotateGesture", isRotateGestureEnabled());
    m_settingsRef->setValue("sceneWidget/enableScaleGesture", isScaleGestureEnabled());
    m_settingsRef->setValue("sceneWidget/enableUndoGesture", isUndoGestureEnabled());
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
    GLenum err = 0;
    if (!Scene::initialize(&err)) {
        qFatal("Cannot initialize GLEW: %d", err);
    }
    /* 一時的にログ出力を抑制し、そのあと MainWindow::bindSceneLoader までログ出力を抑制する */
    LoggerWidget::quietLogMessages(false);
    qDebug("VPVL2 version: %s (%d)", VPVL2_VERSION_STRING, VPVL2_VERSION);
    qDebug("GL_VERSION: %s", glGetString(GL_VERSION));
    qDebug("GL_VENDOR: %s", glGetString(GL_VENDOR));
    qDebug("GL_RENDERER: %s", glGetString(GL_RENDERER));
    LoggerWidget::quietLogMessages(true);
    m_config.insert(std::make_pair("dir.system.kernels", ":kernels"));
    m_config.insert(std::make_pair("dir.system.shaders", ":shaders"));
    m_config.insert(std::make_pair("dir.system.toon", ":textures"));
    m_config.insert(std::make_pair("dir.system.effects", ":effects"));
    /* Delegate/SceneLoader は OpenGL のコンテキストが必要なのでここで初期化する */
    m_renderContext.reset(new RenderContext(0, &m_config));
    m_loader.reset(new SceneLoader(m_encodingRef, m_factoryRef, m_renderContext.data()));
    connect(m_loader.data(), SIGNAL(projectDidLoad(bool)), SLOT(openErrorDialogIfLoadingProjectFailed(bool)));
    connect(m_loader.data(), SIGNAL(projectDidSave(bool)), SLOT(openErrorDialogIfSavingProjectFailed(bool)));
    connect(m_loader.data(), SIGNAL(preprocessDidPerform()), SLOT(renderBackgroundObjects()));
    connect(m_loader.data(), SIGNAL(modelDidSelect(IModelSharedPtr)), SLOT(setSelectedModel(IModelSharedPtr)));
#ifdef IS_VPVM
    const QSize &s = size();
    m_handles.reset(new Handles(m_loader.data(), m_renderContext.data(), s));
    connect(this, SIGNAL(modelDidMove(Vector3)), m_handles.data(), SLOT(updateHandleModel()));
    connect(this, SIGNAL(modelDidRotate(Quaternion)), m_handles.data(), SLOT(updateHandleModel()));
    /* テクスチャ情報を必要とするため、ハンドルのリソースの読み込みはここで行う */
    m_handles->loadImageHandles();
    m_info.reset(new InfoPanel(s, m_renderContext.data()));
    /* 動的なテクスチャ作成を行うため、情報パネルのリソースの読み込みも個々で行った上で初期設定を行う */
    m_info->load();
    m_debugDrawer.reset(new DebugDrawer(m_renderContext.data()));
    /* デバッグ表示のシェーダ読み込み(ハンドルと同じソースを使う) */
    m_debugDrawer->load();
    m_background.reset(new BackgroundImage(s, m_renderContext.data()));
    /* OpenGL を利用するため、格子状フィールドの初期化もここで行う */
    m_grid.reset(new Grid(m_renderContext.data()));
    m_grid->load();
    m_loader->updateDepthBuffer(QSize());
    m_info->setModel(0);
    m_info->setFPS(0.0f);
    m_info->update();
#endif
    m_refreshTimer.start();
    startAutomaticRendering();
    emit initailizeGLContextDidDone();
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QPointF &pos = event->localPos();
#else
    const QPointF &pos = event->posF();
#endif
    int flags;
    m_lockTouchEvent = true;
    m_clickOrigin = pos;
    m_handles->setPoint2D(pos);
    Vector3 znear, zfar, hit;
#ifdef IS_VPVM
    QRectF rect;
    makeRay(pos, znear, zfar);
    m_loader->setMousePosition(event, geometry());
    m_plane->test(znear, zfar, hit);
    /* 今は決め打ちの値にしている */
    const Scalar &delta = 0.0005 * m_loader->sceneRef()->camera()->distance();
    m_delta.setX(delta);
    m_delta.setY(delta);
    /*
     * モデルのハンドルと重なるケースを考慮して右下のハンドルを優先的に処理する。
     * また、右下のハンドル処理はひとつ以上ボーンが選択されていなければならない。
     */
    bool movable = false, rotateable = false;
    if (!m_selectedBoneRefs.isEmpty()) {
        IBone *bone = m_selectedBoneRefs.last();
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
    if (const IModel *model = m_loader->selectedModelRef().data()) {
        /* ボーン選択モードである */
        if (m_editMode == kSelect) {
            IBone *nearestBone = findNearestBone(model, znear, zfar, 0.1);
            /* 操作可能で最も近いボーンを選択状態にする */
            if (nearestBone) {
                QList<IBone *> selectedBones;
                /* CTRL が押されている場合は前回の選択状態を引き継ぐ */
                if (event->modifiers() & Qt::CTRL)
                    selectedBones.append(m_selectedBoneRefs);
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
                m_currentSelectedBoneRef = bone;
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QPointF &pos = event->localPos();
#else
    const QPointF &pos = event->posF();
#endif
    IBone *bone = m_handles->currentBone();
    m_isImageHandleRectIntersect = false;
    m_loader->setMousePosition(event, geometry());
    if (m_currentSelectedBoneRef) {
        Vector3 znear, zfar, hit;
        makeRay(pos, znear, zfar);
        m_plane->test(znear, zfar, hit);
        const Vector3 &delta = hit - m_lastBonePosition;
        emit handleDidMoveAbsolute(delta, m_currentSelectedBoneRef, 'G');
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
            ILight *light = m_loader->sceneRef()->light();
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    const QPointF &pos = event->localPos();
#else
    const QPointF &pos = event->posF();
#endif
    if (testHitModelHandle(pos))
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
    if (isModelHandle || isImageHandle || m_currentSelectedBoneRef)
        emit handleDidRelease();
    m_currentSelectedBoneRef = 0;
    m_lastBonePosition.setZero();
}

void SceneWidget::paintGL()
{
#ifdef IS_VPVM
    Scene *scene = m_loader->sceneRef();
    /* ボーン選択モード以外でのみ深度バッファのレンダリングを行う */
    ILight *light = scene->light();
    if (m_editMode != kSelect) {
        m_renderContext->renderShadowMap();
        light->setToonEnable(true);
    }
    else {
        m_loader->sceneRef()->setShadowMapRef(0);
        light->setToonEnable(false);
    }
    /* 通常のレンダリングを行うよう切り替えてレンダリングする */
    qglClearColor(m_loader->screenColor());
    m_loader->renderOffscreen();
    m_renderContext->updateCameraMatrices(glm::vec2(width(), height()));
    m_loader->renderWindow();
    /* ボーン選択済みかどうか？ボーンが選択されていればハンドル描写を行う */
    IBone *bone = 0;
    const IModel *model = m_loader->selectedModelRef().data();
    if (!m_selectedBoneRefs.isEmpty())
        bone = m_selectedBoneRefs.first();
    switch (m_editMode) {
    case kSelect: /* ボーン選択モード */
        /* モデルのボーンの接続部分をレンダリング */
        if (!(m_handleFlags & Handles::kEnable)) {
            QSet<const IBone *> boneSet;
            foreach (const IBone *bone, m_selectedBoneRefs)
                boneSet.insert(bone);
            m_debugDrawer->drawModelBones(model, m_loader.data(), boneSet);
        }
        /* 右下のハンドルが領域に入ってる場合は軸を表示する */
        if (m_isImageHandleRectIntersect)
            m_debugDrawer->drawBoneTransform(bone, model, m_loader.data(), m_handles->modeFromConstraint());
        /*
         * 情報パネルと右下のハンドルを最後にレンダリング(表示上最上位に持っていく)
         * 右下の操作ハンドルはモデルが選択されていない場合は非表示にするように変更
         */
        if (m_loader->selectedModelRef())
            m_handles->drawImageHandles(bone);
        m_info->draw();
        break;
    case kRotate: /* 回転モード */
        /* kRotate と kMove の場合はレンダリングがうまくいかない関係でモデルのハンドルを最後に持ってく */
        m_debugDrawer->drawMovableBone(bone, model, m_loader.data());
        m_handles->drawImageHandles(bone);
        m_info->draw();
        m_handles->drawRotationHandle();
        break;
    case kMove: /* 移動モード */
        m_debugDrawer->drawMovableBone(bone, model, m_loader.data());
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
    if (event->timerId() == m_updateTimer.timerId()) {
        if (m_playing) {
            /* タイマーの仕様上一定ではないため、差分をここで吸収する */
            Scalar elapsed = m_refreshTimer.elapsed() / Scene::defaultFPS();
            Scalar diff = elapsed - m_prevElapsed;
            m_prevElapsed = elapsed;
            if (diff < 0)
                diff = elapsed;
            advanceMotion(diff);
            updateFPS();
        }
        else {
            /* 非再生中(編集中)はモーションを一切動かさず、カメラの状態更新だけ行う */
            Scene *scene = m_loader->sceneRef();
            scene->update(Scene::kUpdateCamera);
        }
        updateScene();
    }
    QGLWidget::timerEvent(event);
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
    if (!m_selectedBoneRefs.isEmpty()) {
        IBone *bone = m_selectedBoneRefs.last();
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
    else if (IModelSharedPtr model = m_loader->selectedModelRef())
        translateModel(model, newDelta);
    else
        translateScene(newDelta);
}

void SceneWidget::pinchTriggered(QPinchGesture *event)
{
    const Qt::GestureState state = event->state();
    QPinchGesture::ChangeFlags flags = event->changeFlags();
    ICamera *camera = m_loader->sceneRef()->camera();
    /* 回転ジェスチャー */
    if (m_enableRotateGesture && flags & QPinchGesture::RotationAngleChanged) {
        qreal value = event->rotationAngle() - event->lastRotationAngle();
        const Scalar &radian = btRadians(value);
        /* ボーンが選択されている場合はボーンの回転 (現時点でY軸のみ) */
        if (!m_selectedBoneRefs.isEmpty()) {
            IBone *bone = m_selectedBoneRefs.last();
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
        else if (IModelSharedPtr model = m_loader->selectedModelRef()) {
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

void SceneWidget::openErrorDialogIfLoadingProjectFailed(bool ok)
{
    if (!ok) {
        Util::warning(this,
                      tr("Failed loading the project"),
                      tr("Failed loading the project. The project contains duplicated UUID or corrupted."));
    }
}

void SceneWidget::openErrorDialogIfSavingProjectFailed(bool ok)
{
    if (!ok) {
        Util::warning(this,
                      tr("Failed saving the project"),
                      tr("Failed saving the project. Retry saving the project again after seconds."));
    }
}

bool SceneWidget::acceptAddingModel(const IModel *model)
{
    /* モデルを追加する前にモデルの名前とコメントを出すダイアログを表示 */
    QMessageBox mbox;
    QString comment = Util::toQStringFromString(model->comment());
    mbox.setText(tr("Model Information of \"%1\"").arg(Util::toQStringFromModel(model)));
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
    if (m_refreshTimer.elapsed() > 1000) {
        m_currentFPS = m_frameCount;
        m_frameCount = 0;
        m_refreshTimer.restart();
        m_info->setFPS(m_currentFPS);
        m_info->update();
        emit fpsDidUpdate(m_currentFPS);
    }
    m_frameCount++;
}

void SceneWidget::updateScene()
{
    if (m_enableUpdateGL)
        updateGL();
    emit cameraPerspectiveDidSet(m_loader->sceneRef()->camera());
}

void SceneWidget::clearSelectedBones()
{
    m_selectedBoneRefs.clear();
    m_info->setBones(m_selectedBoneRefs, "");
    m_handles->setBone(0);
}

void SceneWidget::clearSelectedMorphs()
{
    m_selectedMorphRefs.clear();
    m_info->setMorphs(m_selectedMorphRefs, "");
}

void SceneWidget::loadModelMotion(IMotionSharedPtr motion, const QString &path, IModelSharedPtr model)
{
    const QFileInfo fi(path);
    UIAlertMVDMotion(motion, this);
    emit fileDidOpenProgress(tr("Loading %1").arg(fi.baseName()), false);
    emit fileDidUpdateProgress(0, 0, tr("Loading %1...").arg(fi.baseName()));
    m_loader->setModelMotion(motion, model);
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
    model->getBoneRefs(bones);
    const int nbones = bones.count();
    IBone *nearestBone = 0;
    Scalar hitLambda = 1.0f;
    Vector3 normal;
    /* 操作可能なボーンを探す */
    for (int i = 0; i < nbones; i++) {
        IBone *bone = bones[i];
        const Vector3 &o = model->worldPosition() + bone->worldTransform().getOrigin(),
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

} /*  namespace vpvm */

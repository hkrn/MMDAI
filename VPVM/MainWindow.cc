/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include "MainWindow.h"
#include "BoneUIDelegate.h"
#include "ScenePlayer.h"

#include "common/Handles.h"
#include "common/LoggerWidget.h"
#include "common/SceneLoader.h"
#include "common/SceneWidget.h"
#include "common/VPDFile.h"
#include "common/util.h"
#include "dialogs/BackgroundImageSettingDialog.h"
#include "dialogs/BoneDialog.h"
#include "dialogs/ExportVideoDialog.h"
#include "dialogs/GravitySettingDialog.h"
#include "dialogs/FrameSelectionDialog.h"
#include "dialogs/InterpolationDialog.h"
#include "dialogs/PlaySettingDialog.h"
#include "dialogs/RenderOrderDialog.h"
#include "dialogs/ShadowMapSettingDialog.h"
#include "models/BoneMotionModel.h"
#include "models/MorphMotionModel.h"
#include "models/SceneMotionModel.h"
#include "video/AudioDecoder.h"
#include "video/VideoEncoder.h"
#include "widgets/AssetWidget.h"
#include "widgets/CameraPerspectiveWidget.h"
#include "widgets/MorphWidget.h"
#include "widgets/LicenseWidget.h"
#include "widgets/ModelInfoWidget.h"
#include "widgets/ModelSettingWidget.h"
#include "widgets/ModelTabWidget.h"
#include "widgets/SceneLightWidget.h"
#include "widgets/TabWidget.h"
#include "widgets/TimelineTabWidget.h"

#include "video/IAudioDecoder.h"
#include "video/IVideoEncoder.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>
#include <vpvl2/qt/CString.h>
#include <vpvl2/qt/Encoding.h>

using namespace vpvm;
using namespace vpvl2;
using namespace vpvl2::qt;

namespace {

static int UIFindIndexOfActions(IModel *model, const QList<QAction *> &actions)
{
    const QString &name = toQStringFromModel(model);
    int i = 0, found = -1;
    foreach (QAction *action, actions) {
        if (action->text() == name) {
            found = i;
            break;
        }
        i++;
    }
    return found;
}


static inline void UICreatePlaySettingDialog(MainWindow *mainWindow,
                                             QSettings *settings,
                                             const QScopedPointer<SceneWidget> &sceneWidget,
                                             QScopedPointer<PlaySettingDialog> &dialog)
{
    if (!dialog) {
        dialog.reset(new PlaySettingDialog(sceneWidget->sceneLoaderRef(), settings, mainWindow));
        QObject::connect(dialog.data(), SIGNAL(playingDidStart()), mainWindow, SLOT(invokePlayer()));
    }
}

static inline void UICreateScenePlayer(MainWindow *mainWindow,
                                       const QScopedPointer<SceneWidget> &sceneWidget,
                                       const QScopedPointer<PlaySettingDialog> &dialog,
                                       const QScopedPointer<TimelineTabWidget> &timeline,
                                       QScopedPointer<ScenePlayer> &player)
{
    if (!player) {
        player.reset(new ScenePlayer(sceneWidget.data(), dialog.data()));
        QObject::connect(dialog.data(), SIGNAL(playingDidStart()), dialog.data(), SLOT(hide()));
        QObject::connect(dialog.data(), SIGNAL(playingDidStart()), player.data(), SLOT(setRestoreState()));
        QObject::connect(player.data(), SIGNAL(motionDidSeek(int)), timeline.data(), SLOT(setCurrentFrameIndex(int)));
        QObject::connect(player.data(), SIGNAL(renderFrameDidStop()), mainWindow, SLOT(enableSelectingBonesAndMorphs()));
        QObject::connect(player.data(), SIGNAL(renderFrameDidStopAndRestoreState()), dialog.data(), SLOT(show()));
    }
}

static QGLFormat UIGetQGLFormat()
{
    QGLFormat format;
    format.setSampleBuffers(true);
#ifdef Q_OS_DARWIN
    format.setSamples(4);
    format.setRedBufferSize(16);
    format.setBlueBufferSize(16);
    format.setGreenBufferSize(16);
    format.setDepthBufferSize(32);
#endif
    return format;
}

}

struct MainWindow::WindowState {
    WindowState()
        : timeIndex(0),
          preferredFPS(0),
          isGridVisible(false),
          isImage(false)
    {
    }
    QRect mainGeometry;
    QSize minSize;
    QSize maxSize;
    QSize scenesize;
    QSizePolicy policy;
    IKeyframe::TimeIndex timeIndex;
    Scalar preferredFPS;
    bool isGridVisible;
    bool isImage;
};

namespace vpvm
{

MainWindow::MainWindow(const Encoding::Dictionary &dictionary, QWidget *parent)
    : QMainWindow(parent),
      m_encoding(new Encoding(dictionary)),
      m_factory(new Factory(m_encoding.data())),
      m_settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qAppName()),
      m_undo(new QUndoGroup()),
      m_sceneWidget(new SceneWidget(UIGetQGLFormat(), m_encoding.data(), m_factory.data(), &m_settings)),
      m_sceneTabWidget(new TabWidget(&m_settings)),
      m_boneMotionModel(new BoneMotionModel(m_factory.data(), m_undo.data())),
      m_morphMotionModel(new MorphMotionModel(m_factory.data(), m_undo.data())),
      m_sceneMotionModel(new SceneMotionModel(m_factory.data(), m_undo.data(), m_sceneWidget.data())),
      m_modelTabWidget(new ModelTabWidget(&m_settings, m_morphMotionModel.data())),
      m_timelineTabWidget(new TimelineTabWidget(&m_settings, m_boneMotionModel.data(), m_morphMotionModel.data(), m_sceneMotionModel.data())),
      m_boneUIDelegate(new BoneUIDelegate(m_boneMotionModel.data(), &m_settings, this)),
      m_timelineDockWidget(new QDockWidget(this)),
      m_sceneDockWidget(new QDockWidget(this)),
      m_modelDockWidget(new QDockWidget(this)),
      m_mainToolBar(new QToolBar()),
      m_actionClearRecentFiles(new QAction(0)),
      m_actionNewProject(new QAction(0)),
      m_actionNewMotion(new QAction(0)),
      m_actionLoadProject(new QAction(0)),
      m_actionAddModel(new QAction(0)),
      m_actionAddAsset(new QAction(0)),
      m_actionInsertToAllModels(new QAction(0)),
      m_actionInsertToSelectedModel(new QAction(0)),
      m_actionSetCamera(new QAction(0)),
      m_actionSaveProject(new QAction(0)),
      m_actionSaveProjectAs(new QAction(0)),
      m_actionSaveMotion(new QAction(0)),
      m_actionSaveMotionAs(new QAction(0)),
      m_actionSaveCameraMotionAs(new QAction(0)),
      m_actionLoadModelPose(new QAction(0)),
      m_actionSaveModelPose(new QAction(0)),
      m_actionLoadAssetMetadata(new QAction(0)),
      m_actionSaveAssetMetadata(new QAction(0)),
      m_actionExportImage(new QAction(0)),
      m_actionExportVideo(new QAction(0)),
      m_actionExit(new QAction(0)),
      m_actionAbout(new QAction(0)),
      m_actionAboutQt(new QAction(0)),
      m_actionPlay(new QAction(0)),
      m_actionPlaySettings(new QAction(0)),
      m_actionOpenGravitySettingsDialog(new QAction(0)),
      m_actionOpenRenderOrderDialog(new QAction(0)),
      m_actionOpenScreenColorDialog(new QAction(0)),
      m_actionOpenShadowMapDialog(new QAction(0)),
      m_actionEnablePhysics(new QAction(0)),
      m_actionShowGrid(new QAction(0)),
      m_actionSetBackgroundImage(new QAction(0)),
      m_actionClearBackgroundImage(new QAction(0)),
      m_actionOpenBackgroundImageDialog(new QAction(0)),
      m_actionZoomIn(new QAction(0)),
      m_actionZoomOut(new QAction(0)),
      m_actionRotateUp(new QAction(0)),
      m_actionRotateDown(new QAction(0)),
      m_actionRotateLeft(new QAction(0)),
      m_actionRotateRight(new QAction(0)),
      m_actionTranslateUp(new QAction(0)),
      m_actionTranslateDown(new QAction(0)),
      m_actionTranslateLeft(new QAction(0)),
      m_actionTranslateRight(new QAction(0)),
      m_actionResetCamera(new QAction(0)),
      m_actionSelectNextModel(new QAction(0)),
      m_actionSelectPreviousModel(new QAction(0)),
      m_actionRevertSelectedModel(new QAction(0)),
      m_actionDeleteSelectedModel(new QAction(0)),
      m_actionTranslateModelUp(new QAction(0)),
      m_actionTranslateModelDown(new QAction(0)),
      m_actionTranslateModelLeft(new QAction(0)),
      m_actionTranslateModelRight(new QAction(0)),
      m_actionResetModelPosition(new QAction(0)),
      m_actionBoneXPosZero(new QAction(0)),
      m_actionBoneYPosZero(new QAction(0)),
      m_actionBoneZPosZero(new QAction(0)),
      m_actionBoneRotationZero(new QAction(0)),
      m_actionBoneResetAll(new QAction(0)),
      m_actionBoneDialog(new QAction(0)),
      m_actionRegisterKeyframe(new QAction(0)),
      m_actionSelectAllKeyframes(new QAction(0)),
      m_actionSelectKeyframeDialog(new QAction(0)),
      m_actionKeyframeWeightDialog(new QAction(0)),
      m_actionInterpolationDialog(new QAction(0)),
      m_actionInsertEmptyFrame(new QAction(0)),
      m_actionDeleteSelectedKeyframe(new QAction(0)),
      m_actionNextFrame(new QAction(0)),
      m_actionPreviousFrame(new QAction(0)),
      m_actionCut(new QAction(0)),
      m_actionCopy(new QAction(0)),
      m_actionPaste(new QAction(0)),
      m_actionReversedPaste(new QAction(0)),
      m_actionOpenUndoView(new QAction(0)),
      m_actionViewLogMessage(new QAction(0)),
      m_actionEnableGestures(new QAction(0)),
      m_actionEnableMoveGesture(new QAction(0)),
      m_actionEnableRotateGesture(new QAction(0)),
      m_actionEnableScaleGesture(new QAction(0)),
      m_actionEnableUndoGesture(new QAction(0)),
      m_actionShowTimelineDock(new QAction(0)),
      m_actionShowSceneDock(new QAction(0)),
      m_actionShowModelDock(new QAction(0)),
      m_actionShowModelDialog(new QAction(0)),
      m_actionSetSoftwareSkinningFallback(new QAction(0)),
      m_actionSetOpenCLSkinningType1(new QAction(0)),
      m_actionSetOpenCLSkinningType2(new QAction(0)),
      m_actionSetVertexShaderSkinningType1(new QAction(0)),
      m_actionEnableEffect(new QAction(0)),
      m_menuBar(new QMenuBar()),
      m_menuFile(new QMenu()),
      m_menuEdit(new QMenu()),
      m_menuProject(new QMenu()),
      m_menuScene(new QMenu()),
      m_menuModel(new QMenu()),
      m_menuKeyframe(new QMenu()),
      m_menuEffect(new QMenu()),
      m_menuView(new QMenu()),
      m_menuRetainModels(new QMenu()),
      m_menuRetainAssets(new QMenu()),
      m_menuRecentFiles(new QMenu()),
      m_menuHelp(new QMenu()),
      m_menuAcceleration(new QMenu()),
      m_loggerWidgetRef(0),
      m_modelRef(0),
      m_boneRef(0),
      m_position(0.0f, 0.0f, 0.0f),
      m_angle(0.0f, 0.0f, 0.0f),
      m_fovy(0.0f),
      m_distance(0.0f),
      m_currentFPS(-1)
{
    m_actionRecentFiles.reserve(kMaxRecentFiles);
    m_loggerWidgetRef = LoggerWidget::sharedInstance(&m_settings);
    createActionsAndMenus();
    bindWidgets();
    restoreGeometry(m_settings.value("mainWindow/geometry").toByteArray());
    restoreState(m_settings.value("mainWindow/state").toByteArray());
    updateWindowTitle();
    statusBar()->show();
    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
    /* null アクセスが発生してしまうため、先に以下のシグナルを解除しておく */
    SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
    disconnect(loader, SIGNAL(modelWillDelete(IModel*,QUuid)), this, SLOT(deleteModel(IModel*,QUuid)));
    /* 所有権が移動しているため、事前に take で所有権を放棄してメモリ解放しないようにする */
    m_modelTabWidget.take();
    m_sceneTabWidget.take();
    m_timelineTabWidget.take();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSaveProject()) {
        m_settings.setValue("mainWindow/geometry", saveGeometry());
        m_settings.setValue("mainWindow/state", saveState());
        m_settings.setValue("mainWindow/visibleTabs", m_sceneTabWidget->isVisible());
        m_settings.setValue("mainWindow/visibleTimeline", m_timelineTabWidget->isVisible());
        m_settings.setValue("mainWindow/timelineDockWidgetGeometry", m_timelineDockWidget->saveGeometry());
        m_settings.setValue("mainWindow/sceneDockWidgetGeometry", m_sceneDockWidget->saveGeometry());
        m_settings.setValue("mainWindow/modelDockWidgetGeometry", m_modelDockWidget->saveGeometry());
        qApp->sendEvent(m_sceneWidget.data(), event);
        event->accept();
    }
    else {
        event->ignore();
    }
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    const QPoint &pos = event->globalPos();
    if (m_sceneWidget->rect().contains(m_sceneWidget->mapFromGlobal(pos))) {
        QMenu menu(this);
        menu.addAction(m_actionAddModel.data());
        menu.addAction(m_actionAddAsset.data());
        menu.addAction(m_actionInsertToSelectedModel.data());
        menu.addSeparator();
        menu.addMenu(m_menuRetainModels.data());
        menu.addSeparator();
        menu.addAction(m_actionShowTimelineDock.data());
        menu.addAction(m_actionShowSceneDock.data());
        menu.addAction(m_actionShowModelDock.data());
        menu.exec(pos);
    }
    else if (m_timelineTabWidget->rect().contains(m_timelineTabWidget->mapFromGlobal(pos))) {
        QMenu menu(this);
        menu.addAction(m_actionRegisterKeyframe.data());
        menu.addAction(m_actionSelectAllKeyframes.data());
        menu.addSeparator();
        menu.addAction(m_actionCut.data());
        menu.addAction(m_actionCopy.data());
        menu.addAction(m_actionPaste.data());
        menu.addAction(m_actionReversedPaste.data());
        menu.addSeparator();
        menu.addAction(m_actionUndo.data());
        menu.addAction(m_actionRedo.data());
        menu.addSeparator();
        menu.addAction(m_actionDeleteSelectedKeyframe.data());
        menu.addSeparator();
        menu.addAction(m_actionSelectKeyframeDialog.data());
        menu.addAction(m_actionKeyframeWeightDialog.data());
        menu.addAction(m_actionInterpolationDialog.data());
        menu.exec(pos);
    }
}

void MainWindow::hideEvent(QHideEvent * /* event */)
{
    if (!m_videoEncoder || !m_videoEncoder->isRunning())
        m_sceneWidget->stopAutomaticRendering();
}

void MainWindow::showEvent(QShowEvent * /* event */)
{
    if (!m_videoEncoder || !m_videoEncoder->isRunning())
        m_sceneWidget->startAutomaticRendering();
}

void MainWindow::selectModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        const QUuid uuid(action->data().toString());
        IModel *model = m_sceneWidget->sceneLoaderRef()->findModel(uuid);
        m_sceneWidget->setSelectedModel(model, SceneWidget::kSelect);
    }
}

void MainWindow::setCurrentModel(IModel *model)
{
    m_modelRef = model;
}

void MainWindow::newMotionFile()
{
    if (maybeSaveMotion()) {
        /*
         * PMDMotionModel のデータを空にしてから新規のモーションを作成する
         * なお、PMDMotionModel のデータは VMDMotion とは独立している
         */
        m_currentMotionFilename = "";
        m_boneMotionModel->removeMotion();
        m_morphMotionModel->removeMotion();
        m_sceneWidget->setEmptyMotion();
        updateWindowTitle();
    }
}

void MainWindow::newProjectFile()
{
    if (maybeSaveProject()) {
        SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
        bool isEffectEnable = loader->isEffectEnabled();
        /*
         * カメラを含むモーションとモデルを全て削除してからプロジェクトを新規に作成する
         * SceneWidget#clear は内部的に削除と同時に新しい空のプロジェクトが作成される
         */
        m_currentProjectFilename = "";
        m_currentMotionFilename = "";
        m_boneMotionModel->removeMotion();
        m_morphMotionModel->removeMotion();
        m_sceneMotionModel->removeMotion();
        m_sceneWidget->clear();
        /*
         * 物理とグリッド表示のチェックをプロジェクト新規作成直後の状態に戻す
         * エフェクトだけ例外で新規作成直前の状態に戻す
         */
        m_actionEnablePhysics->setChecked(loader->isPhysicsEnabled());
        m_actionShowGrid->setChecked(loader->isGridVisible());
        loader->setEffectEnable(isEffectEnable);
        updateWindowTitle();
    }
}

void MainWindow::loadProject()
{
    if (maybeSaveProject()) {
        const QString &filename = openFileDialog("mainWindow/lastProjectDirectory",
                                                 tr("Open VPVM file"),
                                                 tr("VPVM file (*.xml)"),
                                                 &m_settings);
        if (!filename.isEmpty()) {
            m_boneMotionModel->removeMotion();
            m_morphMotionModel->removeMotion();
            m_sceneMotionModel->removeMotion();
            m_sceneWidget->loadProject(filename);
            SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
            m_actionEnablePhysics->setChecked(loader->isPhysicsEnabled());
            m_actionShowGrid->setChecked(loader->isGridVisible());
            m_actionSetOpenCLSkinningType1->setChecked(loader->isOpenCLSkinningType1Enabled());
            m_actionSetOpenCLSkinningType2->setChecked(loader->isOpenCLSkinningType2Enabled());
            m_actionSetVertexShaderSkinningType1->setChecked(loader->isVertexShaderSkinningType1Enabled());
            m_currentProjectFilename = filename;
            updateWindowTitle();
        }
    }
}

void MainWindow::saveMotion()
{
    if (m_currentMotionFilename.isEmpty())
        saveMotionAs(m_currentMotionFilename);
    else
        saveMotionFile(m_currentMotionFilename);
    updateWindowTitle();
}

void MainWindow::saveProject()
{
    if (m_currentProjectFilename.isEmpty())
        saveProjectAs(m_currentProjectFilename);
    else
        saveProjectFile(m_currentProjectFilename);
    updateWindowTitle();
}

void MainWindow::revertSelectedModel()
{
    m_sceneWidget->revertSelectedModel();
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        m_sceneWidget->loadFile(action->data().toString());
}

void MainWindow::addRecentFile(const QString &filename)
{
    QStringList files = m_settings.value("mainWindow/recentFiles").toStringList();
    files.removeAll(filename);
    files.prepend(filename);
    while (files.size() > kMaxRecentFiles)
        files.removeLast();
    m_settings.setValue("mainWindow/recentFiles", files);
    updateRecentFiles();
}

void MainWindow::updateRecentFiles()
{
    QStringList files = m_settings.value("mainWindow/recentFiles").toStringList();
    int maxFiles = kMaxRecentFiles, nRecentFiles = qMin(files.size(), maxFiles);
    QFileInfo fileInfo;
    for (int i = 0; i < nRecentFiles; i++) {
        QAction *action = m_actionRecentFiles[i];
        fileInfo.setFile(files[i]);
        action->setText(tr("&%1 %2").arg(i + 1).arg(fileInfo.fileName()));
        action->setData(files[i]);
        action->setVisible(true);
    }
    for (int i = nRecentFiles; i < kMaxRecentFiles; i++)
        m_actionRecentFiles[i]->setVisible(false);
}

void MainWindow::clearRecentFiles()
{
    m_settings.setValue("mainWindow/recentFiles", QStringList());
    updateRecentFiles();
}

void MainWindow::addModel(IModel *model, const QUuid &uuid)
{
    /* 追加されたモデルをモデル選択のメニューに追加する */
    QString name = toQStringFromModel(model);
    QAction *action = new QAction(name, this);
    action->setData(uuid.toString());
    action->setStatusTip(tr("Select a model %1").arg(name));
    connect(action, SIGNAL(triggered()), SLOT(selectModel()));
    m_menuRetainModels->addAction(action);
    m_sceneWidget->setSelectedModel(model, SceneWidget::kSelect);
}

void MainWindow::deleteModel(IModel *model, const QUuid &uuid)
{
    /* 削除されるモデルが選択中のモデルと同じなら選択状態を解除しておく(残すと不正アクセスの原因になるので) */
    if (model == m_sceneWidget->sceneLoaderRef()->selectedModelRef())
        m_sceneWidget->revertSelectedModel();
    /* 削除されるモデルをモデル選択のメニューから削除する */
    QAction *actionToRemove = 0;
    const QString &uuidString = uuid.toString();
    foreach (QAction *action, m_menuRetainModels->actions()) {
        if (action->data() == uuidString) {
            actionToRemove = action;
            break;
        }
    }
    if (actionToRemove)
        m_menuRetainModels->removeAction(actionToRemove);
}

void MainWindow::addAsset(IModel *asset, const QUuid &uuid)
{
    /* 追加されたアクセサリをアクセサリ選択のメニューに追加する */
    QString name = toQStringFromModel(asset);
    QAction *action = new QAction(name, this);
    action->setData(uuid.toString());
    action->setStatusTip(tr("Select an asset %1").arg(name));
    //connect(action, SIGNAL(triggered()), SLOT(selectCurrentModel()));
    m_menuRetainAssets->addAction(action);
}

void MainWindow::deleteAsset(IModel * /* asset */, const QUuid &uuid)
{
    /* 削除されたアクセサリをアクセサリ選択のメニューから削除する */
    QAction *actionToRemove = 0;
    const QString &uuidString = uuid.toString();
    foreach (QAction *action, m_menuRetainAssets->actions()) {
        if (action->data() == uuidString) {
            actionToRemove = action;
            break;
        }
    }
    if (actionToRemove)
        m_menuRetainAssets->removeAction(actionToRemove);
}

void MainWindow::saveMotionAs()
{
    saveMotionAs(m_currentMotionFilename);
    updateWindowTitle();
}

bool MainWindow::saveMotionAs(QString &filename)
{
    filename = openSaveDialog("mainWindow/lastModelMotionDirectory",
                              tr("Save model motion as a VMD/MVD file"),
                              tr("Model motion file (*.vmd *.mvd)"),
                              tr("untitiled_model_motion.vmd"),
                              &m_settings);
    return !filename.isEmpty() ? saveMotionFile(filename) : false;
}

void MainWindow::saveCameraMotionAs()
{
    const QString &filename = openSaveDialog("mainWindow/lastCameraMotionDirectory",
                                             tr("Save camera motion as a VMD/MVD file"),
                                             tr("Camera motion file (*.vmd *.mvd)"),
                                             tr("untitiled_camera_motion.vmd"),
                                             &m_settings);
    QScopedPointer<IMotion> motion(m_factory->createMotion(IMotion::kVMD, 0));
    IMotion *motionPtr = motion.data();
    m_sceneMotionModel->saveMotion(motionPtr);
    saveMotionFile(filename, motionPtr);
}

bool MainWindow::saveMotionFile(const QString &filename)
{
    /* 全てのボーンフレーム、頂点モーフフレーム、カメラフレームをファイルとして書き出しを行う */
    QScopedPointer<IMotion> motion(m_factory->createMotion(IMotion::kVMD, 0));
    IMotion *motionPtr = motion.data();
    motionPtr->setParentModel(m_sceneWidget->sceneLoaderRef()->selectedModelRef());
    m_boneMotionModel->saveMotion(motionPtr);
    m_morphMotionModel->saveMotion(motionPtr);
    return saveMotionFile(filename, motionPtr);
}

bool MainWindow::saveMotionFile(const QString &filename, IMotion *motion)
{
    IMotion::Type type = filename.endsWith(".mvd") ? IMotion::kMVD : IMotion::kVMD;
    QScopedPointer<IMotion> newMotion(m_factory->convertMotion(motion, type));
    size_t size = newMotion->estimateSize();
    QScopedArrayPointer<uint8_t> buffer(new uint8_t[size]);
    newMotion->save(buffer.data());
    QFile file(filename);
    bool ret = true;
    if (file.open(QFile::WriteOnly)) {
        file.write(reinterpret_cast<const char *>(buffer.data()), size);
        file.close();
        qDebug("Saved a motion: %s", qPrintable(filename));
    }
    else {
        qWarning("Failed exporting VMD to file: %s", qPrintable(file.errorString()));
        ret = false;
    }
    return ret;
}

void MainWindow::saveProjectAs()
{
    saveProjectAs(m_currentProjectFilename);
    updateWindowTitle();
}

bool MainWindow::saveProjectAs(QString &filename)
{
    filename = openSaveDialog("mainWindow/lastProjectDirectory",
                              tr("Save projct as a VPVM project file"),
                              tr("VPVM project file (*.xml)"),
                              tr("untitled.xml"),
                              &m_settings);
    return !filename.isEmpty() ? saveProjectFile(filename) : false;
}

bool MainWindow::saveProjectFile(const QString &filename)
{
    SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
    loader->setOpenCLSkinningEnableType1(m_actionSetOpenCLSkinningType1->isChecked());
    loader->setOpenCLSkinningEnableType2(m_actionSetOpenCLSkinningType2->isChecked());
    loader->setVertexShaderSkinningType1Enable(m_actionSetVertexShaderSkinningType1->isChecked());
    loader->setPhysicsEnabled(m_actionEnablePhysics->isChecked());
    loader->setGridVisible(m_actionShowGrid->isChecked());
    m_sceneWidget->saveProject(filename);
    return true;
}

bool MainWindow::maybeSaveMotion()
{
    bool cancel, cond = m_boneMotionModel->isModified()
            || m_morphMotionModel->isModified()
            || m_sceneMotionModel->isModified();
    if (confirmSave(cond, cancel))
        saveMotion();
    return !cancel;
}

bool MainWindow::maybeSaveProject()
{
    bool cancel, cond = m_boneMotionModel->isModified()
            || m_morphMotionModel->isModified()
            || m_sceneMotionModel->isModified()
            || m_sceneWidget->sceneLoaderRef()->isProjectModified();
    if (confirmSave(cond, cancel))
        saveProject();
    return !cancel;
}

bool MainWindow::confirmSave(bool condition, bool &cancel)
{
    cancel = false;
    if (condition) {
        int ret = warning(0,
                          tr("Confirm"),
                          tr("Do you want to save your changes?"),
                          "",
                          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch (ret) {
        case QMessageBox::Save:
            return true;
        case QMessageBox::Cancel:
            cancel = true;
            break;
        default:
            break;
        }
    }
    return false;
}

void MainWindow::createActionsAndMenus()
{
    m_timelineDockWidget->setWidget(m_timelineTabWidget.data());
    m_timelineDockWidget->restoreGeometry(m_settings.value("mainWindow/timelineDockWidgetGeometry").toByteArray());
    addDockWidget(Qt::LeftDockWidgetArea, m_timelineDockWidget.data());
    m_sceneDockWidget->setWidget(m_sceneTabWidget.data());
    m_sceneDockWidget->restoreGeometry(m_settings.value("mainWindow/sceneDockWidgetGeometry").toByteArray());
    addDockWidget(Qt::LeftDockWidgetArea, m_sceneDockWidget.data());
    m_modelDockWidget->setWidget(m_modelTabWidget.data());
    m_modelDockWidget->restoreGeometry(m_settings.value("mainWindow/modelDockWidgetGeometry").toByteArray());
    addDockWidget(Qt::LeftDockWidgetArea, m_modelDockWidget.data());
    tabifyDockWidget(m_timelineDockWidget.data(), m_sceneDockWidget.data());
    tabifyDockWidget(m_sceneDockWidget.data(), m_modelDockWidget.data());

    connect(m_actionNewProject.data(), SIGNAL(triggered()), SLOT(newProjectFile()));
    connect(m_actionNewMotion.data(), SIGNAL(triggered()), SLOT(addNewMotion()));
    connect(m_actionLoadProject.data(), SIGNAL(triggered()), SLOT(loadProject()));
    connect(m_actionAddModel.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(addModel()));
    connect(m_actionAddAsset.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(addAsset()));
    connect(m_actionInsertToAllModels.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(insertMotionToAllModels()));
    connect(m_actionInsertToSelectedModel.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(insertMotionToSelectedModel()));
    connect(m_actionSetCamera.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(setCamera()));
    connect(m_actionLoadModelPose.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(insertPoseToSelectedModel()));
    connect(m_actionSaveModelPose.data(), SIGNAL(triggered()), SLOT(saveModelPose()));
    connect(m_actionLoadAssetMetadata.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(addAssetFromMetadata()));
    connect(m_actionSaveAssetMetadata.data(), SIGNAL(triggered()), SLOT(saveAssetMetadata()));
    connect(m_actionSaveProject.data(), SIGNAL(triggered()), SLOT(saveProject()));
    connect(m_actionSaveProjectAs.data(), SIGNAL(triggered()), SLOT(saveProjectAs()));
    connect(m_actionSaveMotion.data(), SIGNAL(triggered()), SLOT(saveMotion()));
    connect(m_actionSaveMotionAs.data(), SIGNAL(triggered()), SLOT(saveMotionAs()));
    connect(m_actionSaveCameraMotionAs.data(), SIGNAL(triggered()), SLOT(saveCameraMotionAs()));
    connect(m_actionExportImage.data(), SIGNAL(triggered()), SLOT(exportImage()));
    connect(m_actionExportVideo.data(), SIGNAL(triggered()), SLOT(exportVideo()));
    m_actionExit->setMenuRole(QAction::QuitRole);
    connect(m_actionExit.data(), SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    connect(m_actionRegisterKeyframe.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(addKeyframesFromSelectedIndices()));
    connect(m_actionInsertEmptyFrame.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(insertKeyframesBySelectedIndices()));
    connect(m_actionDeleteSelectedKeyframe.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(deleteKeyframesBySelectedIndices()));
    connect(m_actionCut.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(cutKeyframes()));
    connect(m_actionCopy.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(copyKeyframes()));
    connect(m_actionPaste.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(pasteKeyframes()));
    connect(m_actionReversedPaste.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(pasteKeyframesWithReverse()));
    m_actionUndo.reset(m_undo->createUndoAction(this));
    m_actionRedo.reset(m_undo->createRedoAction(this));
    connect(m_actionOpenUndoView.data(), SIGNAL(triggered()), SLOT(openUndoView()));
    connect(m_actionBoneXPosZero.data(), SIGNAL(triggered()), m_boneUIDelegate.data(), SLOT(resetBoneX()));
    connect(m_actionBoneYPosZero.data(), SIGNAL(triggered()), m_boneUIDelegate.data(), SLOT(resetBoneY()));
    connect(m_actionBoneZPosZero.data(), SIGNAL(triggered()), m_boneUIDelegate.data(), SLOT(resetBoneZ()));
    connect(m_actionBoneRotationZero.data(), SIGNAL(triggered()), m_boneUIDelegate.data(), SLOT(resetBoneRotation()));
    connect(m_actionBoneResetAll.data(), SIGNAL(triggered()), m_boneUIDelegate.data(), SLOT(resetAllBones()));
    connect(m_actionBoneDialog.data(), SIGNAL(triggered()), m_boneUIDelegate.data(), SLOT(openBoneDialog()));
    connect(m_actionDeleteSelectedModel.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(deleteSelectedModel()));

    connect(m_actionPlay.data(), SIGNAL(triggered()), SLOT(invokePlayer()));
    connect(m_actionPlaySettings.data(), SIGNAL(triggered()), SLOT(openPlaySettingDialog()));
    connect(m_actionOpenGravitySettingsDialog.data(), SIGNAL(triggered()), SLOT(openGravitySettingDialog()));
    connect(m_actionOpenRenderOrderDialog.data(), SIGNAL(triggered()), SLOT(openRenderOrderDialog()));
    connect(m_actionOpenScreenColorDialog.data(), SIGNAL(triggered()), SLOT(openScreenColorDialog()));
    connect(m_actionOpenShadowMapDialog.data(), SIGNAL(triggered()), SLOT(openShadowMapDialog()));
    m_actionEnablePhysics->setCheckable(true);
    m_actionEnablePhysics->setChecked(true);

    m_actionShowGrid->setCheckable(true);
    m_actionShowGrid->setChecked(true);
    connect(m_actionSetBackgroundImage.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(setBackgroundImage()));
    connect(m_actionClearBackgroundImage.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(clearBackgroundImage()));
    connect(m_actionOpenBackgroundImageDialog.data(), SIGNAL(triggered()), SLOT(openBackgroundImageDialog()));

    connect(m_actionZoomIn.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(zoomIn()));
    connect(m_actionZoomOut.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(zoomOut()));
    connect(m_actionRotateUp.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(rotateUp()));
    connect(m_actionRotateDown.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(rotateDown()));
    connect(m_actionRotateLeft.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(rotateLeft()));
    connect(m_actionRotateRight.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(rotateRight()));
    connect(m_actionTranslateUp.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(translateUp()));
    connect(m_actionTranslateDown.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(translateDown()));
    connect(m_actionTranslateLeft.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(translateLeft()));
    connect(m_actionTranslateRight.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(translateRight()));
    connect(m_actionResetCamera.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(resetCamera()));

    connect(m_actionSelectNextModel.data(), SIGNAL(triggered()), SLOT(selectNextModel()));
    connect(m_actionSelectPreviousModel.data(), SIGNAL(triggered()), SLOT(selectPreviousModel()));
    connect(m_actionRevertSelectedModel.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(revertSelectedModel()));
    connect(m_actionTranslateModelUp.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(translateModelUp()));
    connect(m_actionTranslateModelDown.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(translateModelDown()));
    connect(m_actionTranslateModelLeft.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(translateModelLeft()));
    connect(m_actionTranslateModelRight.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(translateModelRight()));
    connect(m_actionResetModelPosition.data(), SIGNAL(triggered()), m_sceneWidget.data(), SLOT(resetModelPosition()));

    connect(m_actionSelectAllKeyframes.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(selectAllRegisteredKeyframes()));
    connect(m_actionSelectKeyframeDialog.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(openFrameSelectionDialog()));
    connect(m_actionKeyframeWeightDialog.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(openFrameWeightDialog()));
    connect(m_actionInterpolationDialog.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(openInterpolationDialogBySelectedIndices()));
    connect(m_actionNextFrame.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(nextFrame()));
    connect(m_actionPreviousFrame.data(), SIGNAL(triggered()), m_timelineTabWidget.data(), SLOT(previousFrame()));

    m_actionEnableEffect->setCheckable(true);
    m_actionEnableEffect->setChecked(false);

    connect(m_actionViewLogMessage.data(), SIGNAL(triggered()), m_loggerWidgetRef, SLOT(show()));
    m_actionEnableGestures->setCheckable(true);
    m_actionEnableGestures->setChecked(m_sceneWidget->isGesturesEnabled());
    connect(m_actionEnableGestures.data(), SIGNAL(triggered(bool)), m_sceneWidget.data(), SLOT(setGesturesEnable(bool)));
    m_actionEnableMoveGesture->setCheckable(true);
    m_actionEnableMoveGesture->setChecked(m_sceneWidget->isMoveGestureEnabled());
    connect(m_actionEnableMoveGesture.data(), SIGNAL(triggered(bool)), m_sceneWidget.data(), SLOT(setMoveGestureEnable(bool)));
    m_actionEnableRotateGesture->setCheckable(true);
    m_actionEnableRotateGesture->setChecked(m_sceneWidget->isRotateGestureEnabled());
    connect(m_actionEnableRotateGesture.data(), SIGNAL(triggered(bool)), m_sceneWidget.data(), SLOT(setRotateGestureEnable(bool)));
    m_actionEnableScaleGesture->setCheckable(true);
    m_actionEnableScaleGesture->setChecked(m_sceneWidget->isRotateGestureEnabled());
    connect(m_actionEnableScaleGesture.data(), SIGNAL(triggered(bool)), m_sceneWidget.data(), SLOT(setScaleGestureEnable(bool)));
    m_actionEnableUndoGesture->setCheckable(true);
    m_actionEnableUndoGesture->setChecked(m_sceneWidget->isUndoGestureEnabled());
    connect(m_actionEnableUndoGesture.data(), SIGNAL(triggered(bool)), m_sceneWidget.data(), SLOT(setUndoGestureEnable(bool)));
    connect(m_actionShowTimelineDock.data(), SIGNAL(triggered()), m_timelineDockWidget.data(), SLOT(show()));
    connect(m_actionShowSceneDock.data(), SIGNAL(triggered()), m_sceneDockWidget.data(), SLOT(show()));
    connect(m_actionShowModelDock.data(), SIGNAL(triggered()), m_modelDockWidget.data(), SLOT(show()));
    m_actionShowModelDialog->setCheckable(true);
    m_actionShowModelDialog->setChecked(m_sceneWidget->showModelDialog());
    connect(m_actionShowModelDialog.data(), SIGNAL(triggered(bool)), m_sceneWidget.data(), SLOT(setShowModelDialog(bool)));

    connect(m_actionClearRecentFiles.data(), SIGNAL(triggered()), SLOT(clearRecentFiles()));
    connect(m_actionAbout.data(), SIGNAL(triggered()), SLOT(showLicenseWidget()));
    m_actionAbout->setMenuRole(QAction::AboutRole);
    connect(m_actionAboutQt.data(), SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    m_actionAboutQt->setMenuRole(QAction::AboutQtRole);
    QActionGroup *accelerationGroup = new QActionGroup(this);
    bool hasAcceleration = Scene::isAcceleratorSupported();
    accelerationGroup->setExclusive(true);
    m_actionSetSoftwareSkinningFallback->setCheckable(true);
    accelerationGroup->addAction(m_actionSetSoftwareSkinningFallback.data());
    m_actionSetOpenCLSkinningType1->setCheckable(hasAcceleration);
    accelerationGroup->addAction(m_actionSetOpenCLSkinningType1.data());
    m_actionSetOpenCLSkinningType2->setCheckable(hasAcceleration);
    accelerationGroup->addAction(m_actionSetOpenCLSkinningType2.data());
    m_actionSetVertexShaderSkinningType1->setCheckable(true);
    accelerationGroup->addAction(m_actionSetVertexShaderSkinningType1.data());
    m_menuAcceleration->addAction(m_actionSetSoftwareSkinningFallback.data());
    m_menuAcceleration->addAction(m_actionSetOpenCLSkinningType1.data());
    m_menuAcceleration->addAction(m_actionSetOpenCLSkinningType2.data());
    m_menuAcceleration->addAction(m_actionSetVertexShaderSkinningType1.data());

#ifndef Q_OS_MACX
    m_menuBar.reset(menuBar());
#endif
    m_menuFile->addAction(m_actionNewProject.data());
    m_menuFile->addAction(m_actionNewMotion.data());
    m_menuFile->addAction(m_actionLoadProject.data());
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionAddModel.data());
    m_menuFile->addAction(m_actionAddAsset.data());
    m_menuFile->addAction(m_actionInsertToAllModels.data());
    m_menuFile->addAction(m_actionInsertToSelectedModel.data());
    m_menuFile->addAction(m_actionSetCamera.data());
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionSaveProject.data());
    m_menuFile->addAction(m_actionSaveProjectAs.data());
    m_menuFile->addAction(m_actionSaveMotion.data());
    m_menuFile->addAction(m_actionSaveMotionAs.data());
    m_menuFile->addAction(m_actionSaveCameraMotionAs.data());
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionLoadModelPose.data());
    m_menuFile->addAction(m_actionSaveModelPose.data());
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionLoadAssetMetadata.data());
    m_menuFile->addAction(m_actionSaveAssetMetadata.data());
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionExportImage.data());
    m_menuFile->addAction(m_actionExportVideo.data());

    m_actionRecentFiles.clear();
    for (int i = 0; i < kMaxRecentFiles; i++) {
        QAction *action = new QAction(this);
        connect(action, SIGNAL(triggered()), SLOT(openRecentFile()));
        action->setVisible(false);
        m_menuRecentFiles->addAction(action);
        m_actionRecentFiles.append(action);
    }
    m_menuRecentFiles->addSeparator();
    m_menuRecentFiles->addAction(m_actionClearRecentFiles.data());
    m_menuFile->addMenu(m_menuRecentFiles.data());
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionExit.data());
    m_menuBar->addMenu(m_menuFile.data());

    m_menuEdit->addAction(m_actionUndo.data());
    m_menuEdit->addAction(m_actionRedo.data());
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(m_actionCut.data());
    m_menuEdit->addAction(m_actionCopy.data());
    m_menuEdit->addAction(m_actionPaste.data());
    m_menuEdit->addAction(m_actionReversedPaste.data());
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(m_actionOpenUndoView.data());
    m_menuBar->addMenu(m_menuEdit.data());

    m_menuProject->addAction(m_actionPlay.data());
    m_menuProject->addAction(m_actionPlaySettings.data());
    m_menuProject->addSeparator();
    m_menuProject->addAction(m_actionOpenGravitySettingsDialog.data());
    m_menuProject->addAction(m_actionOpenRenderOrderDialog.data());
    m_menuProject->addAction(m_actionOpenScreenColorDialog.data());
    m_menuProject->addAction(m_actionOpenShadowMapDialog.data());
    m_menuProject->addSeparator();
    m_menuProject->addMenu(m_menuAcceleration.data());
    m_menuProject->addAction(m_actionEnablePhysics.data());
    m_menuProject->addAction(m_actionShowGrid.data());
    m_menuProject->addSeparator();
    m_menuProject->addAction(m_actionSetBackgroundImage.data());
    m_menuProject->addAction(m_actionClearBackgroundImage.data());
    m_menuProject->addAction(m_actionOpenBackgroundImageDialog.data());
    m_menuBar->addMenu(m_menuProject.data());

    m_menuScene->addAction(m_actionZoomIn.data());
    m_menuScene->addAction(m_actionZoomOut.data());
    m_menuScene->addSeparator();
    m_menuScene->addAction(m_actionRotateUp.data());
    m_menuScene->addAction(m_actionRotateDown.data());
    m_menuScene->addAction(m_actionRotateLeft.data());
    m_menuScene->addAction(m_actionRotateRight.data());
    m_menuScene->addSeparator();
    m_menuScene->addAction(m_actionTranslateUp.data());
    m_menuScene->addAction(m_actionTranslateDown.data());
    m_menuScene->addAction(m_actionTranslateLeft.data());
    m_menuScene->addAction(m_actionTranslateRight.data());
    m_menuScene->addSeparator();
    m_menuScene->addAction(m_actionResetCamera.data());
    m_menuBar->addMenu(m_menuScene.data());


    m_menuModel->addMenu(m_menuRetainModels.data());

    m_menuScene->addMenu(m_menuRetainAssets.data());
    m_menuModel->addAction(m_actionSelectNextModel.data());
    m_menuModel->addAction(m_actionSelectPreviousModel.data());
    m_menuModel->addAction(m_actionRevertSelectedModel.data());
    m_menuModel->addSeparator();
    m_menuModel->addAction(m_actionTranslateModelUp.data());
    m_menuModel->addAction(m_actionTranslateModelDown.data());
    m_menuModel->addAction(m_actionTranslateModelLeft.data());
    m_menuModel->addAction(m_actionTranslateModelRight.data());
    m_menuModel->addSeparator();
    m_menuModel->addAction(m_actionResetModelPosition.data());
    m_menuModel->addSeparator();
    m_menuModel->addAction(m_actionDeleteSelectedModel.data());
    m_menuBar->addMenu(m_menuModel.data());

    m_menuKeyframe->addAction(m_actionRegisterKeyframe.data());
    m_menuKeyframe->addAction(m_actionInsertEmptyFrame.data());
    m_menuKeyframe->addAction(m_actionDeleteSelectedKeyframe.data());
    m_menuKeyframe->addSeparator();
    m_menuKeyframe->addAction(m_actionSelectAllKeyframes.data());
    m_menuKeyframe->addAction(m_actionSelectKeyframeDialog.data());
    m_menuKeyframe->addAction(m_actionKeyframeWeightDialog.data());
    m_menuKeyframe->addAction(m_actionInterpolationDialog.data());
    m_menuKeyframe->addSeparator();
    m_menuKeyframe->addAction(m_actionNextFrame.data());
    m_menuKeyframe->addAction(m_actionPreviousFrame.data());
    m_menuKeyframe->addSeparator();
    m_menuKeyframe->addAction(m_actionBoneXPosZero.data());
    m_menuKeyframe->addAction(m_actionBoneYPosZero.data());
    m_menuKeyframe->addAction(m_actionBoneZPosZero.data());
    m_menuKeyframe->addAction(m_actionBoneResetAll.data());
    m_menuKeyframe->addAction(m_actionBoneDialog.data());
    m_menuBar->addMenu(m_menuKeyframe.data());

    m_menuEffect->addAction(m_actionEnableEffect.data());
    m_menuBar->addMenu(m_menuEffect.data());

    m_menuView->addAction(m_actionViewLogMessage.data());
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionShowTimelineDock.data());
    m_menuView->addAction(m_actionShowSceneDock.data());
    m_menuView->addAction(m_actionShowModelDock.data());
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionShowModelDialog.data());
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionEnableGestures.data());
    m_menuView->addAction(m_actionEnableMoveGesture.data());
    m_menuView->addAction(m_actionEnableRotateGesture.data());
    m_menuView->addAction(m_actionEnableScaleGesture.data());
    m_menuView->addAction(m_actionEnableUndoGesture.data());
    m_menuBar->addMenu(m_menuView.data());
    m_menuHelp->addAction(m_actionAbout.data());
    m_menuHelp->addAction(m_actionAboutQt.data());
    m_menuBar->addMenu(m_menuHelp.data());
    setCentralWidget(m_sceneWidget.data());
    updateRecentFiles();

    m_mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_mainToolBar->setMovable(true);
    m_actionRegisterKeyframeOnToolBar.reset(m_mainToolBar->addAction("", m_timelineTabWidget.data(), SLOT(addKeyframesFromSelectedIndices())));
    m_actionDeleteSelectedKeyframeOnToolBar.reset(m_mainToolBar->addAction("", m_timelineTabWidget.data(), SLOT(deleteKeyframesBySelectedIndices())));
    m_mainToolBar->addSeparator();
    m_actionCreateProjectOnToolBar.reset(m_mainToolBar->addAction("", this, SLOT(newProjectFile())));
    m_actionCreateMotionOnToolBar.reset(m_mainToolBar->addAction("", this, SLOT(newMotionFile())));
    m_actionAddObjectOnToolBar.reset(m_mainToolBar->addAction("", m_sceneWidget.data(), SLOT(addFile())));
    m_actionDeleteModelOnToolBar.reset(m_mainToolBar->addAction("", m_sceneWidget.data(), SLOT(deleteSelectedModel())));
    m_mainToolBar->addSeparator();
    m_actionEnableEffectOnToolBar.reset(m_mainToolBar->addAction(""));
    m_actionEnableEffectOnToolBar->setCheckable(true);
    m_mainToolBar->addSeparator();
    m_actionPlayOnToolBar.reset(m_mainToolBar->addAction("", this, SLOT(invokePlayer())));
    m_actionExportVideoOnToolBar.reset(m_mainToolBar->addAction("", this, SLOT(exportVideo())));
    m_actionExportImageOnToolBar.reset(m_mainToolBar->addAction("", this, SLOT(exportImage())));
    addToolBar(m_mainToolBar.data());

    retranslate();
    bindActions();
}

void MainWindow::bindActions()
{
    static const QString &kPrefix = "keyboardBindings/";
    m_actionNewProject->setShortcut(m_settings.value(kPrefix + "newProject", QKeySequence(QKeySequence::New).toString()).toString());
    m_actionNewMotion->setShortcut(m_settings.value(kPrefix + "newMotion", "Ctrl+Shift+N").toString());
    m_actionLoadProject->setShortcut(m_settings.value(kPrefix + "loadProject", QKeySequence(QKeySequence::Open).toString()).toString());
    m_actionAddModel->setShortcut(m_settings.value(kPrefix + "addModel", "Ctrl+Shift+M").toString());
    m_actionAddAsset->setShortcut(m_settings.value(kPrefix + "addAsset", "Ctrl+Shift+A").toString());
    m_actionInsertToAllModels->setShortcut(m_settings.value(kPrefix + "insertToAllModels", "Ctrl+Shift+V").toString());
    m_actionInsertToSelectedModel->setShortcut(m_settings.value(kPrefix + "insertToSelectedModel", "Ctrl+Alt+Shift+V").toString());
    m_actionSaveProject->setShortcut(m_settings.value(kPrefix + "saveProject", QKeySequence(QKeySequence::Save).toString()).toString());
    m_actionSaveProjectAs->setShortcut(m_settings.value(kPrefix + "saveProjectAs", QKeySequence(QKeySequence::SaveAs).toString()).toString());
    m_actionSaveMotion->setShortcut(m_settings.value(kPrefix + "saveMotion", "").toString());
    m_actionSaveMotionAs->setShortcut(m_settings.value(kPrefix + "saveMotionAs", "").toString());
    m_actionSaveCameraMotionAs->setShortcut(m_settings.value(kPrefix + "saveCameraMotionAs", "").toString());
    m_actionLoadModelPose->setShortcut(m_settings.value(kPrefix + "loadModelPose").toString());
    m_actionSaveModelPose->setShortcut(m_settings.value(kPrefix + "saveModelPose").toString());
    m_actionLoadAssetMetadata->setShortcut(m_settings.value(kPrefix + "loadAssetMetadata").toString());
    m_actionSaveAssetMetadata->setShortcut(m_settings.value(kPrefix + "saveAssetMetadata").toString());
    m_actionExportImage->setShortcut(m_settings.value(kPrefix + "exportImage").toString());
    m_actionExportVideo->setShortcut(m_settings.value(kPrefix + "exportVideo").toString());
    m_actionSetCamera->setShortcut(m_settings.value(kPrefix + "setCamera", "Ctrl+Shift+C").toString());
    m_actionExit->setShortcut(m_settings.value(kPrefix + "exit", QKeySequence(QKeySequence::Quit).toString()).toString());
    m_actionPlay->setShortcut(m_settings.value(kPrefix + "play").toString());
    m_actionPlaySettings->setShortcut(m_settings.value(kPrefix + "playSettings").toString());
    m_actionOpenGravitySettingsDialog->setShortcut(m_settings.value(kPrefix + "gravitySettings").toString());
    m_actionOpenRenderOrderDialog->setShortcut(m_settings.value(kPrefix + "renderOrderDialog").toString());
    m_actionOpenScreenColorDialog->setShortcut(m_settings.value(kPrefix + "screenColorDialog").toString());
    m_actionOpenShadowMapDialog->setShortcut(m_settings.value(kPrefix + "shadowMapDialog").toString());
    m_actionEnablePhysics->setShortcut(m_settings.value(kPrefix + "enablePhysics", "Ctrl+Shift+P").toString());
    m_actionShowGrid->setShortcut(m_settings.value(kPrefix + "showGrid", "Ctrl+Shift+G").toString());
    m_actionSetBackgroundImage->setShortcut(m_settings.value(kPrefix + "setBackgroundImage").toString());
    m_actionClearBackgroundImage->setShortcut(m_settings.value(kPrefix + "clearBackgroundImage").toString());
    m_actionOpenBackgroundImageDialog->setShortcut(m_settings.value(kPrefix + "openBackgroundImageDialog").toString());
    m_actionZoomIn->setShortcut(m_settings.value(kPrefix + "zoomIn", QKeySequence(QKeySequence::ZoomIn).toString()).toString());
    m_actionZoomOut->setShortcut(m_settings.value(kPrefix + "zoomOut", QKeySequence(QKeySequence::ZoomOut).toString()).toString());
    m_actionRotateUp->setShortcut(m_settings.value(kPrefix + "rotateUp", "Ctrl+Up").toString());
    m_actionRotateDown->setShortcut(m_settings.value(kPrefix + "rotateDown", "Ctrl+Down").toString());
    m_actionRotateLeft->setShortcut(m_settings.value(kPrefix + "rotateLeft", "Ctrl+Left").toString());
    m_actionRotateRight->setShortcut(m_settings.value(kPrefix + "rotateRight", "Ctrl+Right").toString());
    m_actionTranslateUp->setShortcut(m_settings.value(kPrefix + "translateUp", "Shift+Up").toString());
    m_actionTranslateDown->setShortcut(m_settings.value(kPrefix + "translateDown", "Shift+Down").toString());
    m_actionTranslateLeft->setShortcut(m_settings.value(kPrefix + "translateLeft", "Shift+Left").toString());
    m_actionTranslateRight->setShortcut(m_settings.value(kPrefix + "translateRight", "Shift+Right").toString());
    m_actionResetCamera->setShortcut(m_settings.value(kPrefix + "resetCamera").toString());
    m_actionSelectNextModel->setShortcut(m_settings.value(kPrefix + "selectNextModel", QKeySequence(QKeySequence::SelectNextPage).toString()).toString());
    m_actionSelectPreviousModel->setShortcut(m_settings.value(kPrefix + "selectPreviousModel", QKeySequence(QKeySequence::SelectPreviousPage).toString()).toString());
    m_actionRevertSelectedModel->setShortcut(m_settings.value(kPrefix + "revertSelectedModel").toString());
    m_actionDeleteSelectedModel->setShortcut(m_settings.value(kPrefix + "deleteSelectedModel", "Ctrl+Shift+Backspace").toString());
    m_actionTranslateModelUp->setShortcut(m_settings.value(kPrefix + "translateModelUp", "Ctrl+Shift+Up").toString());
    m_actionTranslateModelDown->setShortcut(m_settings.value(kPrefix + "translateModelDown", "Ctrl+Shift+Down").toString());
    m_actionTranslateModelLeft->setShortcut(m_settings.value(kPrefix + "translateModelLeft", "Ctrl+Shift+Left").toString());
    m_actionTranslateModelRight->setShortcut(m_settings.value(kPrefix + "translateModelRight", "Ctrl+Shift+Right").toString());
    m_actionResetModelPosition->setShortcut(m_settings.value(kPrefix + "resetModelPosition").toString());
    m_actionBoneXPosZero->setShortcut(m_settings.value(kPrefix + "boneXPosZero").toString());
    m_actionBoneYPosZero->setShortcut(m_settings.value(kPrefix + "boneYPosZero").toString());
    m_actionBoneZPosZero->setShortcut(m_settings.value(kPrefix + "boneZPosZero").toString());
    m_actionBoneRotationZero->setShortcut(m_settings.value(kPrefix + "boneRotationZero").toString());
    m_actionBoneResetAll->setShortcut(m_settings.value(kPrefix + "boneResetAll").toString());
    m_actionBoneDialog->setShortcut(m_settings.value(kPrefix + "boneDialog").toString());
    m_actionRegisterKeyframe->setShortcut(m_settings.value(kPrefix + "registerFrame", "Ctrl+E").toString());
    m_actionSelectAllKeyframes->setShortcut(m_settings.value(kPrefix + "selectAllFrames", "Ctrl+A").toString());
    m_actionSelectKeyframeDialog->setShortcut(m_settings.value(kPrefix + "selectFrameDialog", "Ctrl+Alt+S").toString());
    m_actionKeyframeWeightDialog->setShortcut(m_settings.value(kPrefix + "frameWeightDialog", "Ctrl+Alt+W").toString());
    m_actionInterpolationDialog->setShortcut(m_settings.value(kPrefix + "interpolationDialog", "Ctrl+Alt+I").toString());
    m_actionInsertEmptyFrame->setShortcut(m_settings.value(kPrefix + "insertEmptyFrame", "Ctrl+I").toString());
    m_actionDeleteSelectedKeyframe->setShortcut(m_settings.value(kPrefix + "deleteSelectedFrame", "Ctrl+K").toString());
    m_actionNextFrame->setShortcut(m_settings.value(kPrefix + "nextFrame", QKeySequence(QKeySequence::Forward).toString()).toString());
    m_actionPreviousFrame->setShortcut(m_settings.value(kPrefix + "previousFrame", QKeySequence(QKeySequence::Back).toString()).toString());
    m_actionCut->setShortcut(m_settings.value(kPrefix + "cut", QKeySequence(QKeySequence::Cut).toString()).toString());
    m_actionCopy->setShortcut(m_settings.value(kPrefix + "copy", QKeySequence(QKeySequence::Copy).toString()).toString());
    m_actionPaste->setShortcut(m_settings.value(kPrefix + "paste", QKeySequence(QKeySequence::Paste).toString()).toString());
    m_actionReversedPaste->setShortcut(m_settings.value(kPrefix + "reversedPaste", "Alt+Ctrl+V").toString());
    m_actionUndo->setShortcut(m_settings.value(kPrefix + "undoFrame", QKeySequence(QKeySequence::Undo).toString()).toString());
    m_actionRedo->setShortcut(m_settings.value(kPrefix + "redoFrame", QKeySequence(QKeySequence::Redo).toString()).toString());
    m_actionOpenUndoView->setShortcut(m_settings.value(kPrefix + "undoView").toString());
    m_actionViewLogMessage->setShortcut(m_settings.value(kPrefix + "viewLogMessage").toString());
    m_actionEnableGestures->setShortcut(m_settings.value(kPrefix + "enableGestures").toString());
    m_actionEnableMoveGesture->setShortcut(m_settings.value(kPrefix + "enableMoveGesture").toString());
    m_actionEnableRotateGesture->setShortcut(m_settings.value(kPrefix + "enableRotateGesture").toString());
    m_actionEnableScaleGesture->setShortcut(m_settings.value(kPrefix + "enableScaleGesture").toString());
    m_actionEnableUndoGesture->setShortcut(m_settings.value(kPrefix + "enableUndoGesture").toString());
    m_actionShowTimelineDock->setShortcut(m_settings.value(kPrefix + "showTimelineDock").toString());
    m_actionShowSceneDock->setShortcut(m_settings.value(kPrefix + "showSceneDock").toString());
    m_actionShowModelDock->setShortcut(m_settings.value(kPrefix + "showModelDock").toString());
    m_actionShowModelDialog->setShortcut(m_settings.value(kPrefix + "showModelDialog").toString());
    m_actionAbout->setShortcut(m_settings.value(kPrefix + "about", "Alt+Q, Alt+/").toString());
    m_actionAboutQt->setShortcut(m_settings.value(kPrefix + "aboutQt").toString());
    m_actionClearRecentFiles->setShortcut(m_settings.value(kPrefix + "clearRecentFiles").toString());
    m_actionSetSoftwareSkinningFallback->setShortcut(m_settings.value(kPrefix + "setSoftwareSkinningFallback").toString());
    m_actionSetOpenCLSkinningType1->setShortcut(m_settings.value(kPrefix + "setOpenCLSkinning").toString());
    m_actionSetOpenCLSkinningType2->setShortcut(m_settings.value(kPrefix + "setOpenCLSkinningType2").toString());
    m_actionSetVertexShaderSkinningType1->setShortcut(m_settings.value(kPrefix + "setOpenCLSkinning").toString());
    m_actionEnableEffect->setShortcut(m_settings.value(kPrefix + "enableEffect").toString());
    QShortcut *cameraFront = new QShortcut(m_settings.value(kPrefix + "cameraFront", QKeySequence(Qt::Key_2)).toString(), this);
    connect(cameraFront, SIGNAL(activated()), m_sceneTabWidget->cameraPerspectiveWidgetRef(), SLOT(setCameraPerspectiveFront()));
    QShortcut *cameraBack = new QShortcut(m_settings.value(kPrefix + "cameraBack", QKeySequence(Qt::Key_8)).toString(), this);
    connect(cameraBack, SIGNAL(activated()), m_sceneTabWidget->cameraPerspectiveWidgetRef(), SLOT(setCameraPerspectiveBack()));
    QShortcut *cameraLeft = new QShortcut(m_settings.value(kPrefix + "cameraLeft", QKeySequence(Qt::Key_4)).toString(), this);
    connect(cameraLeft, SIGNAL(activated()), m_sceneTabWidget->cameraPerspectiveWidgetRef(), SLOT(setCameraPerspectiveLeft()));
    QShortcut *cameraRight = new QShortcut(m_settings.value(kPrefix + "cameraRight", QKeySequence(Qt::Key_6)).toString(), this);
    connect(cameraRight, SIGNAL(activated()), m_sceneTabWidget->cameraPerspectiveWidgetRef(), SLOT(setCameraPerspectiveRight()));
    QShortcut *cameraTop = new QShortcut(m_settings.value(kPrefix + "cameraTop", QKeySequence(Qt::Key_5)).toString(), this);
    connect(cameraTop, SIGNAL(activated()), m_sceneTabWidget->cameraPerspectiveWidgetRef(), SLOT(setCameraPerspectiveTop()));
    QShortcut *cameraBottom = new QShortcut(m_settings.value(kPrefix + "cameraBottom", QKeySequence(Qt::Key_0)).toString(), this);
    connect(cameraBottom, SIGNAL(activated()), m_sceneTabWidget->cameraPerspectiveWidgetRef(), SLOT(setCameraPerspectiveBottom()));
}

void MainWindow::retranslate()
{
    m_mainToolBar->setWindowTitle(tr("Toolbar"));
    m_timelineDockWidget->setWindowTitle(tr("Timeline"));
    m_timelineDockWidget->setObjectName(m_timelineDockWidget->windowTitle());
    m_sceneDockWidget->setWindowTitle(tr("Scene"));
    m_sceneDockWidget->setObjectName(m_sceneDockWidget->windowTitle());
    m_modelDockWidget->setWindowTitle(tr("Model"));
    m_modelDockWidget->setObjectName(m_modelDockWidget->windowTitle());
    m_actionNewProject->setText(tr("New project"));
    m_actionNewProject->setStatusTip(tr("Create a new project."));
    m_actionNewMotion->setText(tr("New motion"));
    m_actionNewMotion->setStatusTip(tr("Insert a new motion to the selected model."));
    m_actionLoadProject->setText(tr("Load project"));
    m_actionLoadProject->setStatusTip(tr("Load a project to the scene."));
    m_actionAddModel->setText(tr("Add new model"));
    m_actionAddModel->setStatusTip(tr("Add a model to the scene."));
    m_actionAddAsset->setText(tr("Add new asset"));
    m_actionAddAsset->setStatusTip(tr("Add an asset to the scene."));
    m_actionInsertToAllModels->setText(tr("Insert motion to all models"));
    m_actionInsertToAllModels->setStatusTip(tr("Insert a motion to the all models."));
    m_actionInsertToSelectedModel->setText(tr("Insert motion to selected model"));
    m_actionInsertToSelectedModel->setStatusTip(tr("Insert a motion to the selected model."));
    m_actionSaveProject->setText(tr("Save project"));
    m_actionSaveProject->setStatusTip(tr("Save current project to the current file."));
    m_actionSaveProjectAs->setText(tr("Save project as"));
    m_actionSaveProjectAs->setStatusTip(tr("Save current project as a new project file."));
    m_actionSaveMotion->setText(tr("Save model motion"));
    m_actionSaveMotion->setStatusTip(tr("Export all bone and morph keyframes to the current file."));
    m_actionSaveMotionAs->setText(tr("Save model motion as"));
    m_actionSaveMotionAs->setStatusTip(tr("Export all bone and morph keyframes as a new motion file."));
    m_actionSaveCameraMotionAs->setText(tr("Save camera motion as"));
    m_actionSaveCameraMotionAs->setStatusTip(tr("Export all camera keyframes as a new motion file."));
    m_actionLoadModelPose->setText(tr("Load model pose"));
    m_actionLoadModelPose->setStatusTip(tr("Load a model pose to the selected model."));
    m_actionSaveModelPose->setText(tr("Save model pose"));
    m_actionSaveModelPose->setStatusTip(tr("Save selected frame bones as a model pose file."));
    m_actionLoadAssetMetadata->setText(tr("Load asset metadata"));
    m_actionLoadAssetMetadata->setStatusTip(tr("Load asset from VAC file."));
    m_actionSaveAssetMetadata->setText(tr("Save current asset metadata"));
    m_actionSaveAssetMetadata->setStatusTip(tr("Save current asset metadata as a VAC."));
    m_actionExportImage->setText(tr("Export scene as image"));
    m_actionExportImage->setStatusTip(tr("Export current scene as an image."));
    m_actionExportVideo->setText(tr("Export scene as video"));
    m_actionExportVideo->setStatusTip(tr("Export current scene as a video."));
    m_actionSetCamera->setText(tr("Set camera motion"));
    m_actionSetCamera->setStatusTip(tr("Set a camera motion to the scene."));
    m_actionExit->setText(tr("Exit"));
    m_actionExit->setStatusTip(tr("Exit this application."));
    m_actionPlay->setText(tr("Play"));
    m_actionPlay->setStatusTip(tr("Play current scene."));
    m_actionPlaySettings->setText(tr("Play settings"));
    m_actionPlaySettings->setStatusTip(tr("Open a dialog to set settings of playing scene."));
    m_actionOpenGravitySettingsDialog->setText(tr("Gravity setting"));
    m_actionOpenGravitySettingsDialog->setStatusTip(tr("Open a dialog to set gravity for physics simulation."));
    m_actionOpenRenderOrderDialog->setText(tr("Render order setting"));
    m_actionOpenRenderOrderDialog->setStatusTip(tr("Open a dialog to set order of rendering assets and models."));
    m_actionOpenScreenColorDialog->setText(tr("Screen color setting"));
    m_actionOpenScreenColorDialog->setStatusTip(tr("Open a dialog to set screen color."));
    m_actionOpenShadowMapDialog->setText(tr("Shadow map setting"));
    m_actionOpenShadowMapDialog->setStatusTip(tr("Open a dialog to configure shadow map."));
    m_actionEnablePhysics->setText(tr("Enable physics simulation"));
    m_actionEnablePhysics->setStatusTip(tr("Enable or disable physics simulation using Bullet."));
    m_actionShowGrid->setText(tr("Show grid"));
    m_actionShowGrid->setStatusTip(tr("Show or hide scene grid."));
    m_actionSetBackgroundImage->setText(tr("Set background image"));
    m_actionSetBackgroundImage->setStatusTip(tr("Set a background image to the scene."));
    m_actionClearBackgroundImage->setText(tr("Clear background image"));
    m_actionClearBackgroundImage->setStatusTip(tr("Clear current background image."));
    m_actionOpenBackgroundImageDialog->setText(tr("Background image setting"));
    m_actionOpenBackgroundImageDialog->setStatusTip(tr("Open a dialog to configure background image."));
    m_actionZoomIn->setText(tr("Zoom in"));
    m_actionZoomIn->setStatusTip(tr("Zoom in the scene."));
    m_actionZoomOut->setText(tr("Zoom out"));
    m_actionZoomOut->setStatusTip(tr("Zoom out the scene."));
    m_actionRotateUp->setText(tr("Rotate up"));
    m_actionRotateUp->setStatusTip(tr("Rotate up the scene."));
    m_actionRotateDown->setText(tr("Rotate down"));
    m_actionRotateDown->setStatusTip(tr("Rotate down the scene."));
    m_actionRotateLeft->setText(tr("Rotate left"));
    m_actionRotateLeft->setStatusTip(tr("Rotate left the scene."));
    m_actionRotateRight->setText(tr("Rotate right"));
    m_actionRotateRight->setStatusTip(tr("Rotate right the scene."));
    m_actionTranslateUp->setText(tr("Translate up"));
    m_actionTranslateUp->setStatusTip(tr("Translate up the scene."));
    m_actionTranslateDown->setText(tr("Translate down"));
    m_actionTranslateDown->setStatusTip(tr("Translate down the scene."));
    m_actionTranslateLeft->setText(tr("Translate left"));
    m_actionTranslateLeft->setStatusTip(tr("Translate left the scene."));
    m_actionTranslateRight->setText(tr("Translate right"));
    m_actionTranslateRight->setStatusTip(tr("Translate right the scene."));
    m_actionResetCamera->setText(tr("Reset camera"));
    m_actionResetCamera->setStatusTip(tr("Reset camera perspective."));
    m_actionSelectNextModel->setText(tr("Select next model"));
    m_actionSelectNextModel->setStatusTip(tr("Select the next model."));
    m_actionSelectPreviousModel->setText(tr("Select previous model"));
    m_actionSelectPreviousModel->setStatusTip(tr("Select the previous model."));
    m_actionRevertSelectedModel->setText(tr("Revert selected model"));
    m_actionRevertSelectedModel->setStatusTip(tr("Revert the selected model."));
    m_actionDeleteSelectedModel->setText(tr("Delete selected model"));
    m_actionDeleteSelectedModel->setStatusTip(tr("Delete the selected model from the scene."));
    m_actionTranslateModelUp->setText(tr("Translate selected model up"));
    m_actionTranslateModelUp->setStatusTip(tr("Translate the selected model up."));
    m_actionTranslateModelDown->setText(tr("Translate selected model down"));
    m_actionTranslateModelDown->setStatusTip(tr("Translatethe the selected model down."));
    m_actionTranslateModelLeft->setText(tr("Translate selected model left"));
    m_actionTranslateModelLeft->setStatusTip(tr("Translate the selected model left."));
    m_actionTranslateModelRight->setText(tr("Translate selected model right"));
    m_actionTranslateModelRight->setStatusTip(tr("Translate the selected model right."));
    m_actionResetModelPosition->setText(tr("Reset model position"));
    m_actionResetModelPosition->setStatusTip(tr("Reset the position of selected model to zero."));
    m_actionBoneXPosZero->setText(tr("Make X position of bone zero"));
    m_actionBoneXPosZero->setStatusTip(tr("Reset X axis of the selected bone to the selected model."));
    m_actionBoneYPosZero->setText(tr("Make Y position of bone zero"));
    m_actionBoneYPosZero->setStatusTip(tr("Reset Y axis of the selected bone to the selected model."));
    m_actionBoneZPosZero->setText(tr("Make Z position of bone zero"));
    m_actionBoneZPosZero->setStatusTip(tr("Reset Z axis of the selected bone to the selected model."));
    m_actionBoneRotationZero->setText(tr("Make rotation of bone zero"));
    m_actionBoneRotationZero->setStatusTip(tr("Reset rotation of the selected bone to the selected model."));
    m_actionBoneResetAll->setText(tr("Reset all bone's position and rotation"));
    m_actionBoneResetAll->setStatusTip(tr("Reset all bone's position and rotation to the selected model."));
    m_actionBoneDialog->setText(tr("Open bone dialog"));
    m_actionBoneDialog->setStatusTip(tr("Open bone dialog to change position or rotation of the bone manually."));
    m_actionRegisterKeyframe->setText(tr("Register keyframe"));
    m_actionRegisterKeyframe->setStatusTip(tr("Register keyframes by selected indices from the timeline."));
    m_actionSelectAllKeyframes->setText(tr("Select all keyframes"));
    m_actionSelectAllKeyframes->setStatusTip(tr("Select all registered keyframes."));
    m_actionSelectKeyframeDialog->setText(tr("Open keyframe range selection dialog"));
    m_actionSelectKeyframeDialog->setStatusTip(tr("Open keyframe range selection dialog to select multiple frame indices."));
    m_actionKeyframeWeightDialog->setText(tr("Open keyframe weight dialog"));
    m_actionKeyframeWeightDialog->setStatusTip(tr("Open keyframe weight dialog to set weight to selected registered keyframes."));
    m_actionInterpolationDialog->setText(tr("Open interpolation dialog"));
    m_actionInterpolationDialog->setStatusTip(tr("Open interpolation dialog to configure interpolation parameter of keyframes."));
    m_actionInsertEmptyFrame->setText(tr("Insert empty keyframe"));
    m_actionInsertEmptyFrame->setStatusTip(tr("Insert an empty keyframe to the selected keyframe."));
    m_actionDeleteSelectedKeyframe->setText(tr("Delete selected keyframe"));
    m_actionDeleteSelectedKeyframe->setStatusTip(tr("Delete a selected keyframe."));
    m_actionNextFrame->setText(tr("Next keyframe"));
    m_actionNextFrame->setStatusTip(tr("Select a next keyframe from the current keyframe."));
    m_actionPreviousFrame->setText(tr("Previous keyframe"));
    m_actionPreviousFrame->setStatusTip(tr("Select a previous keyframe from the current keyframe."));
    m_actionCut->setText(tr("Cut"));
    m_actionCut->setStatusTip(tr("Cut a selected keyframe."));
    m_actionCopy->setText(tr("Copy"));
    m_actionCopy->setStatusTip(tr("Copy a selected keyframe."));
    m_actionPaste->setText(tr("Paste"));
    m_actionPaste->setStatusTip(tr("Paste a selected keyframe."));
    m_actionReversedPaste->setText(tr("Paste with reversed"));
    m_actionReversedPaste->setStatusTip(tr("Paste a selected keyframe with reversed."));
    m_actionOpenUndoView->setText(tr("Undo history"));
    m_actionOpenUndoView->setStatusTip(tr("Open a window to view undo history."));
    m_actionViewLogMessage->setText(tr("Logger Window"));
    m_actionViewLogMessage->setStatusTip(tr("Open logger window."));
    m_actionEnableGestures->setText(tr("Enable gestures feature"));
    m_actionEnableGestures->setStatusTip(tr("Enable below gesture features."));
    m_actionEnableMoveGesture->setText(tr("Enable move gesture"));
    m_actionEnableMoveGesture->setStatusTip(tr("Enable moving scene/model/bone by pan gesture."));
    m_actionEnableRotateGesture->setText(tr("Enable rotate gesture"));
    m_actionEnableRotateGesture->setStatusTip(tr("Enable rotate scene/model/bone by pinch gesture."));
    m_actionEnableScaleGesture->setText(tr("Enable scale gesture"));
    m_actionEnableScaleGesture->setStatusTip(tr("Enable scale scene by pinch gesture."));
    m_actionEnableUndoGesture->setText(tr("Enable undo gesture"));
    m_actionEnableUndoGesture->setStatusTip(tr("Enable undo or redo by swipe gesture."));
    m_actionShowTimelineDock->setText(tr("Show timeline tab"));
    m_actionShowTimelineDock->setStatusTip(tr("Show timeline tab if it's closed."));
    m_actionShowSceneDock->setText(tr("Show scene tab"));
    m_actionShowSceneDock->setStatusTip(tr("Show scene tab if it's closed."));
    m_actionShowModelDock->setText(tr("Show model tab"));
    m_actionShowModelDock->setStatusTip(tr("Show model tab if it's closed."));
    m_actionShowModelDialog->setText(tr("Show model dialog"));
    m_actionShowModelDialog->setStatusTip(tr("Show or hide model dialog when the model is loaded."));
    m_actionAbout->setText(tr("About"));
    m_actionAbout->setStatusTip(tr("About this application."));
    m_actionAboutQt->setText(tr("About Qt"));
    m_actionAboutQt->setStatusTip(tr("About Qt."));
    m_actionClearRecentFiles->setText(tr("Clear recent files history"));
    m_actionClearRecentFiles->setStatusTip(tr("Clear the history of recently opened files."));
    m_actionRegisterKeyframeOnToolBar->setIcon(QIcon(":icons/ok.png"));
    m_actionRegisterKeyframeOnToolBar->setIconText(tr("Register Keyframes"));
    m_actionRegisterKeyframeOnToolBar->setStatusTip(m_actionRegisterKeyframe->statusTip());
    m_actionDeleteSelectedKeyframeOnToolBar->setIcon(QIcon(":icons/remove.png"));
    m_actionDeleteSelectedKeyframeOnToolBar->setIconText(tr("Delete Keyframes"));
    m_actionDeleteSelectedKeyframeOnToolBar->setStatusTip(m_actionDeleteSelectedKeyframeOnToolBar->statusTip());
    m_actionCreateProjectOnToolBar->setIcon(QIcon(":icons/folder.png"));
    m_actionCreateProjectOnToolBar->setIconText(tr("Create Project"));
    m_actionCreateProjectOnToolBar->setStatusTip(m_actionNewProject->statusTip());
    m_actionCreateMotionOnToolBar->setIcon(QIcon(":icons/file.png"));
    m_actionCreateMotionOnToolBar->setIconText(tr("Create Motion"));
    m_actionCreateMotionOnToolBar->setStatusTip(tr("Create an empty model motion (discards previous model motion)."));
    m_actionAddObjectOnToolBar->setIcon(QIcon(":icons/circle-plus.png"));
    m_actionAddObjectOnToolBar->setIconText(tr("Add Model/Motion"));
    m_actionAddObjectOnToolBar->setStatusTip(m_actionAddModel->statusTip());
    m_actionDeleteModelOnToolBar->setIcon(QIcon(":icons/circle-minus.png"));
    m_actionDeleteModelOnToolBar->setIconText(tr("Delete Model"));
    m_actionDeleteModelOnToolBar->setStatusTip(m_actionDeleteSelectedModel->statusTip());
    m_actionEnableEffectOnToolBar->setIcon(QIcon(":icons/magic.png"));
    m_actionEnableEffectOnToolBar->setIconText(tr("Effect"));
    m_actionEnableEffectOnToolBar->setStatusTip(m_actionEnableEffect->statusTip());
    m_actionPlayOnToolBar->setIcon(QIcon(":icons/play.png"));
    m_actionPlayOnToolBar->setIconText(tr("Play"));
    m_actionPlayOnToolBar->setStatusTip(m_actionPlay->statusTip());
    m_actionExportVideoOnToolBar->setIcon(QIcon(":icons/film.png"));
    m_actionExportVideoOnToolBar->setIconText(tr("Export Video"));
    m_actionExportVideoOnToolBar->setStatusTip(m_actionExportVideo->statusTip());
    m_actionExportImageOnToolBar->setIcon(QIcon(":icons/picture.png"));
    m_actionExportImageOnToolBar->setIconText(tr("Export Image"));
    m_actionExportImageOnToolBar->setStatusTip(m_actionExportImage->statusTip());
    m_actionSetSoftwareSkinningFallback->setText(tr("Software skinning"));
    m_actionSetSoftwareSkinningFallback->setStatusTip(tr("Enable software skinning. This is default and stable but slow."));
    m_actionSetOpenCLSkinningType1->setText(tr("OpenCL skinning (GPU)"));
    m_actionSetOpenCLSkinningType1->setStatusTip(tr("Enable OpenCL skinning with GPU. This is fast (faster than vertex shader skinning by case) but maybe causes unstable."));
    m_actionSetOpenCLSkinningType2->setText(tr("OpenCL skinning (CPU only)"));
    m_actionSetOpenCLSkinningType2->setStatusTip(tr("Enable OpenCL skinning with CPU. This is slower than OpenCL skinning with GPU but faster than software skinning and stable basically."));
    m_actionSetVertexShaderSkinningType1->setText(tr("Vertex shader skinning"));
    m_actionSetVertexShaderSkinningType1->setStatusTip(tr("Enable Vertex shader skinning. This is fast but maybe causes unstable."));
    m_actionEnableEffect->setText(tr("Enable effect"));
    m_actionEnableEffect->setStatusTip(tr("Enable effect feature using NVIDIA CgFX (under development)."));
    m_menuFile->setTitle(tr("&File"));
    m_menuEdit->setTitle(tr("&Edit"));
    m_menuProject->setTitle(tr("&Project"));
    m_menuScene->setTitle(tr("&Scene"));
    m_menuModel->setTitle(tr("&Model"));
    m_menuKeyframe->setTitle(tr("&Keyframe"));
    m_menuEffect->setTitle(tr("Effect"));
    m_menuView->setTitle(tr("&View"));
    m_menuRetainAssets->setTitle(tr("Select asset"));
    m_menuRetainModels->setTitle(tr("Select model"));
    m_menuRecentFiles->setTitle(tr("Open recent files"));
    m_menuHelp->setTitle(tr("&Help"));
    m_menuAcceleration->setTitle(tr("Set acceleration type"));
}

void MainWindow::bindSceneLoader()
{
    SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
    AssetWidget *assetWidget = m_sceneTabWidget->assetWidgetRef();
    connect(loader, SIGNAL(modelDidAdd(IModel*,QUuid)), SLOT(addModel(IModel*,QUuid)));
    connect(loader, SIGNAL(modelWillDelete(IModel*,QUuid)), SLOT(deleteModel(IModel*,QUuid)));
    connect(loader, SIGNAL(modelWillDelete(IModel*,QUuid)), m_boneMotionModel.data(), SLOT(removeModel()));
    connect(loader, SIGNAL(motionDidAdd(IMotion*,const IModel*,QUuid)), m_boneMotionModel.data(), SLOT(loadMotion(IMotion*,const IModel*)));
    connect(loader, SIGNAL(modelDidMakePose(VPDFilePtr,IModel*)), m_timelineTabWidget.data(), SLOT(loadPose(VPDFilePtr,IModel*)));
    connect(loader, SIGNAL(modelWillDelete(IModel*,QUuid)), m_morphMotionModel.data(), SLOT(removeModel()));
    connect(loader, SIGNAL(motionDidAdd(IMotion*,const IModel*,QUuid)), m_morphMotionModel.data(), SLOT(loadMotion(IMotion*,const IModel*)));
    connect(loader, SIGNAL(modelDidAdd(IModel*,QUuid)), assetWidget, SLOT(addModel(IModel*)));
    connect(loader, SIGNAL(modelWillDelete(IModel*,QUuid)), assetWidget, SLOT(removeModel(IModel*)));
    connect(loader, SIGNAL(modelWillDelete(IModel*,QUuid)), m_timelineTabWidget.data(), SLOT(clearLastSelectedModel()));
    connect(loader, SIGNAL(modelDidAdd(IModel*,QUuid)), m_timelineTabWidget.data(), SLOT(notifyCurrentTabIndex()));
    connect(loader, SIGNAL(motionDidAdd(IMotion*,const IModel*,QUuid)), m_sceneMotionModel.data(), SLOT(loadMotion(IMotion*)));
    connect(loader, SIGNAL(cameraMotionDidSet(IMotion*,QUuid)), m_sceneMotionModel.data(), SLOT(loadMotion(IMotion*)));
    connect(loader, SIGNAL(projectDidInitialized()), this, SLOT(resetSceneToModels()));
    connect(loader, SIGNAL(projectDidLoad(bool)), m_sceneWidget.data(), SLOT(refreshScene()));
    connect(loader, SIGNAL(projectDidLoad(bool)), m_sceneMotionModel.data(), SLOT(markAsNew()));
    connect(loader, SIGNAL(modelDidSelect(IModel*,SceneLoader*)), SLOT(setCurrentModel(IModel*)));
    connect(loader, SIGNAL(modelDidSelect(IModel*,SceneLoader*)), m_boneMotionModel.data(), SLOT(setPMDModel(IModel*)));
    connect(loader, SIGNAL(modelDidSelect(IModel*,SceneLoader*)), m_morphMotionModel.data(), SLOT(setPMDModel(IModel*)));
    connect(loader, SIGNAL(modelDidSelect(IModel*,SceneLoader*)), m_modelTabWidget->modelInfoWidget(), SLOT(setModel(IModel*)));
    connect(loader ,SIGNAL(modelDidSelect(IModel*,SceneLoader*)), m_modelTabWidget->modelSettingWidget(), SLOT(setModel(IModel*,SceneLoader*)));
    connect(loader, SIGNAL(modelDidSelect(IModel*,SceneLoader*)), m_timelineTabWidget.data(), SLOT(setLastSelectedModel(IModel*)));
    connect(loader, SIGNAL(modelDidSelect(IModel*,SceneLoader*)), assetWidget, SLOT(setAssetProperties(IModel*,SceneLoader*)));
    connect(loader, SIGNAL(effectDidEnable(bool)), m_actionEnableEffect.data(), SLOT(setChecked(bool)));
    connect(loader, SIGNAL(effectDidEnable(bool)), m_actionEnableEffectOnToolBar.data(), SLOT(setChecked(bool)));
    connect(m_actionEnableEffect.data(), SIGNAL(triggered(bool)), loader, SLOT(setEffectEnable(bool)));
    connect(m_actionEnableEffectOnToolBar.data(), SIGNAL(toggled(bool)), loader, SLOT(setEffectEnable(bool)));
    connect(m_actionEnableEffect.data(), SIGNAL(triggered(bool)), m_actionEnableEffectOnToolBar.data(), SLOT(setChecked(bool)));
    connect(m_actionEnableEffectOnToolBar.data(), SIGNAL(toggled(bool)), m_actionEnableEffect.data(), SLOT(setChecked(bool)));
    connect(m_actionEnablePhysics.data(), SIGNAL(triggered(bool)), loader, SLOT(setPhysicsEnabled(bool)));
    connect(m_actionSetSoftwareSkinningFallback.data(), SIGNAL(toggled(bool)), loader, SLOT(setSoftwareSkinningEnable(bool)));
    connect(m_actionSetOpenCLSkinningType1.data(), SIGNAL(toggled(bool)), loader, SLOT(setOpenCLSkinningEnableType1(bool)));
    connect(m_actionSetOpenCLSkinningType2.data(), SIGNAL(toggled(bool)), loader, SLOT(setOpenCLSkinningEnableType2(bool)));
    connect(m_actionSetVertexShaderSkinningType1.data(), SIGNAL(toggled(bool)), loader, SLOT(setVertexShaderSkinningType1Enable(bool)));
    connect(m_actionShowGrid.data(), SIGNAL(toggled(bool)), loader, SLOT(setGridVisible(bool)));
    connect(assetWidget, SIGNAL(assetDidRemove(IModel*)), loader, SLOT(deleteModelSlot(IModel*)));
    connect(assetWidget, SIGNAL(assetDidSelect(IModel*)), loader, SLOT(setSelectedModel(IModel*)));
    Handles *handles = m_sceneWidget->handlesRef();
    connect(m_boneMotionModel.data(), SIGNAL(positionDidChange(IBone*,Vector3)), handles, SLOT(updateHandleModel()));
    connect(m_boneMotionModel.data(), SIGNAL(rotationDidChange(IBone*,Quaternion)), handles, SLOT(updateHandleModel()));
    connect(m_undo.data(), SIGNAL(indexChanged(int)), handles, SLOT(updateHandleModel()));
    connect(m_timelineTabWidget.data(), SIGNAL(currentModelDidChange(IModel*,SceneWidget::EditMode)),
            m_sceneWidget.data(), SLOT(setSelectedModel(IModel*,SceneWidget::EditMode)));
    /* カメラの初期値を設定。シグナル発行前に行う */
    CameraPerspectiveWidget *cameraWidget = m_sceneTabWidget->cameraPerspectiveWidgetRef();
    Scene *scene = m_sceneWidget->sceneLoaderRef()->sceneRef();
    const ICamera *camera = scene->camera();
    cameraWidget->setCameraPerspective(camera);
    connect(cameraWidget, SIGNAL(cameraPerspectiveDidChange(QSharedPointer<ICamera>)),
            m_sceneWidget.data(), SLOT(setCameraPerspective(QSharedPointer<ICamera>)));
    connect(cameraWidget, SIGNAL(cameraPerspectiveDidReset()), m_sceneWidget.data(), SLOT(refreshScene()));
    connect(m_sceneWidget.data(), SIGNAL(cameraPerspectiveDidSet(const ICamera*)),
            cameraWidget, SLOT(setCameraPerspective(const ICamera*)));
    connect(m_sceneWidget.data(), SIGNAL(modelDidMove(Vector3)), cameraWidget, SLOT(setPositionFromModel(Vector3)));
    connect(m_boneMotionModel.data(), SIGNAL(bonesDidSelect(QList<IBone*>)), cameraWidget, SLOT(setPositionFromBone(QList<IBone*>)));
    /* 光源の初期値を設定。シグナル発行前に行う */
    SceneLightWidget *lightWidget = m_sceneTabWidget->sceneLightWidgetRef();
    const ILight *light = scene->light();
    lightWidget->setColor(light->color());
    lightWidget->setDirection(light->direction());
    m_boneMotionModel->setSceneRef(scene);
    m_morphMotionModel->setSceneRef(scene);
    connect(loader, SIGNAL(lightColorDidSet(Vector3)), lightWidget, SLOT(setColor(Vector3)));
    connect(loader, SIGNAL(lightDirectionDidSet(Vector3)), lightWidget, SLOT(setDirection(Vector3)));
    connect(lightWidget, SIGNAL(lightColorDidSet(Vector3)), loader, SLOT(setLightColor(Vector3)));
    connect(lightWidget, SIGNAL(lightDirectionDidSet(Vector3)), loader, SLOT(setLightDirection(Vector3)));
    connect(m_sceneMotionModel.data(), SIGNAL(cameraMotionDidLoad()), loader, SLOT(setProjectDirtyFalse()));
    connect(m_sceneMotionModel.data(), SIGNAL(cameraMotionDidLoad()), m_sceneMotionModel.data(), SLOT(markAsNew()));
    connect(m_sceneMotionModel.data(), SIGNAL(cameraMotionDidLoad()), SLOT(disconnectInitialSlots()));
    /* アクセラレーションの状態を設定 */
    m_actionSetSoftwareSkinningFallback->setChecked(false);
    if (loader->isOpenCLSkinningType1Enabled()) {
        m_actionSetOpenCLSkinningType1->setChecked(true);
    }
    else if (loader->isOpenCLSkinningType2Enabled()) {
        m_actionSetOpenCLSkinningType2->setChecked(true);
    }
    else if (loader->isVertexShaderSkinningType1Enabled()) {
        m_actionSetVertexShaderSkinningType1->setChecked(true);
    }
    else if (Scene::isAcceleratorSupported()) {
        m_actionSetOpenCLSkinningType2->setChecked(true);
    }
    else {
        m_actionSetSoftwareSkinningFallback->setChecked(true);
    }
    /*
     * 空のカメラモーションを登録
     * アクセラレーションの設定の後にやるのは setDirtyFalse の後に アクセラレーションの設定変更によって
     * プロジェクト更新がかかり、何もしていないのに終了時に確認ダイアログが出てしまうことを防ぐため
     */
    IMotionPtr cameraMotionPtr;
    loader->newCameraMotion(cameraMotionPtr);
    loader->setCameraMotion(cameraMotionPtr.take());
}

void MainWindow::bindWidgets()
{
    connect(m_sceneWidget.data(), SIGNAL(initailizeGLContextDidDone()), SLOT(bindSceneLoader()));
    connect(m_sceneWidget.data(), SIGNAL(fileDidLoad(QString)), SLOT(addRecentFile(QString)));
    connect(m_sceneWidget.data(), SIGNAL(handleDidMoveAbsolute(Vector3,IBone*,int)), m_boneMotionModel.data(), SLOT(translateTo(Vector3,IBone*,int)));
    connect(m_sceneWidget.data(), SIGNAL(handleDidMoveRelative(Vector3,IBone*,int)), m_boneMotionModel.data(), SLOT(translateDelta(Vector3,IBone*,int)));
    connect(m_sceneWidget.data(), SIGNAL(handleDidRotate(Scalar,IBone*,int)), m_boneMotionModel.data(), SLOT(rotateAngle(Scalar,IBone*,int)));
    connect(m_sceneWidget.data(), SIGNAL(bonesDidSelect(QList<IBone*>)), m_timelineTabWidget.data(), SLOT(selectBones(QList<IBone*>)));
    connect(m_sceneWidget.data(), SIGNAL(morphsDidSelect(QList<IMorph*>)), m_timelineTabWidget.data(), SLOT(selectMorphs(QList<IMorph*>)));
    connect(m_timelineTabWidget.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)),  m_sceneWidget.data(), SLOT(seekMotion(IKeyframe::TimeIndex,bool,bool)));
    connect(m_boneMotionModel.data(), SIGNAL(motionDidModify(bool)), SLOT(setWindowModified(bool)));
    connect(m_morphMotionModel.data(), SIGNAL(motionDidModify(bool)), SLOT(setWindowModified(bool)));
    connect(m_sceneWidget.data(), SIGNAL(newMotionDidSet(IModel*)), m_boneMotionModel.data(), SLOT(markAsNew(IModel*)));
    connect(m_sceneWidget.data(), SIGNAL(newMotionDidSet(IModel*)), m_morphMotionModel.data(), SLOT(markAsNew(IModel*)));
    connect(m_boneMotionModel.data(), SIGNAL(motionDidUpdate(IModel*)), m_sceneWidget.data(), SLOT(refreshMotions()));
    connect(m_morphMotionModel.data(), SIGNAL(motionDidUpdate(IModel*)), m_sceneWidget.data(), SLOT(refreshMotions()));
    connect(m_sceneWidget.data(), SIGNAL(newMotionDidSet(IModel*)), m_timelineTabWidget.data(), SLOT(setCurrentFrameIndexZero()));
    connect(m_modelTabWidget->morphWidget(), SIGNAL(morphDidRegister(IMorph*)), m_timelineTabWidget.data(), SLOT(addMorphKeyframesAtCurrentFrameIndex(IMorph*)));
    connect(m_sceneWidget.data(), SIGNAL(newMotionDidSet(IModel*)), m_sceneMotionModel.data(), SLOT(markAsNew()));
    connect(m_sceneWidget.data(), SIGNAL(handleDidGrab()), m_boneMotionModel.data(), SLOT(saveTransform()));
    connect(m_sceneWidget.data(), SIGNAL(handleDidRelease()), m_boneMotionModel.data(), SLOT(commitTransform()));
    connect(m_sceneWidget.data(), SIGNAL(cameraPerspectiveDidSet(const ICamera*)), m_boneMotionModel.data(), SLOT(setCamera(const ICamera*)));
    connect(m_sceneWidget.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex)), m_modelTabWidget->morphWidget(), SLOT(updateMorphWeightValues()));
    connect(m_sceneWidget.data(), SIGNAL(undoDidRequest()), m_undo.data(), SLOT(undo()));
    connect(m_sceneWidget.data(), SIGNAL(redoDidRequest()), m_undo.data(), SLOT(redo()));
    connect(m_timelineTabWidget.data(), SIGNAL(editModeDidSet(SceneWidget::EditMode)), m_sceneWidget.data(), SLOT(setEditMode(SceneWidget::EditMode)));
    ModelSettingWidget *modelSettingWidget = m_modelTabWidget->modelSettingWidget();
    connect(modelSettingWidget, SIGNAL(edgeOffsetDidChange(double)), m_sceneWidget.data(), SLOT(setModelEdgeOffset(double)));
    connect(modelSettingWidget, SIGNAL(opacityDidChange(Scalar)), m_sceneWidget.data(), SLOT(setModelOpacity(Scalar)));
    connect(modelSettingWidget, SIGNAL(projectiveShadowDidEnable(bool)), m_sceneWidget.data(), SLOT(setModelProjectiveShadowEnable(bool)));
    connect(modelSettingWidget, SIGNAL(selfShadowDidEnable(bool)), m_sceneWidget.data(), SLOT(setModelSelfShadowEnable(bool)));
    connect(modelSettingWidget, SIGNAL(positionOffsetDidChange(Vector3)), m_sceneWidget.data(), SLOT(setModelPositionOffset(Vector3)));
    connect(modelSettingWidget, SIGNAL(rotationOffsetDidChange(Vector3)), m_sceneWidget.data(), SLOT(setModelRotationOffset(Vector3)));
    connect(m_sceneWidget.data(), SIGNAL(modelDidMove(Vector3)), modelSettingWidget, SLOT(setPositionOffset(Vector3)));
    MorphWidget *morphWidget = m_modelTabWidget->morphWidget();
    connect(morphWidget, SIGNAL(morphWillChange()), m_morphMotionModel.data(), SLOT(saveTransform()));
    connect(morphWidget, SIGNAL(morphDidChange()), m_morphMotionModel.data(), SLOT(commitTransform()));
    connect(m_undo.data(), SIGNAL(indexChanged(int)), morphWidget, SLOT(updateMorphWeightValues()));
    enableSelectingBonesAndMorphs();
}

void MainWindow::insertMotionToAllModels()
{
    if (maybeSaveMotion())
        m_sceneWidget->insertMotionToAllModels();
}

void MainWindow::insertMotionToSelectedModel()
{
    if (maybeSaveMotion())
        m_sceneWidget->insertMotionToSelectedModel();
}

void MainWindow::deleteSelectedModel()
{
    if (maybeSaveMotion())
        m_sceneWidget->deleteSelectedModel();
}

void MainWindow::saveModelPose()
{
    const QString &filename = openSaveDialog("mainWindow/lastPoseFileDirectory",
                                             tr("Save model pose as a VPD file"),
                                             tr("VPD file (*.vpd)"),
                                             tr("untitled.vpd"),
                                             &m_settings);
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QFile::WriteOnly)) {
            VPDFile pose;
            QTextStream stream(&file);
            m_timelineTabWidget->savePose(&pose, m_sceneWidget->sceneLoaderRef()->selectedModelRef());
            pose.save(stream);
            file.close();
            qDebug("Saved a pose: %s", qPrintable(filename));
        }
        else {
            qWarning("Failed saving VPD: %s", qPrintable(file.errorString()));
        }
    }
}

void MainWindow::saveAssetMetadata()
{
    m_sceneWidget->saveMetadataFromAsset(m_sceneTabWidget->assetWidgetRef()->currentAsset());
}

void MainWindow::exportImage()
{
    if (!m_exportingVideoDialog) {
        SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
        const QSize &min = m_sceneWidget->size();
        const QSize &max = m_sceneWidget->maximumSize();
        m_exportingVideoDialog.reset(new ExportVideoDialog(loader, min, max, &m_settings));
    }
    connect(m_exportingVideoDialog.data(), SIGNAL(settingsDidSave()), this, SLOT(invokeImageExporter()));
    m_exportingVideoDialog->setImageConfiguration(true);
    m_exportingVideoDialog->exec();
    m_exportingVideoDialog->setImageConfiguration(false);
}

void MainWindow::exportVideo()
{
    if (VideoEncoder::isSupported()) {
        SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
        if (loader->sceneRef()->maxFrameIndex() > 0) {
            if (!m_exportingVideoDialog) {
                const QSize min(160, 160);
                const QSize &max = m_sceneWidget->maximumSize();
                m_exportingVideoDialog.reset(new ExportVideoDialog(loader, min, max, &m_settings));
            }
            connect(m_exportingVideoDialog.data(), SIGNAL(settingsDidSave()), this, SLOT(invokeVideoEncoder()));
            m_exportingVideoDialog->open();
        }
        else {
            warning(this, tr("No motion to export."),
                    tr("You must create or load a motion to export a video."));
        }
    }
    else {
        warning(this, tr("Exporting video feature is not supported."),
                tr("Exporting video is disabled because OpenCV is not linked."));
    }
}

void MainWindow::invokeImageExporter()
{
    disconnect(m_exportingVideoDialog.data(), SIGNAL(settingsDidSave()), this, SLOT(invokeImageExporter()));
    m_exportingVideoDialog->close();
    const QString &filename = openSaveDialog("mainWindow/lastImageDirectory",
                                             tr("Export scene as an image"),
                                             tr("Image (*.bmp, *.jpg, *.png)"),
                                             tr("untitled.png"),
                                             &m_settings);
    if (!filename.isEmpty()) {
        WindowState state;
        QSize videoSize(m_exportingVideoDialog->sceneWidth(), m_exportingVideoDialog->sceneHeight());
        state.isImage = true;
        saveWindowStateAndResize(videoSize, state);
        m_sceneWidget->updateGL();
        const QImage &image = m_sceneWidget->grabFrameBuffer();
        restoreWindowState(state);
        m_sceneWidget->updateGL();
        if (!image.isNull())
            image.save(filename);
        else
            qWarning("Failed exporting scene as an image: %s", qPrintable(filename));
    }
}

void MainWindow::invokeVideoEncoder()
{
    disconnect(m_exportingVideoDialog.data(), SIGNAL(settingsDidSave()), this, SLOT(invokeVideoEncoder()));
    m_exportingVideoDialog->close();
    int fromIndex = m_exportingVideoDialog->fromIndex();
    int toIndex = m_exportingVideoDialog->toIndex();
    if (fromIndex == toIndex) {
        warning(this, tr("Value of \"Index from\" and \"Index to\" are equal."),
                tr("Specify different value of \"Index from\" and \"Index to\"."));
        return;
    }
    else if (fromIndex > toIndex) {
        warning(this, tr("Value of \"Index from\" is bigger than \"Index to\"."),
                tr("\"Index from\" must be less than \"Index to\"."));
        return;
    }
    const QString &filename = openSaveDialog("mainWindow/lastVideoDirectory",
                                             tr("Export scene as a video"),
                                             tr("Video (*.mov)"),
                                             tr("untitled.mov"),
                                             &m_settings);
    if (!filename.isEmpty()) {
        QScopedPointer<QProgressDialog> progress(new QProgressDialog(this));
        progress->setCancelButtonText(tr("Cancel"));
        progress->setWindowModality(Qt::ApplicationModal);
        int width = m_exportingVideoDialog->sceneWidth();
        int height = m_exportingVideoDialog->sceneHeight();
        /* 終了するまで待つ */
        if (m_audioDecoder && !m_audioDecoder->isFinished()) {
            m_audioDecoder->stopSession();
            m_audioDecoder->waitUntilComplete();
        }
        if (m_videoEncoder && !m_videoEncoder->isFinished()) {
            m_videoEncoder->stopSession();
            m_videoEncoder->waitUntilComplete();
        }
        int sceneFPS = m_exportingVideoDialog->sceneFPS();
        m_audioDecoder.reset(new AudioDecoder());
        m_audioDecoder->setFileName(m_sceneWidget->sceneLoaderRef()->backgroundAudio());
        bool canOpenAudio = m_audioDecoder->canOpen();
        int sampleRate = 0, bitRate = 0;
        if (canOpenAudio) {
            sampleRate = 44100;
            bitRate = 64000;
        }
        /* エンコード設定（ファイル名決定やFPSの設定など） */
        const QSize videoSize(width, height);
        m_videoEncoder.reset(new VideoEncoder(this));
        m_videoEncoder->setFileName(filename);
        m_videoEncoder->setSceneFPS(sceneFPS);
        m_videoEncoder->setSceneSize(videoSize);
        connect(static_cast<AudioDecoder *>(m_audioDecoder.data()), SIGNAL(audioDidDecode(QByteArray)),
                static_cast<VideoEncoder *>(m_videoEncoder.data()), SLOT(audioSamplesDidQueue(QByteArray)));
        connect(this, SIGNAL(sceneDidRendered(QImage)),
                static_cast<VideoEncoder *>(m_videoEncoder.data()), SLOT(videoFrameDidQueue(QImage)));
        m_sceneWidget->setPreferredFPS(sceneFPS);
        const QString &exportingFormat = tr("Exporting frame %1 of %2...");
        int maxRangeIndex = toIndex - fromIndex;
        progress->setRange(0, maxRangeIndex);
        WindowState state;
        saveWindowStateAndResize(videoSize, state);
        /* モーションを0フレーム目に移動し、その後指定のキーフレームのインデックスに advance で移動させる */
        m_sceneWidget->seekMotion(0.0f, true, true);
        m_sceneWidget->advanceMotion(fromIndex);
        progress->setLabelText(exportingFormat.arg(0).arg(maxRangeIndex));
        /* 指定のキーフレームまで動画にフレームの書き出しを行う */
        m_videoEncoder->startSession();
        if (canOpenAudio)
            m_audioDecoder->startSession();
        const IKeyframe::TimeIndex &advanceSecond = 1.0f / (sceneFPS / Scene::defaultFPS());
        IKeyframe::TimeIndex totalAdvanced = 0.0f;
        /* 全てのモーションが終了するまでエンコード処理 */
        const Scene *scene = m_sceneWidget->sceneLoaderRef()->sceneRef();
        Q_UNUSED(scene)
        while (!scene->isReachedTo(toIndex)) {
            if (progress->wasCanceled())
                break;
            const QImage &image = m_sceneWidget->grabFrameBuffer();
            if (image.width() != width || image.height() != height)
                emit sceneDidRendered(image.scaled(width, height));
            else
                emit sceneDidRendered(image);
            int value = progress->value();
            if (totalAdvanced >= 1.0f) {
                value += 1;
                totalAdvanced = 0.0f;
            }
            progress->setValue(value);
            progress->setLabelText(exportingFormat.arg(value).arg(maxRangeIndex));
            m_sceneWidget->advanceMotion(advanceSecond);
            m_sceneWidget->resize(videoSize);
            totalAdvanced += advanceSecond;
        }
        /* 最後のフレームを書き出し */
        const QImage &image = m_sceneWidget->grabFrameBuffer();
        if (image.width() != width || image.height() != height)
            emit sceneDidRendered(image.scaled(width, height));
        else
            emit sceneDidRendered(image);
        /* エンコードを終了させるための空のフレーム */
        emit sceneDidRendered(QImage());
        /* エンコードが完了するまで待機 */
        const QString &encodingFormat = tr("Encoding remain frame %1 of %2...");
        int remain = m_videoEncoder->sizeofVideoFrameQueue();
        progress->setValue(0);
        progress->setMaximum(remain);
        progress->setLabelText(encodingFormat.arg(0).arg(remain));
        int size = 0;
        /* 残りフレームをエンコード */
        while (remain > size) {
            if (progress->wasCanceled())
                break;
            size = remain - m_videoEncoder->sizeofVideoFrameQueue();
            progress->setValue(size);
            progress->setLabelText(encodingFormat.arg(size).arg(remain));
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        m_audioDecoder->stopSession();
        m_videoEncoder->stopSession();
        restoreWindowState(state);
    }
}

void MainWindow::saveWindowStateAndResize(const QSize &videoSize, WindowState &state)
{
    SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
    /* 画面を復元するために一時的に情報を保持。mainGeometry はコピーを持たないといけないので参照であってはならない */
    state.mainGeometry = geometry();
    state.minSize = minimumSize();
    state.maxSize = maximumSize();
    state.scenesize = m_sceneWidget->size();
    state.policy = sizePolicy();
    state.timeIndex = m_sceneWidget->currentTimeIndex();
    state.preferredFPS = loader->sceneRef()->preferredFPS();
    state.isGridVisible = loader->isGridVisible();
    m_mainToolBar->hide();
    m_timelineDockWidget->hide();
    m_sceneDockWidget->hide();
    m_modelDockWidget->hide();
    statusBar()->hide();
    /* 動画書き出し用の設定に変更 */
    resize(videoSize);
    setMinimumSize(videoSize);
    setMaximumSize(videoSize);
    adjustSize();
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    /* レンダリングモードを自動更新から手動更新に変更 */
    m_sceneWidget->stopAutomaticRendering();
    /* 画像出力時は物理状態とモーションのリセットを行わない */
    if (!state.isImage) {
        m_sceneWidget->stop();
        loader->startPhysicsSimulation();
    }
    loader->setGridVisible(m_exportingVideoDialog->includesGrid());
    /* ハンドルと情報パネルを非表示にし、ウィンドウを指定されたサイズに変更する */
    m_sceneWidget->setHandlesVisible(false);
    m_sceneWidget->setInfoPanelVisible(false);
    m_sceneWidget->setBoneWireFramesVisible(false);
    m_sceneWidget->resize(videoSize);
}

void MainWindow::restoreWindowState(const WindowState &state)
{
    SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
    /* 画面情報を復元 */
    loader->setGridVisible(state.isGridVisible);
    setSizePolicy(state.policy);
    setMinimumSize(state.minSize);
    setMaximumSize(state.maxSize);
    setGeometry(state.mainGeometry);
    statusBar()->show();
    /* ドック復元 */
    m_timelineDockWidget->show();
    m_sceneDockWidget->show();
    m_modelDockWidget->show();
    m_mainToolBar->show();
    /* 情報パネルとハンドルを再表示 */
    m_sceneWidget->resize(state.scenesize);
    m_sceneWidget->setHandlesVisible(true);
    m_sceneWidget->setInfoPanelVisible(true);
    m_sceneWidget->setBoneWireFramesVisible(true);
    m_sceneWidget->setPreferredFPS(state.preferredFPS);
    /* 画像出力以外は物理状態の停止とモーションの位置再開を行う */
    if (!state.isImage) {
        loader->stopPhysicsSimulation();
        m_sceneWidget->seekMotion(state.timeIndex, true, true);
    }
    /* レンダリングを手動更新から自動更新に戻す */
    m_sceneWidget->startAutomaticRendering();
}

void MainWindow::addNewMotion()
{
    if (maybeSaveMotion()) {
        IModel *model = m_sceneWidget->sceneLoaderRef()->selectedModelRef();
        IMotion *motion = m_boneMotionModel->currentMotionRef();
        if (model && motion) {
            m_boneMotionModel->removeMotion();
            m_morphMotionModel->removeMotion();
            m_sceneMotionModel->removeMotion();
            m_sceneWidget->setEmptyMotion(model, false);
            m_boneMotionModel->markAsNew(model);
            m_morphMotionModel->markAsNew(model);
            m_sceneMotionModel->setModified(false);
        }
    }
}

void MainWindow::invokePlayer()
{
    if (m_sceneWidget->sceneLoaderRef()->sceneRef()->maxFrameIndex() > 0) {
        UICreatePlaySettingDialog(this, &m_settings, m_sceneWidget, m_playSettingDialog);
        UICreateScenePlayer(this, m_sceneWidget, m_playSettingDialog, m_timelineTabWidget, m_player);
        /*
         * 再生中はボーンが全選択になるのでワイヤーフレーム表示のオプションの関係からシグナルを一時的に解除する。
         * 停止後に makeBonesSelectable 経由でシグナルを復活させる
         */
        disconnect(m_boneMotionModel.data(), SIGNAL(bonesDidSelect(QList<IBone*>)), m_sceneWidget.data(), SLOT(selectBones(QList<IBone*>)));
        m_player->start();
    }
    else {
        warning(this, tr("No motion to export."),
                tr("You must create or load a motion to play."));
    }
}

void MainWindow::openPlaySettingDialog()
{
    if (m_sceneWidget->sceneLoaderRef()->sceneRef()->maxFrameIndex() > 0) {
        UICreatePlaySettingDialog(this, &m_settings, m_sceneWidget, m_playSettingDialog);
        UICreateScenePlayer(this, m_sceneWidget, m_playSettingDialog, m_timelineTabWidget, m_player);
        m_playSettingDialog->show();
    }
    else {
        warning(this, tr("No motion to export."),
                tr("You must create or load a motion to open play setting."));
    }
}

void MainWindow::selectNextModel()
{
    const QList<QAction *> &actions = m_menuRetainModels->actions();
    if (!actions.isEmpty()) {
        const SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
        int index = UIFindIndexOfActions(m_sceneWidget->sceneLoaderRef()->selectedModelRef(), actions);
        IModel *model = 0;
        if (index == -1 || index == actions.length() - 1) {
            model = loader->findModel(actions.first()->text());
        }
        else {
            model = loader->findModel(actions.at(index + 1)->text());
        }
        m_sceneWidget->setSelectedModel(model, SceneWidget::kSelect);
    }
}

void MainWindow::selectPreviousModel()
{
    const QList<QAction *> &actions = m_menuRetainModels->actions();
    if (!actions.isEmpty()) {
        const SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
        int index = UIFindIndexOfActions(m_sceneWidget->sceneLoaderRef()->selectedModelRef(), actions);
        IModel *model = 0;
        if (index == -1 || index == 0) {
            model = loader->findModel(actions.last()->text());
        }
        else {
            model = loader->findModel(actions.at(index - 1)->text());
        }
        m_sceneWidget->setSelectedModel(model, SceneWidget::kSelect);
    }
}

void MainWindow::showLicenseWidget()
{
    if (!m_licenseWidget) {
        m_licenseWidget.reset(new LicenseWidget());
        m_licenseWidget->show();
    }
}

void MainWindow::openGravitySettingDialog()
{
    QScopedPointer<GravitySettingDialog> dialog(new GravitySettingDialog(m_sceneWidget->sceneLoaderRef()));
    dialog->exec();
}

void MainWindow::openRenderOrderDialog()
{
    QScopedPointer<RenderOrderDialog> dialog(new RenderOrderDialog(m_sceneWidget->sceneLoaderRef()));
    dialog->exec();
}

void MainWindow::openScreenColorDialog()
{
    SceneLoader *loader = m_sceneWidget->sceneLoaderRef();
    const QColor &before = loader->screenColor();
    QScopedPointer<QColorDialog> dialog(new QColorDialog(before));
    connect(dialog.data(), SIGNAL(currentColorChanged(QColor)), loader, SLOT(setScreenColor(QColor)));
    if (dialog->exec() == QColorDialog::Rejected)
        loader->setScreenColor(before);
}

void MainWindow::openShadowMapDialog()
{
    QScopedPointer<ShadowMapSettingDialog> dialog(new ShadowMapSettingDialog(m_sceneWidget->sceneLoaderRef()));
    dialog->exec();
}

void MainWindow::openBackgroundImageDialog()
{
    QScopedPointer<BackgroundImageSettingDialog> dialog(new BackgroundImageSettingDialog(m_sceneWidget->sceneLoaderRef()));
    connect(dialog.data(), SIGNAL(positionDidChange(QPoint)), m_sceneWidget.data(), SLOT(setBackgroundPosition(QPoint)));
    connect(dialog.data(), SIGNAL(uniformDidEnable(bool)), m_sceneWidget.data(), SLOT(setBackgroundImageUniformEnable(bool)));
    dialog->exec();
}

void MainWindow::openUndoView()
{
    QScopedPointer<QWidget> widget(new QWidget());
    QScopedPointer<QUndoView> undoView(new QUndoView(m_undo.data()));
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    mainLayout->addWidget(undoView.data());
    QScopedPointer<QDialogButtonBox> buttons(new QDialogButtonBox(QDialogButtonBox::Ok));
    connect(buttons.data(), SIGNAL(accepted()), widget.data(), SLOT(close()));
    mainLayout->addWidget(buttons.data());
    widget->setLayout(mainLayout.data());
    widget->setWindowTitle(tr("Undo history"));
    widget->show();
}

void MainWindow::updateWindowTitle()
{
    const QString &initial = "[*]";
    QString value = initial;
    if (!m_currentMotionFilename.isEmpty())
        value += QFileInfo(m_currentMotionFilename).fileName();
    if (!m_currentProjectFilename.isEmpty()) {
        if (initial != value)
            value += " - ";
        value += QFileInfo(m_currentProjectFilename).fileName();
    }
    if (initial == value)
        value += qAppName();
    else
        value += " - " + qAppName();
    setWindowTitle(value);
}

void MainWindow::enableSelectingBonesAndMorphs()
{
    connect(m_boneMotionModel.data(), SIGNAL(bonesDidSelect(QList<IBone*>)), m_sceneWidget.data(), SLOT(selectBones(QList<IBone*>)));
    connect(m_morphMotionModel.data(), SIGNAL(morphsDidSelect(QList<IMorph*>)), m_sceneWidget.data(), SLOT(selectMorphs(QList<IMorph*>)));
}

void MainWindow::disconnectInitialSlots()
{
    /* モデルを読み込んだ後の初期化順序の関係でカメラモーションを読み込んだ後にプロジェクトを無更新に設定する処理を外す */
    disconnect(m_sceneMotionModel.data(), SIGNAL(cameraMotionDidLoad()), m_sceneWidget->sceneLoaderRef(), SLOT(setProjectDirtyFalse()));
    disconnect(m_sceneMotionModel.data(), SIGNAL(cameraMotionDidLoad()), m_sceneMotionModel.data(), SLOT(markAsNew()));
    disconnect(m_sceneMotionModel.data(), SIGNAL(cameraMotionDidLoad()), this, SLOT(disconnectInitialSlots()));
}

void MainWindow::resetSceneToModels()
{
    const Scene *scene = m_sceneWidget->sceneLoaderRef()->sceneRef();
    m_boneMotionModel->setSceneRef(scene);
    m_morphMotionModel->setSceneRef(scene);
}

} /* namespace vpvm */

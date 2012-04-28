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
#include "dialogs/BoneDialog.h"
#include "dialogs/ExportVideoDialog.h"
#include "dialogs/GravitySettingDialog.h"
#include "dialogs/FrameSelectionDialog.h"
#include "dialogs/PlaySettingDialog.h"
#include "dialogs/RenderOrderDialog.h"
#include "models/BoneMotionModel.h"
#include "models/MorphMotionModel.h"
#include "models/SceneMotionModel.h"
#include "video/AudioDecoder.h"
#include "video/VideoEncoder.h"
#include "widgets/AssetWidget.h"
#include "widgets/CameraPerspectiveWidget.h"
#include "widgets/MorphWidget.h"
#include "widgets/InterpolationWidget.h"
#include "widgets/LicenseWidget.h"
#include "widgets/ModelInfoWidget.h"
#include "widgets/ModelSettingWidget.h"
#include "widgets/ModelTabWidget.h"
#include "widgets/SceneLightWidget.h"
#include "widgets/TabWidget.h"
#include "widgets/TimelineTabWidget.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

namespace {

static int UIFindIndexOfActions(IModel *model, const QList<QAction *> &actions)
{
    const QString &name = internal::toQString(model);
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
                                             SceneWidget *sceneWidget,
                                             PlaySettingDialog *&dialog)
{
    if (!dialog) {
        dialog = new PlaySettingDialog(sceneWidget->sceneLoader(), settings, mainWindow);
        QObject::connect(dialog, SIGNAL(playingDidStart()), mainWindow, SLOT(invokePlayer()));
    }
}

static inline void UICreateScenePlayer(MainWindow *mainWindow,
                                       SceneWidget *sceneWidget,
                                       PlaySettingDialog *dialog,
                                       TimelineTabWidget *timeline,
                                       ScenePlayer *&player)
{
    if (!player) {
        player = new ScenePlayer(sceneWidget, dialog);
        QObject::connect(dialog, SIGNAL(playingDidStart()), dialog, SLOT(hide()));
        QObject::connect(dialog, SIGNAL(playingDidStart()), player, SLOT(setRestoreState()));
        QObject::connect(player, SIGNAL(motionDidSeek(int)), timeline, SLOT(setCurrentFrameIndex(int)));
        QObject::connect(player, SIGNAL(renderFrameDidStop()), mainWindow, SLOT(makeBonesSelectable()));
        QObject::connect(player, SIGNAL(renderFrameDidStopAndRestoreState()), dialog, SLOT(show()));
    }
}

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_encoding(0),
    m_factory(0),
    m_settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qAppName()),
    m_undo(0),
    m_licenseWidget(0),
    m_loggerWidget(0),
    m_sceneWidget(0),
    m_sceneTabWidget(0),
    m_modelTabWidget(0),
    m_timelineTabWidget(0),
    m_boneMotionModel(0),
    m_morphMotionModel(0),
    m_sceneMotionModel(0),
    m_exportingVideoDialog(0),
    m_playSettingDialog(0),
    m_boneUIDelegate(0),
    m_audioDecoder(0),
    m_videoEncoder(0),
    m_player(0),
    m_model(0),
    m_bone(0),
    m_position(0.0f, 0.0f, 0.0f),
    m_angle(0.0f, 0.0f, 0.0f),
    m_fovy(0.0f),
    m_distance(0.0f),
    m_currentFPS(-1)
{
    m_encoding = new internal::Encoding();
    m_factory = new vpvl2::Factory(m_encoding);
    m_undo = new QUndoGroup(this);
    m_sceneWidget = new SceneWidget(m_encoding, m_factory, &m_settings);
    /* SceneWidget で渡しているのは Scene が initializeGL で初期化されるという特性のため */
    m_boneMotionModel = new BoneMotionModel(m_factory, m_undo, m_sceneWidget, this);
    m_morphMotionModel = new MorphMotionModel(m_factory, m_undo, this);
    m_sceneMotionModel = new SceneMotionModel(m_factory, m_undo, m_sceneWidget, this);
    m_sceneTabWidget = new TabWidget(&m_settings);
    m_modelTabWidget = new ModelTabWidget(&m_settings, m_morphMotionModel, this);
    m_timelineTabWidget = new TimelineTabWidget(&m_settings, m_boneMotionModel, m_morphMotionModel, m_sceneMotionModel);
    m_boneUIDelegate = new BoneUIDelegate(m_boneMotionModel, this);
    m_loggerWidget = LoggerWidget::createInstance(&m_settings);
    buildUI();
    connectWidgets();
    restoreGeometry(m_settings.value("mainWindow/geometry").toByteArray());
    restoreState(m_settings.value("mainWindow/state").toByteArray());
    updateWindowTitle();
    statusBar()->show();
    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
    delete m_undo;
    delete m_licenseWidget;
    delete m_sceneWidget;
    delete m_boneMotionModel;
    delete m_morphMotionModel;
    delete m_sceneMotionModel;
    delete m_sceneTabWidget;
    delete m_modelTabWidget;
    delete m_timelineTabWidget;
    delete m_boneUIDelegate;
    delete m_videoEncoder;
    delete m_player;
    delete m_menuBar;
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
        qApp->sendEvent(m_sceneWidget, event);
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
        menu.addAction(m_actionAddModel);
        menu.addAction(m_actionAddAsset);
        menu.addAction(m_actionInsertToSelectedModel);
        menu.addSeparator();
        menu.addMenu(m_menuRetainModels);
        menu.addSeparator();
        menu.addAction(m_actionShowTimelineDock);
        menu.addAction(m_actionShowSceneDock);
        menu.addAction(m_actionShowModelDock);
        menu.exec(pos);
    }
    else if (m_timelineTabWidget->rect().contains(m_timelineTabWidget->mapFromGlobal(pos))) {
        QMenu menu(this);
        menu.addAction(m_actionRegisterFrame);
        menu.addAction(m_actionSelectAllFrames);
        menu.addAction(m_actionSelectFrameDialog);
        menu.addSeparator();
        menu.addAction(m_actionCut);
        menu.addAction(m_actionCopy);
        menu.addAction(m_actionPaste);
        menu.addAction(m_actionReversedPaste);
        menu.addSeparator();
        menu.addAction(m_actionUndoFrame);
        menu.addAction(m_actionRedoFrame);
        menu.addSeparator();
        menu.addAction(m_actionDeleteSelectedFrame);
        menu.exec(pos);
    }
}

void MainWindow::selectModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        const QUuid uuid(action->data().toString());
        m_sceneWidget->setSelectedModel(m_sceneWidget->sceneLoader()->findModel(uuid));
    }
}

void MainWindow::setCurrentModel(IModel *model)
{
    m_model = model;
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
        updateWindowTitle();
    }
}

void MainWindow::loadProject()
{
    if (maybeSaveProject()) {
        const QString &filename = internal::openFileDialog("mainWindow/lastProjectDirectory",
                                                           tr("Open VPVM file"),
                                                           tr("VPVM file (*.xml)"),
                                                           &m_settings);
        if (!filename.isEmpty()) {
            m_boneMotionModel->removeMotion();
            m_morphMotionModel->removeMotion();
            m_sceneMotionModel->removeMotion();
            m_sceneWidget->loadProject(filename);
            SceneLoader *loader = m_sceneWidget->sceneLoader();
            m_actionEnableAcceleration->setChecked(loader->isAccelerationEnabled());
            m_actionEnablePhysics->setChecked(loader->isPhysicsEnabled());
            m_actionShowGrid->setChecked(loader->isGridVisible());
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
    m_sceneWidget->setSelectedModel(0);
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
    QString name = internal::toQString(model);
    QAction *action = new QAction(name, this);
    action->setData(uuid.toString());
    action->setStatusTip(tr("Select a model %1").arg(name));
    connect(action, SIGNAL(triggered()), SLOT(selectModel()));
    m_menuRetainModels->addAction(action);
    m_sceneWidget->setSelectedModel(model);
}

void MainWindow::deleteModel(IModel *model, const QUuid &uuid)
{
    /* 削除されるモデルが選択中のモデルと同じなら選択状態を解除しておく(残すと不正アクセスの原因になるので) */
    if (model == m_sceneWidget->sceneLoader()->selectedModel())
        m_sceneWidget->setSelectedModel(0);
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
    QString name = internal::toQString(asset);
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
    filename = internal::openSaveDialog("mainWindow/lastModelMotionDirectory",
                                        tr("Save model motion as a VMD file"),
                                        tr("VMD file (*.vmd)"),
                                        tr("untitiled_model_motion.vmd"),
                                        &m_settings);
    return !filename.isEmpty() ? saveMotionFile(filename) : false;
}

void MainWindow::saveCameraMotionAs()
{
    const QString &filename = internal::openSaveDialog("mainWindow/lastCameraMotionDirectory",
                                                       tr("Save camera motion as a VMD file"),
                                                       tr("VMD file (*.vmd)"),
                                                       tr("untitiled_camera_motion.vmd"),
                                                       &m_settings);
    QScopedPointer<IMotion> motion(m_factory->createMotion());
    IMotion *motionPtr = motion.data();
    m_sceneMotionModel->saveMotion(motionPtr);
    saveMotionFile(filename, motionPtr);
}

bool MainWindow::saveMotionFile(const QString &filename)
{
    /* 全てのボーンフレーム、頂点モーフフレーム、カメラフレームをファイルとして書き出しを行う */
    QScopedPointer<IMotion> motion(m_factory->createMotion());
    IMotion *motionPtr = motion.data();
    m_boneMotionModel->saveMotion(motionPtr);
    m_morphMotionModel->saveMotion(motionPtr);
    return saveMotionFile(filename, motionPtr);
}

bool MainWindow::saveMotionFile(const QString &filename, IMotion *motion)
{
    typedef QScopedPointer<uint8_t, QScopedPointerArrayDeleter<uint8_t> > ByteArrayPtr;
    size_t size = motion->estimateSize();
    ByteArrayPtr buffer(new uint8_t[size]);
    motion->save(buffer.data());
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
    filename = internal::openSaveDialog("mainWindow/lastProjectDirectory",
                                        tr("Save projct as a VPVM project file"),
                                        tr("VPVM project file (*.xml)"),
                                        tr("untitled.xml"),
                                        &m_settings);
    return !filename.isEmpty() ? saveProjectFile(filename) : false;
}

bool MainWindow::saveProjectFile(const QString &filename)
{
    SceneLoader *loader = m_sceneWidget->sceneLoader();
    loader->setAccelerationEnabled(m_actionEnableAcceleration->isChecked());
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
            || m_sceneWidget->sceneLoader()->isProjectModified();
    if (confirmSave(cond, cancel))
        saveProject();
    return !cancel;
}

bool MainWindow::confirmSave(bool condition, bool &cancel)
{
    cancel = false;
    if (condition) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this,
                                   qAppName(),
                                   tr("Do you want to save your changes?"),
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

void MainWindow::buildUI()
{
    m_timelineDockWidget = new QDockWidget(this);
    m_timelineDockWidget->setWidget(m_timelineTabWidget);
    m_timelineDockWidget->restoreGeometry(m_settings.value("mainWindow/timelineDockWidgetGeometry").toByteArray());
    addDockWidget(Qt::LeftDockWidgetArea, m_timelineDockWidget);
    m_sceneDockWidget = new QDockWidget(this);
    m_sceneDockWidget->setWidget(m_sceneTabWidget);
    m_sceneDockWidget->restoreGeometry(m_settings.value("mainWindow/sceneDockWidgetGeometry").toByteArray());
    addDockWidget(Qt::LeftDockWidgetArea, m_sceneDockWidget);
    m_modelDockWidget = new QDockWidget(this);
    m_modelDockWidget->setWidget(m_modelTabWidget);
    m_modelDockWidget->restoreGeometry(m_settings.value("mainWindow/modelDockWidgetGeometry").toByteArray());
    addDockWidget(Qt::LeftDockWidgetArea, m_modelDockWidget);
    tabifyDockWidget(m_timelineDockWidget, m_sceneDockWidget);
    tabifyDockWidget(m_sceneDockWidget, m_modelDockWidget);

    m_actionNewProject = new QAction(this);
    connect(m_actionNewProject, SIGNAL(triggered()), SLOT(newProjectFile()));
    m_actionNewMotion = new QAction(this);
    connect(m_actionNewMotion, SIGNAL(triggered()), SLOT(addNewMotion()));
    m_actionLoadProject = new QAction(this);
    connect(m_actionLoadProject, SIGNAL(triggered()), SLOT(loadProject()));
    m_actionAddModel = new QAction(this);
    connect(m_actionAddModel, SIGNAL(triggered()), m_sceneWidget, SLOT(addModel()));
    m_actionAddAsset = new QAction(this);
    connect(m_actionAddAsset, SIGNAL(triggered()), m_sceneWidget, SLOT(addAsset()));
    m_actionInsertToAllModels = new QAction(this);
    connect(m_actionInsertToAllModels, SIGNAL(triggered()), m_sceneWidget, SLOT(insertMotionToAllModels()));
    m_actionInsertToSelectedModel = new QAction(this);
    connect(m_actionInsertToSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(insertMotionToSelectedModel()));
    m_actionSetCamera = new QAction(this);
    connect(m_actionSetCamera, SIGNAL(triggered()), m_sceneWidget, SLOT(setCamera()));
    m_actionLoadModelPose = new QAction(this);
    connect(m_actionLoadModelPose, SIGNAL(triggered()), m_sceneWidget, SLOT(insertPoseToSelectedModel()));
    m_actionSaveModelPose = new QAction(this);
    connect(m_actionSaveModelPose, SIGNAL(triggered()), SLOT(saveModelPose()));
    m_actionLoadAssetMetadata = new QAction(this);
    connect(m_actionLoadAssetMetadata, SIGNAL(triggered()), m_sceneWidget, SLOT(addAssetFromMetadata()));
    m_actionSaveAssetMetadata = new QAction(this);
    connect(m_actionSaveAssetMetadata, SIGNAL(triggered()), SLOT(saveAssetMetadata()));
    m_actionSaveProject = new QAction(this);
    connect(m_actionSaveProject, SIGNAL(triggered()), SLOT(saveProject()));
    m_actionSaveProjectAs = new QAction(this);
    connect(m_actionSaveProjectAs, SIGNAL(triggered()), SLOT(saveProjectAs()));
    m_actionSaveMotion = new QAction(this);
    connect(m_actionSaveMotion, SIGNAL(triggered()), SLOT(saveMotion()));
    m_actionSaveMotionAs = new QAction(this);
    connect(m_actionSaveMotionAs, SIGNAL(triggered()), SLOT(saveMotionAs()));
    m_actionSaveCameraMotionAs = new QAction(this);
    connect(m_actionSaveCameraMotionAs, SIGNAL(triggered()), SLOT(saveCameraMotionAs()));
    m_actionExportImage = new QAction(this);
    connect(m_actionExportImage, SIGNAL(triggered()), SLOT(exportImage()));
    m_actionExportVideo = new QAction(this);
    connect(m_actionExportVideo, SIGNAL(triggered()), SLOT(exportVideo()));
    m_actionExit = new QAction(this);
    m_actionExit->setMenuRole(QAction::QuitRole);
    connect(m_actionExit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    m_actionRegisterFrame = new QAction(this);
    connect(m_actionRegisterFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(addKeyFramesFromSelectedIndices()));
    m_actionInsertEmptyFrame = new QAction(this);
    connect(m_actionInsertEmptyFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(insertFrame()));
    m_actionDeleteSelectedFrame = new QAction(this);
    connect(m_actionDeleteSelectedFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(deleteFrame()));
    m_actionCut = new QAction(this);
    connect(m_actionCut, SIGNAL(triggered()), m_timelineTabWidget, SLOT(cutKeyframes()));
    m_actionCopy = new QAction(this);
    connect(m_actionCopy, SIGNAL(triggered()), m_timelineTabWidget, SLOT(copyKeyframes()));
    m_actionPaste = new QAction(this);
    connect(m_actionPaste, SIGNAL(triggered()), m_timelineTabWidget, SLOT(pasteKeyframes()));
    m_actionReversedPaste = new QAction(this);
    connect(m_actionReversedPaste, SIGNAL(triggered()), m_timelineTabWidget, SLOT(pasteKeyframesWithReverse()));
    m_actionUndoFrame = m_undo->createUndoAction(this);
    m_actionRedoFrame = m_undo->createRedoAction(this);
    m_actionBoneXPosZero = new QAction(this);
    connect(m_actionBoneXPosZero, SIGNAL(triggered()), m_boneUIDelegate, SLOT(resetBoneX()));
    m_actionBoneYPosZero = new QAction(this);
    connect(m_actionBoneYPosZero, SIGNAL(triggered()), m_boneUIDelegate, SLOT(resetBoneY()));
    m_actionBoneZPosZero = new QAction(this);
    connect(m_actionBoneZPosZero, SIGNAL(triggered()), m_boneUIDelegate, SLOT(resetBoneZ()));
    m_actionBoneRotationZero = new QAction(this);
    connect(m_actionBoneRotationZero, SIGNAL(triggered()), m_boneUIDelegate, SLOT(resetBoneRotation()));
    m_actionBoneResetAll = new QAction(this);
    connect(m_actionBoneResetAll, SIGNAL(triggered()), m_boneUIDelegate, SLOT(resetAllBones()));
    m_actionBoneDialog = new QAction(this);
    connect(m_actionBoneDialog, SIGNAL(triggered()), m_boneUIDelegate, SLOT(openBoneDialog()));
    m_actionDeleteSelectedModel = new QAction(this);
    connect(m_actionDeleteSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(deleteSelectedModel()));

    m_actionPlay = new QAction(this);
    connect(m_actionPlay, SIGNAL(triggered()), SLOT(invokePlayer()));
    m_actionPlaySettings = new QAction(this);
    connect(m_actionPlaySettings, SIGNAL(triggered()), SLOT(openPlaySettingDialog()));
    m_actionOpenGravitySettingsDialog = new QAction(this);
    connect(m_actionOpenGravitySettingsDialog, SIGNAL(triggered()), SLOT(openGravitySettingDialog()));
    m_actionOpenRenderOrderDialog = new QAction(this);
    connect(m_actionOpenRenderOrderDialog, SIGNAL(triggered()), SLOT(openRenderOrderDialog()));
    m_actionOpenScreenColorDialog = new QAction(this);
    connect(m_actionOpenScreenColorDialog, SIGNAL(triggered()), SLOT(openScreenColorDialog()));
    m_actionEnableAcceleration = new QAction(this);
    m_actionEnableAcceleration->setCheckable(true);
    m_actionEnableAcceleration->setEnabled(Scene::isAcceleratorSupported());
    m_actionEnablePhysics = new QAction(this);
    m_actionEnablePhysics->setCheckable(true);
    m_actionEnablePhysics->setChecked(true);
    m_actionShowGrid = new QAction(this);
    m_actionShowGrid->setCheckable(true);
    m_actionShowGrid->setChecked(true);

    m_actionZoomIn = new QAction(this);
    connect(m_actionZoomIn, SIGNAL(triggered()), m_sceneWidget, SLOT(zoomIn()));
    m_actionZoomOut = new QAction(this);
    connect(m_actionZoomOut, SIGNAL(triggered()), m_sceneWidget, SLOT(zoomOut()));
    m_actionRotateUp = new QAction(this);
    connect(m_actionRotateUp, SIGNAL(triggered()), m_sceneWidget, SLOT(rotateUp()));
    m_actionRotateDown = new QAction(this);
    connect(m_actionRotateDown, SIGNAL(triggered()), m_sceneWidget, SLOT(rotateDown()));
    m_actionRotateLeft = new QAction(this);
    connect(m_actionRotateLeft, SIGNAL(triggered()), m_sceneWidget, SLOT(rotateLeft()));
    m_actionRotateRight = new QAction(this);
    connect(m_actionRotateRight, SIGNAL(triggered()), m_sceneWidget, SLOT(rotateRight()));
    m_actionTranslateUp = new QAction(this);
    connect(m_actionTranslateUp, SIGNAL(triggered()), m_sceneWidget, SLOT(translateUp()));
    m_actionTranslateDown = new QAction(this);
    connect(m_actionTranslateDown, SIGNAL(triggered()), m_sceneWidget, SLOT(translateDown()));
    m_actionTranslateLeft = new QAction(this);
    connect(m_actionTranslateLeft, SIGNAL(triggered()), m_sceneWidget, SLOT(translateLeft()));
    m_actionTranslateRight = new QAction(this);
    connect(m_actionTranslateRight, SIGNAL(triggered()), m_sceneWidget, SLOT(translateRight()));
    m_actionResetCamera = new QAction(this);
    connect(m_actionResetCamera, SIGNAL(triggered()), m_sceneWidget, SLOT(resetCamera()));

    m_actionSelectNextModel = new QAction(this);
    connect(m_actionSelectNextModel, SIGNAL(triggered()), SLOT(selectNextModel()));
    m_actionSelectPreviousModel = new QAction(this);
    connect(m_actionSelectPreviousModel, SIGNAL(triggered()), SLOT(selectPreviousModel()));
    m_actionRevertSelectedModel = new QAction(this);
    connect(m_actionRevertSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(revertSelectedModel()));
    m_actionTranslateModelUp = new QAction(this);
    connect(m_actionTranslateModelUp, SIGNAL(triggered()), m_sceneWidget, SLOT(translateModelUp()));
    m_actionTranslateModelDown = new QAction(this);
    connect(m_actionTranslateModelDown, SIGNAL(triggered()), m_sceneWidget, SLOT(translateModelDown()));
    m_actionTranslateModelLeft = new QAction(this);
    connect(m_actionTranslateModelLeft, SIGNAL(triggered()), m_sceneWidget, SLOT(translateModelLeft()));
    m_actionTranslateModelRight = new QAction(this);
    connect(m_actionTranslateModelRight, SIGNAL(triggered()), m_sceneWidget, SLOT(translateModelRight()));
    m_actionResetModelPosition = new QAction(this);
    connect(m_actionResetModelPosition, SIGNAL(triggered()), m_sceneWidget, SLOT(resetModelPosition()));

    m_actionSelectAllFrames = new QAction(this);
    connect(m_actionSelectAllFrames, SIGNAL(triggered()), m_timelineTabWidget, SLOT(selectAllRegisteredKeyframes()));
    m_actionSelectFrameDialog = new QAction(this);
    connect(m_actionSelectFrameDialog, SIGNAL(triggered()), m_timelineTabWidget, SLOT(openFrameSelectionDialog()));
    m_actionFrameWeightDialog = new QAction(this);
    connect(m_actionFrameWeightDialog, SIGNAL(triggered()), m_timelineTabWidget, SLOT(openFrameWeightDialog()));
    m_actionNextFrame = new QAction(this);
    connect(m_actionNextFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(nextFrame()));
    m_actionPreviousFrame = new QAction(this);
    connect(m_actionPreviousFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(previousFrame()));

    m_actionViewLogMessage = new QAction(this);
    connect(m_actionViewLogMessage, SIGNAL(triggered()), m_loggerWidget, SLOT(show()));
    m_actionEnableMoveGesture = new QAction(this);
    m_actionEnableMoveGesture->setCheckable(true);
    m_actionEnableMoveGesture->setChecked(m_sceneWidget->isMoveGestureEnabled());
    connect(m_actionEnableMoveGesture, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setMoveGestureEnable(bool)));
    m_actionEnableRotateGesture = new QAction(this);
    m_actionEnableRotateGesture->setCheckable(true);
    m_actionEnableRotateGesture->setChecked(m_sceneWidget->isRotateGestureEnabled());
    connect(m_actionEnableRotateGesture, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setRotateGestureEnable(bool)));
    m_actionEnableScaleGesture = new QAction(this);
    m_actionEnableScaleGesture->setCheckable(true);
    m_actionEnableScaleGesture->setChecked(m_sceneWidget->isRotateGestureEnabled());
    connect(m_actionEnableScaleGesture, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setScaleGestureEnable(bool)));
    m_actionEnableUndoGesture = new QAction(this);
    m_actionEnableUndoGesture->setCheckable(true);
    m_actionEnableUndoGesture->setChecked(m_sceneWidget->isUndoGestureEnabled());
    connect(m_actionEnableUndoGesture, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setUndoGestureEnable(bool)));
    m_actionShowTimelineDock = new QAction(this);
    connect(m_actionShowTimelineDock, SIGNAL(triggered()), m_timelineDockWidget, SLOT(show()));
    m_actionShowSceneDock = new QAction(this);
    connect(m_actionShowSceneDock, SIGNAL(triggered()), m_sceneDockWidget, SLOT(show()));
    m_actionShowModelDock = new QAction(this);
    connect(m_actionShowModelDock, SIGNAL(triggered()), m_modelDockWidget, SLOT(show()));
    m_actionShowModelDialog = new QAction(this);
    m_actionShowModelDialog->setCheckable(true);
    m_actionShowModelDialog->setChecked(m_sceneWidget->showModelDialog());
    connect(m_actionShowModelDialog, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setShowModelDialog(bool)));

    m_actionClearRecentFiles = new QAction(this);
    connect(m_actionClearRecentFiles, SIGNAL(triggered()), SLOT(clearRecentFiles()));
    m_actionAbout = new QAction(this);
    connect(m_actionAbout, SIGNAL(triggered()), SLOT(showLicenseWidget()));
    m_actionAbout->setMenuRole(QAction::AboutRole);
    m_actionAboutQt = new QAction(this);
    connect(m_actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    m_actionAboutQt->setMenuRole(QAction::AboutQtRole);

#ifdef Q_OS_MACX
    m_menuBar = new QMenuBar(0);
#else
    m_menuBar = menuBar();
#endif
    m_menuFile = new QMenu(this);
    m_menuFile->addAction(m_actionNewProject);
    m_menuFile->addAction(m_actionNewMotion);
    m_menuFile->addAction(m_actionLoadProject);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionAddModel);
    m_menuFile->addAction(m_actionAddAsset);
    m_menuFile->addAction(m_actionInsertToAllModels);
    m_menuFile->addAction(m_actionInsertToSelectedModel);
    m_menuFile->addAction(m_actionSetCamera);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionSaveProject);
    m_menuFile->addAction(m_actionSaveProjectAs);
    m_menuFile->addAction(m_actionSaveMotion);
    m_menuFile->addAction(m_actionSaveMotionAs);
    m_menuFile->addAction(m_actionSaveCameraMotionAs);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionLoadModelPose);
    m_menuFile->addAction(m_actionSaveModelPose);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionLoadAssetMetadata);
    m_menuFile->addAction(m_actionSaveAssetMetadata);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionExportImage);
    m_menuFile->addAction(m_actionExportVideo);
    m_menuRecentFiles = new QMenu(this);
    for (int i = 0; i < kMaxRecentFiles; i++) {
        QAction *action = new QAction(this);
        connect(action, SIGNAL(triggered()), SLOT(openRecentFile()));
        action->setVisible(false);
        m_actionRecentFiles[i] = action;
        m_menuRecentFiles->addAction(action);
    }
    m_menuRecentFiles->addSeparator();
    m_menuRecentFiles->addAction(m_actionClearRecentFiles);
    m_menuFile->addMenu(m_menuRecentFiles);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionExit);
    m_menuBar->addMenu(m_menuFile);
    m_menuEdit = new QMenu(this);
    m_menuEdit->addAction(m_actionUndoFrame);
    m_menuEdit->addAction(m_actionRedoFrame);
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(m_actionCut);
    m_menuEdit->addAction(m_actionCopy);
    m_menuEdit->addAction(m_actionPaste);
    m_menuEdit->addAction(m_actionReversedPaste);
    m_menuBar->addMenu(m_menuEdit);
    m_menuProject = new QMenu(this);
    m_menuProject->addAction(m_actionPlay);
    m_menuProject->addAction(m_actionPlaySettings);
    m_menuProject->addSeparator();
    m_menuProject->addAction(m_actionOpenGravitySettingsDialog);
    m_menuProject->addAction(m_actionOpenRenderOrderDialog);
    m_menuProject->addAction(m_actionOpenScreenColorDialog);
    m_menuProject->addSeparator();
    m_menuProject->addAction(m_actionEnableAcceleration);
    m_menuProject->addAction(m_actionEnablePhysics);
    m_menuProject->addAction(m_actionShowGrid);
    m_menuBar->addMenu(m_menuProject);
    m_menuScene = new QMenu(this);
    m_menuScene->addAction(m_actionZoomIn);
    m_menuScene->addAction(m_actionZoomOut);
    m_menuScene->addSeparator();
    m_menuScene->addAction(m_actionRotateUp);
    m_menuScene->addAction(m_actionRotateDown);
    m_menuScene->addAction(m_actionRotateLeft);
    m_menuScene->addAction(m_actionRotateRight);
    m_menuScene->addSeparator();
    m_menuScene->addAction(m_actionTranslateUp);
    m_menuScene->addAction(m_actionTranslateDown);
    m_menuScene->addAction(m_actionTranslateLeft);
    m_menuScene->addAction(m_actionTranslateRight);
    m_menuScene->addSeparator();
    m_menuScene->addAction(m_actionResetCamera);
    m_menuBar->addMenu(m_menuScene);
    m_menuModel = new QMenu(this);
    m_menuRetainModels = new QMenu(this);
    m_menuModel->addMenu(m_menuRetainModels);
    m_menuRetainAssets = new QMenu(this);
    //if (Asset::isSupported())
        m_menuScene->addMenu(m_menuRetainAssets);
    m_menuModel->addAction(m_actionSelectNextModel);
    m_menuModel->addAction(m_actionSelectPreviousModel);
    m_menuModel->addAction(m_actionRevertSelectedModel);
    m_menuModel->addSeparator();
    m_menuModel->addAction(m_actionTranslateModelUp);
    m_menuModel->addAction(m_actionTranslateModelDown);
    m_menuModel->addAction(m_actionTranslateModelLeft);
    m_menuModel->addAction(m_actionTranslateModelRight);
    m_menuModel->addSeparator();
    m_menuModel->addAction(m_actionResetModelPosition);
    m_menuModel->addSeparator();
    m_menuModel->addAction(m_actionDeleteSelectedModel);
    m_menuBar->addMenu(m_menuModel);
    m_menuFrame = new QMenu(this);
    m_menuFrame->addAction(m_actionRegisterFrame);
    m_menuFrame->addAction(m_actionInsertEmptyFrame);
    m_menuFrame->addAction(m_actionDeleteSelectedFrame);
    m_menuFrame->addSeparator();
    m_menuFrame->addAction(m_actionSelectAllFrames);
    m_menuFrame->addAction(m_actionSelectFrameDialog);
    m_menuFrame->addAction(m_actionFrameWeightDialog);
    m_menuFrame->addSeparator();
    m_menuFrame->addAction(m_actionNextFrame);
    m_menuFrame->addAction(m_actionPreviousFrame);
    m_menuFrame->addSeparator();
    m_menuFrame->addAction(m_actionBoneXPosZero);
    m_menuFrame->addAction(m_actionBoneYPosZero);
    m_menuFrame->addAction(m_actionBoneZPosZero);
    m_menuFrame->addAction(m_actionBoneResetAll);
    m_menuFrame->addAction(m_actionBoneDialog);
    m_menuBar->addMenu(m_menuFrame);
    m_menuView = new QMenu(this);
    m_menuView->addAction(m_actionViewLogMessage);
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionShowTimelineDock);
    m_menuView->addAction(m_actionShowSceneDock);
    m_menuView->addAction(m_actionShowModelDock);
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionShowModelDialog);
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionEnableMoveGesture);
    m_menuView->addAction(m_actionEnableRotateGesture);
    m_menuView->addAction(m_actionEnableScaleGesture);
    m_menuView->addAction(m_actionEnableUndoGesture);
    m_menuBar->addMenu(m_menuView);
    m_menuHelp = new QMenu(this);
    m_menuHelp->addAction(m_actionAbout);
    m_menuHelp->addAction(m_actionAboutQt);
    m_menuBar->addMenu(m_menuHelp);

    setCentralWidget(m_sceneWidget);
    updateRecentFiles();

    m_mainToolBar = new QToolBar();
    m_actionAddModelOnToolBar = m_mainToolBar->addAction("", m_sceneWidget, SLOT(addModel()));
    m_actionCreateMotionOnToolBar = m_mainToolBar->addAction("", this, SLOT(newMotionFile()));
    m_actionInsertMotionOnToolBar = m_mainToolBar->addAction("", m_sceneWidget, SLOT(insertMotionToSelectedModel()));
    m_actionAddAssetOnToolBar = m_mainToolBar->addAction("", m_sceneWidget, SLOT(addAsset()));
    m_actionDeleteModelOnToolBar = m_mainToolBar->addAction("", m_sceneWidget, SLOT(deleteSelectedModel()));
    addToolBar(m_mainToolBar);

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
    m_actionEnableAcceleration->setShortcut(m_settings.value(kPrefix + "enableAcceleration").toString());
    m_actionEnablePhysics->setShortcut(m_settings.value(kPrefix + "enablePhysics", "Ctrl+Shift+P").toString());
    m_actionShowGrid->setShortcut(m_settings.value(kPrefix + "showGrid", "Ctrl+Shift+G").toString());
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
    m_actionRegisterFrame->setShortcut(m_settings.value(kPrefix + "registerFrame", "Ctrl+E").toString());
    m_actionSelectAllFrames->setShortcut(m_settings.value(kPrefix + "selectAllFrames", "Ctrl+A").toString());
    m_actionSelectFrameDialog->setShortcut(m_settings.value(kPrefix + "selectFrameDialog", "Ctrl+Alt+S").toString());
    m_actionFrameWeightDialog->setShortcut(m_settings.value(kPrefix + "frameWeightDialog", "Ctrl+Alt+W").toString());
    m_actionInsertEmptyFrame->setShortcut(m_settings.value(kPrefix + "insertEmptyFrame", "Ctrl+I").toString());
    m_actionDeleteSelectedFrame->setShortcut(m_settings.value(kPrefix + "deleteSelectedFrame", "Ctrl+K").toString());
    m_actionNextFrame->setShortcut(m_settings.value(kPrefix + "nextFrame", QKeySequence(QKeySequence::Forward).toString()).toString());
    m_actionPreviousFrame->setShortcut(m_settings.value(kPrefix + "previousFrame", QKeySequence(QKeySequence::Back).toString()).toString());
    m_actionCut->setShortcut(m_settings.value(kPrefix + "cut", QKeySequence(QKeySequence::Cut).toString()).toString());
    m_actionCopy->setShortcut(m_settings.value(kPrefix + "copy", QKeySequence(QKeySequence::Copy).toString()).toString());
    m_actionPaste->setShortcut(m_settings.value(kPrefix + "paste", QKeySequence(QKeySequence::Paste).toString()).toString());
    m_actionReversedPaste->setShortcut(m_settings.value(kPrefix + "reversedPaste", "Alt+Ctrl+V").toString());
    m_actionUndoFrame->setShortcut(m_settings.value(kPrefix + "undoFrame", QKeySequence(QKeySequence::Undo).toString()).toString());
    m_actionRedoFrame->setShortcut(m_settings.value(kPrefix + "redoFrame", QKeySequence(QKeySequence::Redo).toString()).toString());
    m_actionViewLogMessage->setShortcut(m_settings.value(kPrefix + "viewLogMessage").toString());
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
    m_actionAddModel->setText(tr("Add model"));
    m_actionAddModel->setStatusTip(tr("Add a model to the scene."));
    m_actionAddAsset->setText(tr("Add asset"));
    m_actionAddAsset->setStatusTip(tr("Add an asset to the scene."));
    // m_actionAddAsset->setEnabled(Asset::isSupported());
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
    m_actionEnableAcceleration->setText(tr("Enable acceleration"));
    m_actionEnableAcceleration->setStatusTip(tr("Enable or disable acceleration using OpenCL if supported."));
    m_actionEnablePhysics->setText(tr("Enable physics simulation"));
    m_actionEnablePhysics->setStatusTip(tr("Enable or disable physics simulation using Bullet."));
    m_actionShowGrid->setText(tr("Show grid"));
    m_actionShowGrid->setStatusTip(tr("Show or hide scene grid."));
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
    m_actionRegisterFrame->setText(tr("Register keyframe"));
    m_actionRegisterFrame->setStatusTip(tr("Register keyframes by selected indices from the timeline."));
    m_actionSelectAllFrames->setText(tr("Select all keyframes"));
    m_actionSelectAllFrames->setStatusTip(tr("Select all registered keyframes."));
    m_actionSelectFrameDialog->setText(tr("Open keyframe range selection dialog"));
    m_actionSelectFrameDialog->setStatusTip(tr("Open keyframe range selection dialog to select multiple frame indices."));
    m_actionFrameWeightDialog->setText(tr("Open keyframe weight dialog"));
    m_actionFrameWeightDialog->setStatusTip(tr("Open keyframe weight dialog to set weight to selected registered keyframes."));
    m_actionInsertEmptyFrame->setText(tr("Insert empty keyframe"));
    m_actionInsertEmptyFrame->setStatusTip(tr("Insert an empty keyframe to the selected keyframe."));
    m_actionDeleteSelectedFrame->setText(tr("Delete selected keyframe"));
    m_actionDeleteSelectedFrame->setStatusTip(tr("Delete a selected keyframe."));
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
    m_actionViewLogMessage->setText(tr("Logger Window"));
    m_actionViewLogMessage->setStatusTip(tr("Open logger window."));
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
    m_actionAddModelOnToolBar->setText(m_actionAddModel->text());
    m_actionAddModelOnToolBar->setStatusTip(m_actionAddModel->statusTip());
    m_actionAddAssetOnToolBar->setText(m_actionAddAsset->text());
    m_actionAddAssetOnToolBar->setStatusTip(m_actionAddAsset->statusTip());
    m_actionInsertMotionOnToolBar->setText(tr("Add motion"));
    m_actionInsertMotionOnToolBar->setStatusTip(m_actionInsertToSelectedModel->toolTip());
    //m_actionSelectModelOnToolBar->setText(tr("Select model"));
    m_actionCreateMotionOnToolBar->setText(tr("Cerate motion"));
    m_actionDeleteModelOnToolBar->setText(tr("Delete model"));
    m_menuFile->setTitle(tr("&File"));
    m_menuEdit->setTitle(tr("&Edit"));
    m_menuProject->setTitle(tr("&Project"));
    m_menuScene->setTitle(tr("&Scene"));
    m_menuModel->setTitle(tr("&Model"));
    m_menuFrame->setTitle(tr("&Keyframe"));
    m_menuView->setTitle(tr("&View"));
    m_menuRetainAssets->setTitle(tr("Select asset"));
    m_menuRetainModels->setTitle(tr("Select model"));
    m_menuRecentFiles->setTitle(tr("Open recent files"));
    m_menuHelp->setTitle(tr("&Help"));
}

void MainWindow::connectSceneLoader()
{
    SceneLoader *loader = m_sceneWidget->sceneLoader();
    AssetWidget *assetWidget = m_sceneTabWidget->assetWidget();
    connect(loader, SIGNAL(modelDidAdd(vpvl2::IModel*,QUuid)), SLOT(addModel(vpvl2::IModel*,QUuid)));
    connect(loader, SIGNAL(modelWillDelete(vpvl2::IModel*,QUuid)), SLOT(deleteModel(vpvl2::IModel*,QUuid)));
    connect(loader, SIGNAL(assetDidAdd(vpvl2::IModel*,QUuid)), SLOT(deleteAsset(vpvl2::IModel*,QUuid)));
    connect(loader, SIGNAL(assetWillDelete(vpvl2::IModel*,QUuid)), SLOT(deleteAsset(vpvl2::IModel*,QUuid)));
    connect(loader, SIGNAL(modelWillDelete(vpvl2::IModel*,QUuid)), m_boneMotionModel, SLOT(removeModel()));
    connect(loader, SIGNAL(motionDidAdd(vpvl2::IMotion*,vpvl2::IModel*,QUuid)), m_boneMotionModel,SLOT(loadMotion(vpvl2::IMotion*,vpvl2::IModel*)));
    connect(loader, SIGNAL(modelDidMakePose(VPDFilePtr,vpvl2::IModel*)), m_timelineTabWidget, SLOT(loadPose(VPDFilePtr,vpvl2::IModel*)));
    connect(loader, SIGNAL(modelWillDelete(vpvl2::IModel*,QUuid)), m_morphMotionModel, SLOT(removeModel()));
    connect(loader, SIGNAL(motionDidAdd(vpvl2::IMotion*,vpvl2::IModel*,QUuid)), m_morphMotionModel, SLOT(loadMotion(vpvl2::IMotion*,vpvl2::IModel*)));
    connect(loader, SIGNAL(modelWillDelete(vpvl2::IModel*,QUuid)), m_timelineTabWidget->interpolationWidget(), SLOT(disable()));
    connect(loader, SIGNAL(assetDidAdd(vpvl2::IModel*,QUuid)), assetWidget, SLOT(addAsset(vpvl2::IModel*)));
    connect(loader, SIGNAL(assetWillDelete(vpvl2::IModel*,QUuid)), assetWidget, SLOT(removeAsset(vpvl2::IModel*)));
    connect(loader, SIGNAL(modelDidAdd(vpvl2::IModel*,QUuid)), assetWidget, SLOT(addModel(vpvl2::IModel*)));
    connect(loader, SIGNAL(modelWillDelete(vpvl2::IModel*,QUuid)), assetWidget, SLOT(removeModel(vpvl2::IModel*)));
    connect(loader, SIGNAL(modelDidAdd(vpvl2::IModel*,QUuid)), m_timelineTabWidget, SLOT(notifyCurrentTabIndex()));
    connect(loader, SIGNAL(motionDidAdd(vpvl2::IMotion*,vpvl2::IModel*,QUuid)), m_sceneMotionModel, SLOT(loadMotion(vpvl2::IMotion*)));
    connect(loader, SIGNAL(cameraMotionDidSet(vpvl2::IMotion*,QUuid)), m_sceneMotionModel, SLOT(loadMotion(vpvl2::IMotion*)));
    connect(loader, SIGNAL(projectDidLoad(bool)), m_sceneWidget, SLOT(updateSceneMotion()));
    connect(loader, SIGNAL(projectDidLoad(bool)), m_sceneMotionModel, SLOT(markAsNew()));
    connect(loader, SIGNAL(modelDidSelect(vpvl2::IModel*,SceneLoader*)), SLOT(setCurrentModel(vpvl2::IModel*)));
    connect(loader, SIGNAL(modelDidSelect(vpvl2::IModel*,SceneLoader*)), m_boneMotionModel, SLOT(setPMDModel(vpvl2::IModel*)));
    connect(loader, SIGNAL(modelDidSelect(vpvl2::IModel*,SceneLoader*)), m_morphMotionModel, SLOT(setPMDModel(vpvl2::IModel*)));
    connect(loader, SIGNAL(modelDidSelect(vpvl2::IModel*,SceneLoader*)), m_modelTabWidget->modelInfoWidget(), SLOT(setModel(vpvl2::IModel*)));
    connect(loader ,SIGNAL(modelDidSelect(vpvl2::IModel*,SceneLoader*)), m_modelTabWidget->modelSettingWidget(), SLOT(setModel(vpvl2::IModel*,SceneLoader*)));
    connect(loader, SIGNAL(assetDidSelect(vpvl2::IModel*,SceneLoader*)), assetWidget, SLOT(setAssetProperties(vpvl2::IModel*,SceneLoader*)));
    connect(m_actionEnableAcceleration, SIGNAL(triggered(bool)), loader, SLOT(setAccelerationEnabled(bool)));
    connect(m_actionEnablePhysics, SIGNAL(triggered(bool)), loader, SLOT(setPhysicsEnabled(bool)));
    connect(m_actionShowGrid, SIGNAL(toggled(bool)), loader, SLOT(setGridVisible(bool)));
    connect(assetWidget, SIGNAL(assetDidRemove(vpvl2::IModel*)), loader, SLOT(deleteAsset(vpvl2::IModel*)));
    connect(assetWidget, SIGNAL(assetDidSelect(vpvl2::IModel*)), loader, SLOT(setSelectedAsset(vpvl2::IModel*)));
    Handles *handles = m_sceneWidget->handles();
    connect(m_boneMotionModel, SIGNAL(positionDidChange(vpvl2::IBone*,vpvl2::Vector3)), handles, SLOT(updateBone()));
    connect(m_boneMotionModel, SIGNAL(rotationDidChange(vpvl2::IBone*,vpvl2::Quaternion)), handles, SLOT(updateBone()));
    connect(m_undo, SIGNAL(indexChanged(int)), handles, SLOT(updateBone()));
    connect(m_sceneWidget, SIGNAL(modelDidMove(vpvl2::Vector3)), handles, SLOT(updateBone()));
    connect(m_sceneWidget, SIGNAL(modelDidRotate(vpvl2::Quaternion)), handles, SLOT(updateBone()));
    /* カメラの初期値を設定。シグナル発行前に行う */
    CameraPerspectiveWidget *cameraWidget = m_sceneTabWidget->cameraPerspectiveWidget();
    Scene *scene = m_sceneWidget->sceneLoader()->scene();
    Scene::ICamera *camera = scene->camera();
    cameraWidget->setCameraPerspective(camera->position(), camera->angle(), camera->fovy(), camera->distance());
    connect(cameraWidget, SIGNAL(cameraPerspectiveDidChange(vpvl2::Vector3*,vpvl2::Vector3*,float*,float*)), m_sceneWidget, SLOT(setCameraPerspective(vpvl2::Vector3*,vpvl2::Vector3*,float*,float*)));
    connect(cameraWidget, SIGNAL(cameraPerspectiveDidReset()), m_sceneWidget, SLOT(updateSceneMotion()));
    connect(m_sceneWidget, SIGNAL(cameraPerspectiveDidSet(vpvl2::Vector3,vpvl2::Vector3,float,float)), cameraWidget, SLOT(setCameraPerspective(vpvl2::Vector3,vpvl2::Vector3,float,float)));
    /* 光源の初期値を設定。シグナル発行前に行う */
    SceneLightWidget *lightWidget = m_sceneTabWidget->sceneLightWidget();
    Scene::ILight *light = scene->light();
    lightWidget->setColor(light->color());
    lightWidget->setDirection(light->direction());
    connect(loader, SIGNAL(lightColorDidSet(vpvl2::Vector3)), lightWidget, SLOT(setColor(vpvl2::Vector3)));
    connect(loader, SIGNAL(lightDirectionDidSet(vpvl2::Vector3)), lightWidget, SLOT(setDirection(vpvl2::Vector3)));
    connect(lightWidget, SIGNAL(lightColorDidSet(vpvl2::Vector3)), loader, SLOT(setLightColor(vpvl2::Vector3)));
    connect(lightWidget, SIGNAL(lightDirectionDidSet(vpvl2::Vector3)), loader, SLOT(setLightPosition(vpvl2::Vector3)));
}

void MainWindow::connectWidgets()
{
    connect(m_sceneWidget, SIGNAL(initailizeGLContextDidDone()), SLOT(connectSceneLoader()));
    connect(m_sceneWidget, SIGNAL(fileDidLoad(QString)), SLOT(addRecentFile(QString)));
    connect(m_sceneWidget, SIGNAL(handleDidMoveAbsolute(vpvl2::Vector3,vpvl2::IBone*,int)), m_boneMotionModel, SLOT(translateTo(vpvl2::Vector3,vpvl2::IBone*,int)));
    connect(m_sceneWidget, SIGNAL(handleDidMoveRelative(vpvl2::Vector3,vpvl2::IBone*,int)), m_boneMotionModel, SLOT(translateDelta(vpvl2::Vector3,vpvl2::IBone*,int)));
    connect(m_sceneWidget, SIGNAL(handleDidRotate(vpvl2::Scalar,vpvl2::IBone*,int)), m_boneMotionModel, SLOT(rotateAngle(vpvl2::Scalar,vpvl2::IBone*,int)));
    connect(m_timelineTabWidget, SIGNAL(motionDidSeek(float)),  m_sceneWidget, SLOT(seekMotion(float)));
    connect(m_boneMotionModel, SIGNAL(motionDidModify(bool)), SLOT(setWindowModified(bool)));
    connect(m_morphMotionModel, SIGNAL(motionDidModify(bool)), SLOT(setWindowModified(bool)));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl2::IModel*)), m_boneMotionModel, SLOT(markAsNew(vpvl2::IModel*)));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl2::IModel*)), m_morphMotionModel, SLOT(markAsNew(vpvl2::IModel*)));
    connect(m_boneMotionModel, SIGNAL(motionDidUpdate(vpvl2::IModel*)), m_sceneWidget, SLOT(updateMotion()));
    connect(m_morphMotionModel, SIGNAL(motionDidUpdate(vpvl2::IModel*)), m_sceneWidget, SLOT(updateMotion()));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl2::IModel*)), m_timelineTabWidget, SLOT(setCurrentFrameIndexZero()));
    connect(m_sceneWidget, SIGNAL(boneDidSelect(QList<vpvl2::IBone*>)), m_boneMotionModel, SLOT(selectBones(QList<vpvl2::IBone*>)));
    connect(m_modelTabWidget->morphWidget(), SIGNAL(morphDidRegister(vpvl2::IMorph*)), m_timelineTabWidget, SLOT(addFaceKeyFrameAtCurrentFrameIndex(vpvl2::IMorph*)));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl2::IModel*)), m_sceneMotionModel, SLOT(markAsNew()));
    connect(m_sceneWidget, SIGNAL(handleDidGrab()), m_boneMotionModel, SLOT(saveTransform()));
    connect(m_sceneWidget, SIGNAL(handleDidRelease()), m_boneMotionModel, SLOT(commitTransform()));
    connect(m_sceneWidget, SIGNAL(motionDidSeek(float)), m_modelTabWidget->morphWidget(), SLOT(updateMorphWeightValues()));
    connect(m_sceneWidget, SIGNAL(undoDidRequest()), m_undo, SLOT(undo()));
    connect(m_sceneWidget, SIGNAL(redoDidRequest()), m_undo, SLOT(redo()));
    connect(m_timelineTabWidget, SIGNAL(editModeDidSet(SceneWidget::EditMode)), m_sceneWidget, SLOT(setEditMode(SceneWidget::EditMode)));
    ModelSettingWidget *modelSettingWidget = m_modelTabWidget->modelSettingWidget();
    connect(modelSettingWidget, SIGNAL(edgeOffsetDidChange(double)), m_sceneWidget, SLOT(setModelEdgeOffset(double)));
    connect(modelSettingWidget, SIGNAL(projectiveShadowDidChange(bool)), m_sceneWidget, SLOT(setModelProjectiveShadowEnable(bool)));
    connect(modelSettingWidget, SIGNAL(edgeColorDidChange(QColor)), m_sceneWidget, SLOT(setModelEdgeColor(QColor)));
    connect(modelSettingWidget, SIGNAL(positionOffsetDidChange(vpvl2::Vector3)), m_sceneWidget, SLOT(setModelPositionOffset(vpvl2::Vector3)));
    connect(modelSettingWidget, SIGNAL(rotationOffsetDidChange(vpvl2::Vector3)), m_sceneWidget, SLOT(setModelRotationOffset(vpvl2::Vector3)));
    connect(m_sceneWidget, SIGNAL(modelDidMove(vpvl2::Vector3)), modelSettingWidget, SLOT(setPositionOffset(vpvl2::Vector3)));
    makeBonesSelectable();
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
    const QString &filename = internal::openSaveDialog("mainWindow/lastPoseFileDirectory",
                                                       tr("Save model pose as a VPD file"),
                                                       tr("VPD file (*.vpd)"),
                                                       tr("untitled.vpd"),
                                                       &m_settings);
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QFile::WriteOnly)) {
            VPDFile pose;
            QTextStream stream(&file);
            m_timelineTabWidget->savePose(&pose, m_sceneWidget->sceneLoader()->selectedModel());
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
    m_sceneWidget->saveMetadataFromAsset(m_sceneTabWidget->assetWidget()->currentAsset());
}

void MainWindow::exportImage()
{
    const QString &filename = internal::openSaveDialog("mainWindow/lastImageDirectory",
                                                       tr("Export scene as an image"),
                                                       tr("Image (*.bmp, *.jpg, *.png)"),
                                                       tr("untitled.png"),
                                                       &m_settings);
    if (!filename.isEmpty()) {
        SceneLoader *loader = m_sceneWidget->sceneLoader();
        bool isGridVisible = loader->isGridVisible();
        loader->setGridVisible(false);
        m_sceneWidget->setHandlesVisible(false);
        m_sceneWidget->setInfoPanelVisible(false);
        m_sceneWidget->setBoneWireFramesVisible(false);
        m_sceneWidget->updateGL();
        const QImage &image = m_sceneWidget->grabFrameBuffer();
        loader->setGridVisible(isGridVisible);
        m_sceneWidget->setHandlesVisible(true);
        m_sceneWidget->setInfoPanelVisible(true);
        m_sceneWidget->setBoneWireFramesVisible(true);
        m_sceneWidget->updateGL();
        if (!image.isNull())
            image.save(filename);
        else
            qWarning("Failed exporting scene as an image: %s", qPrintable(filename));
    }
}

void MainWindow::exportVideo()
{
    if (VideoEncoder::isSupported()) {
        SceneLoader *loader = m_sceneWidget->sceneLoader();
        if (loader->scene()->maxFrameIndex() > 0) {
            if (!m_exportingVideoDialog) {
                const QSize min(160, 160);
                const QSize &max = m_sceneWidget->maximumSize();
                m_exportingVideoDialog = new ExportVideoDialog(loader, min, max, &m_settings, this);
            }
            m_exportingVideoDialog->show();
        }
        else {
            QMessageBox::warning(this, tr("No motion to export."),
                                 tr("Create or load a motion."));
        }
    }
    else {
        QMessageBox::warning(this, tr("Exporting video feature is not supported."),
                             tr("Exporting video is disabled because OpenCV is not linked."));
    }
}

void MainWindow::invokeVideoEncoder()
{
    m_exportingVideoDialog->close();
    int fromIndex = m_exportingVideoDialog->fromIndex();
    int toIndex = m_exportingVideoDialog->toIndex();
    if (fromIndex == toIndex) {
        QMessageBox::warning(this, tr("Value of \"Index from\" and \"Index to\" are equal."),
                             tr("Specify different value of \"Index from\" and \"Index to\"."));
        return;
    }
    else if (fromIndex > toIndex) {
        QMessageBox::warning(this, tr("Value of \"Index from\" is bigger than \"Index to\"."),
                             tr("\"Index from\" must be less than \"Index to\"."));
        return;
    }
    const QString &filename = internal::openSaveDialog("mainWindow/lastVideoDirectory",
                                                       tr("Export scene as a video"),
                                                       tr("Video (*.mov)"),
                                                       tr("untitled.mov"),
                                                       &m_settings);
    if (!filename.isEmpty()) {
        QProgressDialog *progress = new QProgressDialog(this);
        progress->setCancelButtonText(tr("Cancel"));
        progress->setWindowModality(Qt::ApplicationModal);
        const Scalar &fps = m_sceneWidget->sceneLoader()->scene()->preferredFPS();
        int width = m_exportingVideoDialog->sceneWidth();
        int height = m_exportingVideoDialog->sceneHeight();
        int frameIndex = m_sceneWidget->currentFrameIndex();
        /* 終了するまで待つ */
        if (m_audioDecoder && !m_audioDecoder->isFinished()) {
            m_audioDecoder->stop();
            m_audioDecoder->wait();
        }
        if (m_videoEncoder && !m_videoEncoder->isFinished()) {
            m_videoEncoder->stop();
            m_videoEncoder->wait();
        }
        delete m_audioDecoder;
        delete m_videoEncoder;
        int sceneFPS = m_exportingVideoDialog->sceneFPS();
        m_audioDecoder = new AudioDecoder();
        m_audioDecoder->setFilename(m_sceneWidget->sceneLoader()->backgroundAudio());
        bool canOpenAudio = m_audioDecoder->canOpen();
        int sampleRate = 0, bitRate = 0;
        if (canOpenAudio) {
            sampleRate = 44100;
            bitRate = 64000;
        }
        m_videoEncoder = new VideoEncoder(filename.toUtf8().constData(),
                                          QSize(width, height),
                                          sceneFPS,
                                          m_exportingVideoDialog->videoBitrate(),
                                          bitRate,
                                          sampleRate);
        connect(m_audioDecoder, SIGNAL(audioDidDecode(QByteArray)), m_videoEncoder, SLOT(enqueueAudioBuffer(QByteArray)));
        connect(this, SIGNAL(sceneDidRendered(QImage)), m_videoEncoder, SLOT(enqueueImage(QImage)));
        m_sceneWidget->setPreferredFPS(sceneFPS);
        const QString &exportingFormat = tr("Exporting frame %1 of %2...");
        int maxRangeIndex = toIndex - fromIndex;
        progress->setRange(0, maxRangeIndex);
        /* 画面を復元するために一時的に情報を保持。mainGeometry はコピーを持たないといけないので参照であってはならない */
        const QRect mainGeomtry = geometry();
        const QSize minSize = minimumSize(), maxSize = maximumSize(),
                videoSize = QSize(width, height), sceneSize = m_sceneWidget->size();
        QSizePolicy policy = sizePolicy();
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
        SceneLoader *loader = m_sceneWidget->sceneLoader();
        bool isGridVisible = loader->isGridVisible();
        /* レンダリングモードを自動更新から手動更新に変更 */
        m_sceneWidget->stop();
        m_sceneWidget->stopAutomaticRendering();
        loader->startPhysicsSimulation();
        loader->setGridVisible(m_exportingVideoDialog->includesGrid());
        /* ハンドルと情報パネルを非表示にし、ウィンドウを指定されたサイズに変更する */
        m_sceneWidget->setHandlesVisible(false);
        m_sceneWidget->setInfoPanelVisible(false);
        m_sceneWidget->setBoneWireFramesVisible(false);
        m_sceneWidget->resize(videoSize);
        /* モーションを0フレーム目に移動し、その後指定のキーフレームのインデックスに advance で移動させる */
        m_sceneWidget->seekMotion(0.0f, true);
        m_sceneWidget->advanceMotion(fromIndex);
        progress->setLabelText(exportingFormat.arg(0).arg(maxRangeIndex));
        /* 指定のキーフレームまで動画にフレームの書き出しを行う */
        m_videoEncoder->start();
        if (canOpenAudio)
            m_audioDecoder->start();
        float advanceSecond = 1.0f / (sceneFPS / Scene::defaultFPS()), totalAdvanced = 0.0f;
        /* 全てのモーションが終了するまでエンコード処理 */
        const Scene *scene = m_sceneWidget->sceneLoader()->scene();
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
        int remain = m_videoEncoder->sizeOfVideoQueue();
        progress->setValue(0);
        progress->setMaximum(remain);
        progress->setLabelText(encodingFormat.arg(0).arg(remain));
        int size = 0;
        /* 残りフレームをエンコード */
        while (remain > size) {
            if (progress->wasCanceled())
                break;
            size = remain - m_videoEncoder->sizeOfVideoQueue();
            progress->setValue(size);
            progress->setLabelText(encodingFormat.arg(size).arg(remain));
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        m_audioDecoder->stop();
        m_videoEncoder->stop();
        /* 画面情報を復元 */
        loader->setGridVisible(isGridVisible);
        setSizePolicy(policy);
        setMinimumSize(minSize);
        setMaximumSize(maxSize);
        setGeometry(mainGeomtry);
        statusBar()->show();
        /* ドック復元 */
        m_timelineDockWidget->show();
        m_sceneDockWidget->show();
        m_modelDockWidget->show();
        m_mainToolBar->show();
        /* 情報パネルとハンドルを再表示 */
        m_sceneWidget->resize(sceneSize);
        m_sceneWidget->setHandlesVisible(true);
        m_sceneWidget->setInfoPanelVisible(true);
        m_sceneWidget->setBoneWireFramesVisible(true);
        m_sceneWidget->setPreferredFPS(fps);
        /* レンダリングを手動更新から自動更新に戻す */
        loader->stopPhysicsSimulation();
        m_sceneWidget->seekMotion(frameIndex, true);
        m_sceneWidget->startAutomaticRendering();
        delete progress;
    }
}

void MainWindow::addNewMotion()
{
    if (maybeSaveMotion()) {
        IModel *model = m_sceneWidget->sceneLoader()->selectedModel();
        IMotion *motion = m_boneMotionModel->currentMotion();
        if (model && motion) {
            m_boneMotionModel->removeMotion();
            m_morphMotionModel->removeMotion();
            m_sceneMotionModel->removeMotion();
            m_sceneWidget->setEmptyMotion(model);
            m_boneMotionModel->markAsNew(model);
            m_morphMotionModel->markAsNew(model);
            m_sceneMotionModel->setModified(false);
        }
    }
}

void MainWindow::invokePlayer()
{
    if (m_sceneWidget->sceneLoader()->scene()->maxFrameIndex() > 0) {
        UICreatePlaySettingDialog(this, &m_settings, m_sceneWidget, m_playSettingDialog);
        UICreateScenePlayer(this, m_sceneWidget, m_playSettingDialog, m_timelineTabWidget, m_player);
        /*
         * 再生中はボーンが全選択になるのでワイヤーフレーム表示のオプションの関係からシグナルを一時的に解除する。
         * 停止後に makeBonesSelectable 経由でシグナルを復活させる
         */
        disconnect(m_boneMotionModel, SIGNAL(bonesDidSelect(QList<vpvl2::IBone*>)), m_sceneWidget, SLOT(selectBones(QList<vpvl2::IBone*>)));
        m_player->start();
    }
    else {
        QMessageBox::warning(this, tr("No motion to export."),
                             tr("Create or load a motion."));
    }
}

void MainWindow::openPlaySettingDialog()
{
    if (m_sceneWidget->sceneLoader()->scene()->maxFrameIndex() > 0) {
        UICreatePlaySettingDialog(this, &m_settings, m_sceneWidget, m_playSettingDialog);
        UICreateScenePlayer(this, m_sceneWidget, m_playSettingDialog, m_timelineTabWidget, m_player);
        m_playSettingDialog->show();
    }
    else {
        QMessageBox::warning(this, tr("No motion to export."),
                             tr("Create or load a motion."));
    }
}

void MainWindow::selectNextModel()
{
    const QList<QAction *> &actions = m_menuRetainModels->actions();
    if (!actions.isEmpty()) {
        const SceneLoader *loader = m_sceneWidget->sceneLoader();
        int index = UIFindIndexOfActions(m_sceneWidget->sceneLoader()->selectedModel(), actions);
        if (index == -1 || index == actions.length() - 1)
            m_sceneWidget->setSelectedModel(loader->findModel(actions.first()->text()));
        else
            m_sceneWidget->setSelectedModel(loader->findModel(actions.at(index + 1)->text()));
    }
}

void MainWindow::selectPreviousModel()
{
    const QList<QAction *> &actions = m_menuRetainModels->actions();
    if (!actions.isEmpty()) {
        const SceneLoader *loader = m_sceneWidget->sceneLoader();
        int index = UIFindIndexOfActions(m_sceneWidget->sceneLoader()->selectedModel(), actions);
        if (index == -1 || index == 0)
            m_sceneWidget->setSelectedModel(loader->findModel(actions.last()->text()));
        else
            m_sceneWidget->setSelectedModel(loader->findModel(actions.at(index - 1)->text()));
    }
}

void MainWindow::showLicenseWidget()
{
    if (!m_licenseWidget)
        m_licenseWidget = new LicenseWidget();
    m_licenseWidget->show();
}

void MainWindow::openGravitySettingDialog()
{
    GravitySettingDialog dialog(m_sceneWidget->sceneLoader());
    dialog.exec();
}

void MainWindow::openRenderOrderDialog()
{
    RenderOrderDialog dialog(m_sceneWidget->sceneLoader());
    dialog.exec();
}

void MainWindow::openScreenColorDialog()
{
    QColorDialog dialog;
    SceneLoader *loader = m_sceneWidget->sceneLoader();
    const QColor &before = loader->screenColor();
    connect(&dialog, SIGNAL(currentColorChanged(QColor)), loader, SLOT(setScreenColor(QColor)));
    if (dialog.exec() == QColorDialog::Rejected)
        loader->setScreenColor(before);
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

void MainWindow::makeBonesSelectable()
{
    connect(m_boneMotionModel, SIGNAL(bonesDidSelect(QList<vpvl2::IBone*>)), m_sceneWidget, SLOT(selectBones(QList<vpvl2::IBone*>)));
}

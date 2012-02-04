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

#include "BoneUIDelegate.h"
#include "MainWindow.h"
#include "VideoEncoder.h"

#include "common/Handles.h"
#include "common/LoggerWidget.h"
#include "common/SceneLoader.h"
#include "common/SceneWidget.h"
#include "common/VPDFile.h"
#include "common/util.h"
#include "dialogs/BoneDialog.h"
#include "dialogs/ExportVideoDialog.h"
#include "dialogs/FrameSelectionDialog.h"
#include "dialogs/PlaySettingDialog.h"
#include "models/BoneMotionModel.h"
#include "models/FaceMotionModel.h"
#include "models/SceneMotionModel.h"
#include "widgets/AssetWidget.h"
#include "widgets/CameraPerspectiveWidget.h"
#include "widgets/FaceWidget.h"
#include "widgets/InterpolationWidget.h"
#include "widgets/LicenseWidget.h"
#include "widgets/ModelInfoWidget.h"
#include "widgets/ModelTabWidget.h"
#include "widgets/TabWidget.h"
#include "widgets/TimelineTabWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

namespace {

static int FindIndexOfActions(vpvl::PMDModel *model, const QList<QAction *> &actions)
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

}

namespace internal {

class Player : QObject {
public:
    Player(SceneWidget *sceneWidget, PlaySettingDialog *dialog)
        : QObject(),
          m_sceneWidget(sceneWidget),
          m_dialog(dialog),
          m_progress(0),
          m_format(QApplication::tr("Playing scene frame %1 of %2...")),
          m_selected(0),
          m_step(0.0f),
          m_timerID(0),
          m_fromIndex(0),
          m_toIndex(0),
          m_maxIndex(0),
          m_loop(false)
    {
    }
    ~Player() {
    }

    void start() {
        m_fromIndex = m_dialog->fromIndex();
        m_toIndex = m_dialog->toIndex();
        m_loop = m_dialog->isLoop();
        m_selected = m_sceneWidget->selectedModel();
        m_sceneWidget->stop();
        m_sceneWidget->startPhysicsSimulation();
        /* 赤いエッジが残るため、選択状態のモデルを未選択状態にし、場面を最初の位置に戻す */
        m_sceneWidget->seekMotion(0.0f, true);
        m_sceneWidget->advanceMotion(m_fromIndex);
        /* ハンドルも情報パネルも消す */
        m_sceneWidget->setHandlesVisible(false);
        m_sceneWidget->setInfoPanelVisible(false);
        m_sceneWidget->setSelectedModel(0);
        m_sceneWidget->updateGL();
        m_progress = new QProgressDialog();
        m_progress->setCancelButtonText(QApplication::tr("Cancel"));
        m_progress->setWindowModality(Qt::WindowModal);
        int maxRangeIndex = m_toIndex - m_fromIndex;
        int preferredFPS = m_sceneWidget->scene()->preferredFPS();
        m_progress->setRange(0, maxRangeIndex);
        m_progress->setLabelText(m_format.arg(0).arg(maxRangeIndex));
        /* 再生用のタイマーからのみレンダリングを行わせるため、SceneWidget のタイマーを止めておく */
        m_sceneWidget->stopAutomaticRendering();
        m_timerID = startTimer(1000.0f / preferredFPS);
        m_step = static_cast<float>(vpvl::Scene::kFPS) / preferredFPS;
    }
    bool isActive() const {
        return m_timerID != 0;
    }

protected:
    void timerEvent(QTimerEvent *event) {
        if (event->timerId() == m_timerID) {
            vpvl::Scene *scene = m_sceneWidget->mutableScene();
            bool isReached = scene->isMotionReachedTo(m_toIndex);
            /* 再生完了かつループではない、またはユーザによってキャンセルされた場合再生用のタイマーイベントを終了する */
            if ((!m_loop && isReached) || m_progress->wasCanceled()) {
                killTimer(m_timerID);
                /* ハンドルと情報パネルを復帰させる */
                m_sceneWidget->setHandlesVisible(true);
                m_sceneWidget->setInfoPanelVisible(true);
                m_sceneWidget->setSelectedModel(m_selected);
                /* 再生が終わったら物理を無効にする */
                m_sceneWidget->stopPhysicsSimulation();
                /* SceneWidget を常時レンダリング状態に戻しておく */
                m_sceneWidget->startAutomaticRendering();
                delete m_progress;
                m_progress = 0;
                m_timerID = 0;
                return;
            }
            int value;
            if (isReached) {
                /* ループする場合は一旦リセットしてから開始位置に移動する */
                value = m_fromIndex;
                scene->resetMotion();
                m_sceneWidget->advanceMotion(value);
            }
            else {
                value = m_progress->value() + 1;
                m_sceneWidget->advanceMotion(m_step);
            }
            m_progress->setValue(value);
            m_progress->setLabelText(m_format.arg(value).arg(m_toIndex - m_fromIndex));
        }
    }

private:
    SceneWidget *m_sceneWidget;
    PlaySettingDialog *m_dialog;
    QProgressDialog *m_progress;
    QString m_format;
    vpvl::PMDModel *m_selected;
    float m_step;
    int m_timerID;
    int m_fromIndex;
    int m_toIndex;
    int m_maxIndex;
    bool m_loop;
};

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qAppName()),
    m_undo(0),
    m_licenseWidget(0),
    m_loggerWidget(0),
    m_sceneWidget(0),
    m_sceneTabWidget(0),
    m_modelTabWidget(0),
    m_timelineTabWidget(0),
    m_boneMotionModel(0),
    m_faceMotionModel(0),
    m_sceneMotionModel(0),
    m_exportingVideoDialog(0),
    m_playSettingDialog(0),
    m_boneUIDelegate(0),
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
    m_undo = new QUndoGroup(this);
    m_sceneWidget = new SceneWidget(&m_settings);
    /* SceneWidget で渡しているのは vpvl::Scene が initializeGL で初期化されるという特性のため */
    m_boneMotionModel = new BoneMotionModel(m_undo, m_sceneWidget, this);
    m_faceMotionModel = new FaceMotionModel(m_undo, this);
    m_sceneMotionModel = new SceneMotionModel(m_undo, m_sceneWidget, this);
    m_sceneTabWidget = new TabWidget(&m_settings);
    m_modelTabWidget = new ModelTabWidget(&m_settings, m_boneMotionModel, m_faceMotionModel, m_sceneMotionModel);
    m_timelineTabWidget = new TimelineTabWidget(&m_settings, m_boneMotionModel, m_faceMotionModel, m_sceneMotionModel);
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
    delete m_faceMotionModel;
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

void MainWindow::setCurrentModel(vpvl::PMDModel *model)
{
    m_model = model;
}

void MainWindow::newMotionFile()
{
    if (maybeSaveMotion()) {
        /*
         * PMDMotionModel のデータを空にしてから新規のモーションを作成する
         * なお、PMDMotionModel のデータは vpvl::VMDMotion とは独立している
         */
        m_boneMotionModel->removeMotion();
        m_faceMotionModel->removeMotion();
        m_sceneWidget->setEmptyMotion(m_sceneWidget->selectedModel());
    }
}

void MainWindow::newProjectFile()
{
    if (maybeSaveProject()) {
        /*
         * カメラを含むモーションとモデルを全て削除してからプロジェクトを新規に作成する
         * SceneWidget#clear は内部的に削除と同時に新しい空のプロジェクトが作成される
         */
        m_boneMotionModel->removeMotion();
        m_faceMotionModel->removeMotion();
        m_sceneMotionModel->removeMotion();
        m_sceneWidget->clear();
    }
}

void MainWindow::loadProject()
{
    if (maybeSaveProject()) {
        const QString &filename = m_sceneWidget->openFileDialog("mainWindow/lastProjectDirectory",
                                                                tr("Open VPVM file"),
                                                                tr("VPVM file (*.xml)"));
        if (!filename.isEmpty()) {
            m_boneMotionModel->removeMotion();
            m_faceMotionModel->removeMotion();
            m_sceneMotionModel->removeMotion();
            m_sceneWidget->sceneLoader()->loadProject(filename);
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

void MainWindow::addModel(vpvl::PMDModel *model, const QUuid &uuid)
{
    /* 追加されたモデルをモデル選択のメニューに追加する */
    QString name = internal::toQString(model);
    QAction *action = new QAction(name, this);
    action->setData(uuid.toString());
    action->setStatusTip(tr("Select a model %1").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(selectModel()));
    m_menuRetainModels->addAction(action);
    m_sceneWidget->setSelectedModel(model);
}

void MainWindow::deleteModel(vpvl::PMDModel *model, const QUuid &uuid)
{
    /* 削除されるモデルが選択中のモデルと同じなら選択状態を解除しておく(残すと不正アクセスの原因になるので) */
    if (model == m_sceneWidget->selectedModel())
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

void MainWindow::addAsset(vpvl::Asset *asset, const QUuid &uuid)
{
    /* 追加されたアクセサリをアクセサリ選択のメニューに追加する */
    QString name = internal::toQString(asset);
    QAction *action = new QAction(name, this);
    action->setData(uuid.toString());
    action->setStatusTip(tr("Select an asset %1").arg(name));
    //connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentModel()));
    m_menuRetainAssets->addAction(action);
}

void MainWindow::deleteAsset(vpvl::Asset * /* asset */, const QUuid &uuid)
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
    filename = openSaveDialog("mainWindow/lastVMDDirectory",
                              tr("Save motion as a VMD file"),
                              tr("VMD file (*.vmd)"),
                              tr("untitiled.vmd"));
    return !filename.isEmpty() ? saveMotionFile(filename) : false;
}

bool MainWindow::saveMotionFile(const QString &filename)
{
    /* 全てのボーンフレーム、頂点モーフフレーム、カメラフレームをファイルとして書き出しを行う */
    vpvl::VMDMotion motion;
    m_boneMotionModel->saveMotion(&motion);
    m_faceMotionModel->saveMotion(&motion);
    m_sceneMotionModel->saveMotion(&motion);
    size_t size = motion.estimateSize();
    uint8_t *buffer = new uint8_t[size];
    motion.save(buffer);
    QFile file(filename);
    bool ret = true;
    if (file.open(QFile::WriteOnly)) {
        file.write(reinterpret_cast<const char *>(buffer), size);
        file.close();
        qDebug("Saved a motion: %s", qPrintable(filename));
    }
    else {
        qWarning("Failed exporting VMD: %s", qPrintable(file.errorString()));
        ret = false;
    }
    delete[] buffer;
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
                              tr("untitled.xml"));
    return !filename.isEmpty() ? saveProjectFile(filename) : false;
}

bool MainWindow::saveProjectFile(const QString &filename)
{
    m_sceneWidget->sceneLoader()->saveProject(filename);
    return true;
}

bool MainWindow::maybeSaveMotion()
{
    bool cancel, cond = m_boneMotionModel->isModified()
            || m_faceMotionModel->isModified()
            || m_sceneMotionModel->isModified();
    if (confirmSave(cond, cancel))
        saveMotion();
    return !cancel;
}

bool MainWindow::maybeSaveProject()
{
    bool cancel, cond = m_boneMotionModel->isModified()
            || m_faceMotionModel->isModified()
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
    connect(m_actionNewProject, SIGNAL(triggered()), this, SLOT(newProjectFile()));
    m_actionNewMotion = new QAction(this);
    connect(m_actionNewMotion, SIGNAL(triggered()), this, SLOT(addNewMotion()));
    m_actionLoadProject = new QAction(this);
    connect(m_actionLoadProject, SIGNAL(triggered()), this, SLOT(loadProject()));
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
    connect(m_actionSaveModelPose, SIGNAL(triggered()), this, SLOT(saveModelPose()));
    m_actionLoadAssetMetadata = new QAction(this);
    connect(m_actionLoadAssetMetadata, SIGNAL(triggered()), m_sceneWidget, SLOT(addAssetFromMetadata()));
    m_actionSaveAssetMetadata = new QAction(this);
    connect(m_actionSaveAssetMetadata, SIGNAL(triggered()), this, SLOT(saveAssetMetadata()));
    m_actionSaveProject = new QAction(this);
    connect(m_actionSaveProject, SIGNAL(triggered()), this, SLOT(saveProject()));
    m_actionSaveProjectAs = new QAction(this);
    connect(m_actionSaveProjectAs, SIGNAL(triggered()), this, SLOT(saveProjectAs()));
    m_actionSaveMotion = new QAction(this);
    connect(m_actionSaveMotion, SIGNAL(triggered()), this, SLOT(saveMotion()));
    m_actionSaveMotionAs = new QAction(this);
    connect(m_actionSaveMotionAs, SIGNAL(triggered()), this, SLOT(saveMotionAs()));
    m_actionExportImage = new QAction(this);
    connect(m_actionExportImage, SIGNAL(triggered()), this, SLOT(exportImage()));
    m_actionExportVideo = new QAction(this);
    connect(m_actionExportVideo, SIGNAL(triggered()), this, SLOT(exportVideo()));
    m_actionExit = new QAction(this);
    m_actionExit->setMenuRole(QAction::QuitRole);
    connect(m_actionExit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    m_actionPlay = new QAction(this);
    connect(m_actionPlay, SIGNAL(triggered()), this, SLOT(startPlayingScene()));
    m_actionPlaySettings = new QAction(this);
    connect(m_actionPlaySettings, SIGNAL(triggered()), this, SLOT(openPlaySettingDialog()));
    m_actionEnableAcceleration = new QAction(this);
    m_actionEnableAcceleration->setCheckable(true);
    m_actionEnableAcceleration->setEnabled(SceneWidget::isAccelerationSupported());
    m_actionEnableAcceleration->setChecked(m_sceneWidget->isAccelerationEnabled());
    connect(m_actionEnableAcceleration, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setAccelerationEnable(bool)));
    m_actionEnablePhysics = new QAction(this);
    m_actionEnablePhysics->setCheckable(true);
    m_actionEnablePhysics->setChecked(m_sceneWidget->isPhysicsEnabled());
    connect(m_actionEnablePhysics, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setPhysicsEnable(bool)));
    m_actionShowGrid = new QAction(this);
    m_actionShowGrid->setCheckable(true);
    m_actionShowGrid->setChecked(m_sceneWidget->isGridVisible());
    connect(m_actionShowGrid, SIGNAL(toggled(bool)), m_sceneWidget, SLOT(setGridVisible(bool)));
    m_actionShowModelDialog = new QAction(this);
    m_actionShowModelDialog->setCheckable(true);
    m_actionShowModelDialog->setChecked(m_sceneWidget->showModelDialog());
    connect(m_actionShowModelDialog, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setShowModelDialog(bool)));
    m_actionShowBlackBackground = new QAction(this);
    m_actionShowBlackBackground->setCheckable(true);
    m_actionShowBlackBackground->setChecked(m_sceneWidget->isBlackBackgroundEnabled());
    connect(m_actionShowBlackBackground, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setBlackBackgroundEnable(bool)));

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
    connect(m_actionSelectNextModel, SIGNAL(triggered()), this, SLOT(selectNextModel()));
    m_actionSelectPreviousModel = new QAction(this);
    connect(m_actionSelectPreviousModel, SIGNAL(triggered()), this, SLOT(selectPreviousModel()));
    m_actionRevertSelectedModel = new QAction(this);
    connect(m_actionRevertSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(revertSelectedModel()));
    m_actionDeleteSelectedModel = new QAction(this);
    connect(m_actionDeleteSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(deleteSelectedModel()));
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

    m_actionRegisterFrame = new QAction(this);
    connect(m_actionRegisterFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(addKeyFramesFromSelectedIndices()));
    m_actionSelectAllFrames = new QAction(this);
    connect(m_actionSelectAllFrames, SIGNAL(triggered()), m_timelineTabWidget, SLOT(selectAllRegisteredKeyframes()));
    m_actionSelectFrameDialog = new QAction(this);
    connect(m_actionSelectFrameDialog, SIGNAL(triggered()), m_timelineTabWidget, SLOT(openFrameSelectionDialog()));
    m_actionFrameWeightDialog = new QAction(this);
    connect(m_actionFrameWeightDialog, SIGNAL(triggered()), m_timelineTabWidget, SLOT(openFrameWeightDialog()));
    m_actionCopy = new QAction(this);
    connect(m_actionCopy, SIGNAL(triggered()), m_timelineTabWidget, SLOT(copyFrame()));
    m_actionPaste = new QAction(this);
    connect(m_actionPaste, SIGNAL(triggered()), m_timelineTabWidget, SLOT(pasteFrame()));
    m_actionReversedPaste = new QAction(this);
    connect(m_actionReversedPaste, SIGNAL(triggered()), m_timelineTabWidget, SLOT(pasteReversedFrame()));
    m_actionInsertEmptyFrame = new QAction(this);
    connect(m_actionInsertEmptyFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(insertFrame()));
    m_actionDeleteSelectedFrame = new QAction(this);
    connect(m_actionDeleteSelectedFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(deleteFrame()));
    m_actionNextFrame = new QAction(this);
    connect(m_actionNextFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(nextFrame()));
    m_actionPreviousFrame = new QAction(this);
    connect(m_actionPreviousFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(previousFrame()));
    m_actionUndoFrame = m_undo->createUndoAction(this);
    m_actionRedoFrame = m_undo->createRedoAction(this);

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

    m_actionClearRecentFiles = new QAction(this);
    connect(m_actionClearRecentFiles, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
    m_actionAbout = new QAction(this);
    connect(m_actionAbout, SIGNAL(triggered()), this, SLOT(showLicenseWidget()));
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
        connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
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
    m_menuProject = new QMenu(this);
    m_menuProject->addAction(m_actionPlay);
    m_menuProject->addAction(m_actionPlaySettings);
    m_menuProject->addSeparator();
    m_menuProject->addAction(m_actionEnableAcceleration);
    m_menuProject->addAction(m_actionEnablePhysics);
    m_menuProject->addSeparator();
    m_menuProject->addAction(m_actionShowGrid);
    m_menuProject->addAction(m_actionShowModelDialog);
    m_menuProject->addAction(m_actionShowBlackBackground);
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
    if (vpvl::Asset::isSupported())
        m_menuScene->addMenu(m_menuRetainAssets);
    m_menuModel->addAction(m_actionSelectNextModel);
    m_menuModel->addAction(m_actionSelectPreviousModel);
    m_menuModel->addSeparator();
    m_menuModel->addAction(m_actionRevertSelectedModel);
    m_menuModel->addAction(m_actionDeleteSelectedModel);
    m_menuModel->addSeparator();
    m_menuModel->addAction(m_actionTranslateModelUp);
    m_menuModel->addAction(m_actionTranslateModelDown);
    m_menuModel->addAction(m_actionTranslateModelLeft);
    m_menuModel->addAction(m_actionTranslateModelRight);
    m_menuModel->addSeparator();
    m_menuModel->addAction(m_actionResetModelPosition);
    m_menuBar->addMenu(m_menuModel);
    m_menuBone = new QMenu(this);
    m_menuBone->addAction(m_actionBoneXPosZero);
    m_menuBone->addAction(m_actionBoneYPosZero);
    m_menuBone->addAction(m_actionBoneZPosZero);
    m_menuBone->addAction(m_actionBoneResetAll);
    m_menuBone->addSeparator();
    m_menuBone->addAction(m_actionBoneDialog);
    m_menuBar->addMenu(m_menuBone);
    m_menuFrame = new QMenu(this);
    m_menuFrame->addAction(m_actionRegisterFrame);
    m_menuFrame->addAction(m_actionSelectAllFrames);
    m_menuFrame->addAction(m_actionSelectFrameDialog);
    //m_menuFrame->addAction(m_actionFrameWeightDialog);
    m_menuFrame->addSeparator();
    m_menuFrame->addAction(m_actionInsertEmptyFrame);
    m_menuFrame->addAction(m_actionDeleteSelectedFrame);
    m_menuFrame->addSeparator();
    m_menuFrame->addAction(m_actionNextFrame);
    m_menuFrame->addAction(m_actionPreviousFrame);
    m_menuFrame->addSeparator();
    m_menuFrame->addAction(m_actionCopy);
    m_menuFrame->addAction(m_actionPaste);
    m_menuFrame->addAction(m_actionReversedPaste);
    m_menuFrame->addSeparator();
    m_menuFrame->addAction(m_actionUndoFrame);
    m_menuFrame->addAction(m_actionRedoFrame);
    m_menuBar->addMenu(m_menuFrame);
    m_menuView = new QMenu(this);
    //m_menuView->addAction(m_actionViewTransform);
    m_menuView->addAction(m_actionViewLogMessage);
    m_menuView->addSeparator();
    m_menuView->addAction(m_actionShowTimelineDock);
    m_menuView->addAction(m_actionShowSceneDock);
    m_menuView->addAction(m_actionShowModelDock);
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
    m_actionEnableAcceleration->setShortcut(m_settings.value(kPrefix + "enableAcceleration").toString());
    m_actionEnablePhysics->setShortcut(m_settings.value(kPrefix + "enablePhysics", "Ctrl+Shift+P").toString());
    m_actionShowGrid->setShortcut(m_settings.value(kPrefix + "showGrid", "Ctrl+Shift+G").toString());
    m_actionShowModelDialog->setShortcut(m_settings.value(kPrefix + "showModelDialog").toString());
    m_actionShowBlackBackground->setShortcut(m_settings.value(kPrefix + "showBlackBackground").toString());
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
    m_actionSelectFrameDialog->setShortcut(m_settings.value(kPrefix + "selectFrameDialog").toString());
    m_actionFrameWeightDialog->setShortcut(m_settings.value(kPrefix + "frameWeightDialog").toString());
    m_actionInsertEmptyFrame->setShortcut(m_settings.value(kPrefix + "insertEmptyFrame", "Ctrl+I").toString());
    m_actionDeleteSelectedFrame->setShortcut(m_settings.value(kPrefix + "deleteSelectedFrame", "Ctrl+K").toString());
    m_actionNextFrame->setShortcut(m_settings.value(kPrefix + "nextFrame", QKeySequence(QKeySequence::Forward).toString()).toString());
    m_actionPreviousFrame->setShortcut(m_settings.value(kPrefix + "previousFrame", QKeySequence(QKeySequence::Back).toString()).toString());
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
    m_actionAbout->setShortcut(m_settings.value(kPrefix + "about", "Alt+Q, Alt+/").toString());
    m_actionAboutQt->setShortcut(m_settings.value(kPrefix + "aboutQt").toString());
    m_actionClearRecentFiles->setShortcut(m_settings.value(kPrefix + "clearRecentFiles").toString());
}

void MainWindow::retranslate()
{
    m_mainToolBar->setWindowTitle(tr("Toolbar"));
    m_timelineDockWidget->setWindowTitle(tr("Timeline"));
    m_sceneDockWidget->setWindowTitle(tr("Scene"));
    m_modelDockWidget->setWindowTitle(tr("Model"));
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
    m_actionAddAsset->setEnabled(vpvl::Asset::isSupported());
    m_actionInsertToAllModels->setText(tr("Insert motion to all models"));
    m_actionInsertToAllModels->setStatusTip(tr("Insert a motion to the all models."));
    m_actionInsertToSelectedModel->setText(tr("Insert motion to selected model"));
    m_actionInsertToSelectedModel->setStatusTip(tr("Insert a motion to the selected model."));
    m_actionSaveProject->setText(tr("Save project"));
    m_actionSaveProject->setStatusTip(tr("Save current project as a file."));
    m_actionSaveProjectAs->setText(tr("Save project as"));
    m_actionSaveProjectAs->setStatusTip(tr("Save current project as a new file."));
    m_actionSaveMotion->setText(tr("Save motion"));
    m_actionSaveMotion->setStatusTip(tr("Export all key frames as a VMD."));
    m_actionSaveMotionAs->setText(tr("Save motion as"));
    m_actionSaveMotionAs->setStatusTip(tr("Export all key frames as a new VMD."));
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
    m_actionEnableAcceleration->setText(tr("Enable acceleration"));
    m_actionEnableAcceleration->setStatusTip(tr("Enable or disable acceleration using OpenCL if supported."));
    m_actionEnablePhysics->setText(tr("Enable physics simulation"));
    m_actionEnablePhysics->setStatusTip(tr("Enable or disable physics simulation using Bullet."));
    m_actionShowGrid->setText(tr("Show grid"));
    m_actionShowGrid->setStatusTip(tr("Show or hide scene grid."));
    m_actionShowModelDialog->setText(tr("Show model dialog"));
    m_actionShowModelDialog->setStatusTip(tr("Show or hide model dialog when the model is loaded."));
    m_actionShowBlackBackground->setText(tr("Set scene background black"));
    m_actionShowBlackBackground->setStatusTip(tr("Toggle scene background black/white."));
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
    m_menuProject->setTitle(tr("&Project"));
    m_menuScene->setTitle(tr("&Scene"));
    m_menuModel->setTitle(tr("&Model"));
    m_menuBone->setTitle(tr("&Bone"));
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
    connect(loader, SIGNAL(modelDidAdd(vpvl::PMDModel*,QUuid)), this, SLOT(addModel(vpvl::PMDModel*,QUuid)));
    connect(loader, SIGNAL(modelWillDelete(vpvl::PMDModel*,QUuid)), this, SLOT(deleteModel(vpvl::PMDModel*,QUuid)));
    connect(loader, SIGNAL(assetDidAdd(vpvl::Asset*,QUuid)), this, SLOT(deleteAsset(vpvl::Asset*,QUuid)));
    connect(loader, SIGNAL(assetWillDelete(vpvl::Asset*,QUuid)), this, SLOT(deleteAsset(vpvl::Asset*,QUuid)));
    //connect(loader, SIGNAL(modelDidAdd(vpvl::PMDModel*,QUuid)), m_boneMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(loader, SIGNAL(modelWillDelete(vpvl::PMDModel*,QUuid)), m_boneMotionModel, SLOT(removeModel()));
    connect(loader, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*,QUuid)), m_boneMotionModel,SLOT(loadMotion(vpvl::VMDMotion*,vpvl::PMDModel*)));
    connect(loader, SIGNAL(modelDidMakePose(VPDFile*,vpvl::PMDModel*)), m_timelineTabWidget, SLOT(loadPose(VPDFile*,vpvl::PMDModel*)));
    //connect(loader, SIGNAL(modelDidAdd(vpvl::PMDModel*,QUuid)), m_faceMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(loader, SIGNAL(modelWillDelete(vpvl::PMDModel*,QUuid)), m_faceMotionModel, SLOT(removeModel()));
    connect(loader, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*,QUuid)), m_faceMotionModel, SLOT(loadMotion(vpvl::VMDMotion*,vpvl::PMDModel*)));
    connect(loader, SIGNAL(modelWillDelete(vpvl::PMDModel*,QUuid)), m_modelTabWidget->interpolationWidget(), SLOT(disable()));
    connect(loader, SIGNAL(assetDidAdd(vpvl::Asset*,QUuid)), assetWidget, SLOT(addAsset(vpvl::Asset*)));
    connect(loader, SIGNAL(assetWillDelete(vpvl::Asset*,QUuid)), assetWidget, SLOT(removeAsset(vpvl::Asset*)));
    connect(loader, SIGNAL(modelDidAdd(vpvl::PMDModel*,QUuid)), assetWidget, SLOT(addModel(vpvl::PMDModel*)));
    connect(loader, SIGNAL(modelWillDelete(vpvl::PMDModel*,QUuid)), assetWidget, SLOT(removeModel(vpvl::PMDModel*)));
    connect(loader, SIGNAL(modelDidAdd(vpvl::PMDModel*,QUuid)), m_timelineTabWidget, SLOT(notifyCurrentTabIndex()));
    connect(loader, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*,QUuid)), m_sceneMotionModel, SLOT(loadMotion(vpvl::VMDMotion*)));
    connect(loader, SIGNAL(cameraMotionDidSet(vpvl::VMDMotion*,QUuid)), m_sceneMotionModel, SLOT(loadMotion(vpvl::VMDMotion*)));
    connect(loader, SIGNAL(projectDidLoad()), m_sceneWidget, SLOT(updateGL()));
    connect(assetWidget, SIGNAL(assetDidRemove(vpvl::Asset*)), loader, SLOT(deleteAsset(vpvl::Asset*)));
}

void MainWindow::connectWidgets()
{
    CameraPerspectiveWidget *cameraWidget = m_sceneTabWidget->cameraPerspectiveWidget();
    Handles *handles = m_sceneWidget->handles();
    connect(m_sceneWidget, SIGNAL(initailizeGLContextDidDone()), this, SLOT(connectSceneLoader()));
    connect(m_sceneWidget, SIGNAL(fileDidLoad(QString)), this, SLOT(addRecentFile(QString)));
    connect(m_sceneWidget, SIGNAL(modelDidSelect(vpvl::PMDModel*)), this, SLOT(setCurrentModel(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(handleDidMove(vpvl::Vector3,vpvl::Bone*,int)),
            m_boneMotionModel, SLOT(translate(vpvl::Vector3,vpvl::Bone*,int)));
    connect(m_sceneWidget, SIGNAL(handleDidRotate(vpvl::Quaternion,vpvl::Bone*,int,float)),
            m_boneMotionModel, SLOT(rotate(vpvl::Quaternion,vpvl::Bone*,int,float)));
    connect(cameraWidget, SIGNAL(cameraPerspectiveDidChange(vpvl::Vector3*,vpvl::Vector3*,float*,float*)),
            m_sceneWidget, SLOT(setCameraPerspective(vpvl::Vector3*,vpvl::Vector3*,float*,float*)));
    connect(m_timelineTabWidget, SIGNAL(currentTabDidChange(int)), m_modelTabWidget->interpolationWidget(), SLOT(setMode(int)));
    connect(m_timelineTabWidget, SIGNAL(motionDidSeek(float)),  m_sceneWidget, SLOT(seekMotion(float)));
    connect(m_boneMotionModel, SIGNAL(motionDidModify(bool)), this, SLOT(setWindowModified(bool)));
    connect(m_faceMotionModel, SIGNAL(motionDidModify(bool)), this, SLOT(setWindowModified(bool)));
    connect(m_boneMotionModel, SIGNAL(bonesDidSelect(QList<vpvl::Bone*>)), m_sceneWidget, SLOT(selectBones(QList<vpvl::Bone*>)));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl::PMDModel*)), m_boneMotionModel, SLOT(markAsNew(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl::PMDModel*)), m_faceMotionModel, SLOT(markAsNew(vpvl::PMDModel*)));
    connect(m_boneMotionModel, SIGNAL(motionDidUpdate(vpvl::PMDModel*)), m_sceneWidget, SLOT(updateMotion()));
    connect(m_faceMotionModel, SIGNAL(motionDidUpdate(vpvl::PMDModel*)), m_sceneWidget, SLOT(updateMotion()));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl::PMDModel*)), m_timelineTabWidget, SLOT(setCurrentFrameIndexZero()));
    connect(m_sceneWidget, SIGNAL(boneDidSelect(QList<vpvl::Bone*>)), m_boneMotionModel, SLOT(selectBones(QList<vpvl::Bone*>)));
    connect(m_modelTabWidget->faceWidget(), SIGNAL(faceDidRegister(vpvl::Face*)),
            m_timelineTabWidget, SLOT(addFaceKeyFrameAtCurrentFrameIndex(vpvl::Face*)));
    connect(m_sceneWidget, SIGNAL(cameraPerspectiveDidSet(vpvl::Vector3,vpvl::Vector3,float,float)),
            cameraWidget, SLOT(setCameraPerspective(vpvl::Vector3,vpvl::Vector3,float,float)));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl::PMDModel*)), m_sceneMotionModel, SLOT(markAsNew()));
    connect(m_sceneWidget, SIGNAL(modelDidSelect(vpvl::PMDModel*)), m_boneMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(modelDidSelect(vpvl::PMDModel*)), m_faceMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(m_boneMotionModel, SIGNAL(positionDidChange(vpvl::Bone*,vpvl::Vector3)), handles, SLOT(updateBone()));
    connect(m_boneMotionModel, SIGNAL(rotationDidChange(vpvl::Bone*,vpvl::Quaternion)), handles, SLOT(updateBone()));
    connect(m_undo, SIGNAL(indexChanged(int)), handles, SLOT(updateBone()));
    connect(m_sceneWidget, SIGNAL(handleDidGrab()), m_boneMotionModel, SLOT(saveTransform()));
    connect(m_sceneWidget, SIGNAL(handleDidRelease()), m_boneMotionModel, SLOT(commitTransform()));
    connect(m_sceneWidget, SIGNAL(modelDidMove(vpvl::Vector3)), handles, SLOT(updateBone()));
    connect(m_sceneWidget, SIGNAL(modelDidRotate(vpvl::Quaternion)), handles, SLOT(updateBone()));
    connect(m_sceneWidget, SIGNAL(motionDidSeek(float)), m_modelTabWidget->faceWidget(), SLOT(updateFaceWeightValues()));
    connect(m_sceneWidget, SIGNAL(undoDidRequest()), m_undo, SLOT(undo()));
    connect(m_sceneWidget, SIGNAL(redoDidRequest()), m_undo, SLOT(redo()));
    connect(cameraWidget, SIGNAL(cameraPerspectiveDidReset()), m_sceneWidget, SLOT(updateSceneMotion()));
    connect(m_sceneWidget, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            m_modelTabWidget->modelInfoWidget(), SLOT(setModel(vpvl::PMDModel*)));
    connect(m_modelTabWidget->modelInfoWidget(), SIGNAL(edgeOffsetDidChange(double)),
            m_sceneWidget, SLOT(setModelEdgeOffset(double)));
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
    const QString &filename = openSaveDialog("mainWindow/lastVPDDirectory",
                                             tr("Save model pose as a VPD file"),
                                             tr("VPD file (*.vpd)"),
                                             tr("untitled.vpd"));
    if (!filename.isEmpty()) {
        QFile file(filename);
        if (file.open(QFile::WriteOnly)) {
            VPDFile pose;
            QTextStream stream(&file);
            m_timelineTabWidget->savePose(&pose, m_sceneWidget->selectedModel());
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
    const QString &filename = openSaveDialog("mainWindow/lastImageDirectory",
                                             tr("Export scene as an image"),
                                             tr("Image (*.bmp, *.jpg, *.png)"),
                                             tr("untitled.png"));
    if (!filename.isEmpty()) {
        vpvl::PMDModel *selected = m_sceneWidget->selectedModel();
        bool visibleGrid = m_sceneWidget->isGridVisible();
        m_sceneWidget->setGridVisible(false);
        m_sceneWidget->setHandlesVisible(false);
        m_sceneWidget->setInfoPanelVisible(false);
        m_sceneWidget->setSelectedModel(0);
        m_sceneWidget->updateGL();
        QImage image = m_sceneWidget->grabFrameBuffer(true);
        m_sceneWidget->setGridVisible(visibleGrid);
        m_sceneWidget->setHandlesVisible(true);
        m_sceneWidget->setInfoPanelVisible(true);
        m_sceneWidget->setSelectedModel(selected);
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
        if (m_sceneWidget->scene()->maxFrameIndex() > 0) {
            delete m_exportingVideoDialog;
            m_exportingVideoDialog = new ExportVideoDialog(this, &m_settings, m_sceneWidget);
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

void MainWindow::startExportingVideo()
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
    const QString &filename = openSaveDialog("mainWindow/lastVideoDirectory",
                                             tr("Export scene as a video"),
                                             tr("Video (*.avi)"),
                                             tr("untitled.avi"));
    if (!filename.isEmpty()) {
        QProgressDialog *progress = new QProgressDialog(this);
        progress->setCancelButtonText(tr("Cancel"));
        progress->setWindowModality(Qt::WindowModal);
        int fps = m_sceneWidget->scene()->preferredFPS();
        int width = m_exportingVideoDialog->sceneWidth();
        int height = m_exportingVideoDialog->sceneHeight();
        if (m_videoEncoder && !m_videoEncoder->isFinished()) {
            emit encodingDidStopped();
            m_videoEncoder->wait();
        }
        delete m_videoEncoder;
        int sceneFPS = m_exportingVideoDialog->sceneFPS();
        m_videoEncoder = new VideoEncoder(filename.toUtf8().constData(),
                                          QSize(width, height),
                                          sceneFPS,
                                          m_exportingVideoDialog->videoBitrate(),
                                          64000,
                                          44100);
        m_sceneWidget->setPreferredFPS(sceneFPS);
        if (true) {
            const vpvl::Scene *scene = m_sceneWidget->scene();
            const QString &format = tr("Exporting frame %1 of %2...");
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
            vpvl::PMDModel *selected = m_sceneWidget->selectedModel();
            bool visibleGrid = m_sceneWidget->isGridVisible();
            /* 一旦止めてゼロにシークする。その後指定のキーフレームのインデックスに advance で移動させる */
            m_sceneWidget->stop();
            m_sceneWidget->stopAutomaticRendering();
            m_sceneWidget->startPhysicsSimulation();
            m_sceneWidget->seekMotion(0.0f, true);
            m_sceneWidget->advanceMotion(fromIndex);
            m_sceneWidget->setGridVisible(m_exportingVideoDialog->includesGrid());
            m_sceneWidget->setHandlesVisible(false);
            m_sceneWidget->setInfoPanelVisible(false);
            m_sceneWidget->setSelectedModel(0);
            m_sceneWidget->resize(videoSize);
            progress->setLabelText(format.arg(0).arg(maxRangeIndex));
            connect(this, SIGNAL(sceneDidRendered(QImage)), m_videoEncoder, SLOT(enqueueImage(QImage)));
            connect(this, SIGNAL(encodingDidStopped()), m_videoEncoder, SLOT(stop()));
            /* 画面乱れが発生することがあるのでウィンドウがリサイズするのを一旦待つ */
            QThread::currentThread()->wait(1000);
            /* 指定のキーフレームまで動画にフレームの書き出しを行う。キャンセルに対応している */
            m_videoEncoder->start();
            float advanceSecond = 1.0f / (sceneFPS / static_cast<float>(vpvl::Scene::kFPS)), totalAdvanced = 0.0f;
            while (!scene->isMotionReachedTo(toIndex)) {
                if (progress->wasCanceled())
                    break;
                QImage image = m_sceneWidget->grabFrameBuffer();
                if (image.width() != width || image.height() != height)
                    image = image.scaled(width, height);
                emit sceneDidRendered(image);
                int value = progress->value();
                if (totalAdvanced >= 1.0f) {
                    value += 1;
                    totalAdvanced = 0.0f;
                }
                progress->setValue(value);
                progress->setLabelText(format.arg(value).arg(maxRangeIndex));
                m_sceneWidget->advanceMotion(advanceSecond);
                m_sceneWidget->resize(videoSize);
                totalAdvanced += advanceSecond;
            }
            QImage image = m_sceneWidget->grabFrameBuffer();
            if (image.width() != width || image.height() != height)
                image = image.scaled(width, height);
            emit sceneDidRendered(image);
            emit encodingDidStopped();
            /* 画面情報を復元 */
            m_sceneWidget->setGridVisible(visibleGrid);
            m_sceneWidget->setHandlesVisible(true);
            m_sceneWidget->setInfoPanelVisible(true);
            m_sceneWidget->setSelectedModel(selected);
            m_sceneWidget->setPreferredFPS(fps);
            m_sceneWidget->resize(sceneSize);
            m_sceneWidget->stopPhysicsSimulation();
            m_sceneWidget->startAutomaticRendering();
            m_mainToolBar->show();
            m_timelineDockWidget->show();
            m_sceneDockWidget->show();
            m_modelDockWidget->show();
            statusBar()->show();
            setSizePolicy(policy);
            setMinimumSize(minSize);
            setMaximumSize(maxSize);
            setGeometry(mainGeomtry);
        }
        else {
            QMessageBox::warning(this, tr("Failed exporting video."),
                                 tr("Specified filepath cannot write to export a video."));
        }
        delete progress;
    }
}

void MainWindow::addNewMotion()
{
    if (maybeSaveMotion()) {
        vpvl::PMDModel *model = m_sceneWidget->selectedModel();
        vpvl::VMDMotion *motion = m_boneMotionModel->currentMotion();
        if (model && motion) {
            model->deleteMotion(motion);
            m_boneMotionModel->removeMotion();
            m_faceMotionModel->removeMotion();
            m_sceneMotionModel->removeMotion();
            m_sceneWidget->setEmptyMotion(model);
            m_boneMotionModel->markAsNew(model);
            m_faceMotionModel->markAsNew(model);
            m_sceneMotionModel->setModified(false);
        }
    }
}

void MainWindow::startPlayingScene()
{
    if (m_sceneWidget->scene()->maxFrameIndex() > 0) {
        if (!m_playSettingDialog)
            m_playSettingDialog = new PlaySettingDialog(this, &m_settings, m_sceneWidget);
        if (!m_player)
            m_player = new internal::Player(m_sceneWidget, m_playSettingDialog);
        if (!m_player->isActive())
            m_player->start();
    }
    else {
        QMessageBox::warning(this, tr("No motion to export."),
                             tr("Create or load a motion."));
    }
}

void MainWindow::openPlaySettingDialog()
{
    if (m_sceneWidget->scene()->maxFrameIndex() > 0) {
        delete m_playSettingDialog;
        m_playSettingDialog = new PlaySettingDialog(this, &m_settings, m_sceneWidget);
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
        int index = FindIndexOfActions(m_sceneWidget->selectedModel(), actions);
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
        int index = FindIndexOfActions(m_sceneWidget->selectedModel(), actions);
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

const QString MainWindow::openSaveDialog(const QString &name,
                                         const QString &desc,
                                         const QString &exts,
                                         const QString &defaultFilename)
{
    const QDir base(m_settings.value(name, QDir::homePath()).toString());
    const QString &path = base.absoluteFilePath(defaultFilename);
    const QString &fileName = QFileDialog::getSaveFileName(this, desc, path, exts);
    if (!fileName.isEmpty()) {
        QDir dir(fileName);
        dir.cdUp();
        m_settings.setValue(name, dir.absolutePath());
    }
    return fileName;
}

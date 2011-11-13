/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#include "AssetWidget.h"
#include "BoneDialog.h"
#include "BoneMotionModel.h"
#include "CameraPerspectiveWidget.h"
#include "FaceMotionModel.h"
#include "FaceWidget.h"
#include "InterpolationWidget.h"
#include "LicenseWidget.h"
#include "SceneMotionModel.h"
#include "SceneWidget.h"
#include "TabWidget.h"
#include "TimelineTabWidget.h"
#include "TransformWidget.h"
#include "VPDFile.h"
#include "util.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

#ifdef OPENCV_FOUND
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#endif

ExportVideoDialog::ExportVideoDialog(MainWindow *parent, SceneWidget *scene) : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    int maxFrameIndex = scene->scene()->maxFrameIndex();
    m_widthBox = new QSpinBox();
    m_widthBox->setRange(scene->minimumWidth(), scene->maximumWidth());
    m_heightBox = new QSpinBox();
    m_heightBox->setRange(scene->minimumHeight(), scene->maximumHeight());
    m_fromIndexBox = new QSpinBox();
    m_fromIndexBox->setRange(0, maxFrameIndex);
    m_toIndexBox = new QSpinBox();
    m_toIndexBox->setRange(0, maxFrameIndex);
    m_toIndexBox->setValue(maxFrameIndex);
    m_includeGridBox = new QCheckBox(tr("Include grid field"));
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(new QLabel(tr("Width")), 0, 0);
    gridLayout->addWidget(m_widthBox, 0, 1);
    gridLayout->addWidget(new QLabel(tr("Height")), 0, 2);
    gridLayout->addWidget(m_heightBox, 0, 3);
    gridLayout->addWidget(new QLabel(tr("Keyframe from")), 1, 0);
    gridLayout->addWidget(m_fromIndexBox, 1, 1);
    gridLayout->addWidget(new QLabel(tr("Keyframe to")), 1, 2);
    gridLayout->addWidget(m_toIndexBox, 1, 3);
    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(m_includeGridBox, 0, Qt::AlignCenter);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), parent, SLOT(startExportingVideo()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(close()));
    mainLayout->addWidget(buttons);
    setWindowTitle(tr("Exporting video setting"));
    setLayout(mainLayout);
}

ExportVideoDialog::~ExportVideoDialog()
{
}

int ExportVideoDialog::sceneWidth() const
{
    return m_widthBox->value();
}

int ExportVideoDialog::sceneHeight() const
{
    return m_heightBox->value();
}

int ExportVideoDialog::fromIndex() const
{
    return m_fromIndexBox->value();
}

int ExportVideoDialog::toIndex() const
{
    return m_toIndexBox->value();
}

bool ExportVideoDialog::includesGrid() const
{
    return m_includeGridBox->isChecked();
}

EdgeOffsetDialog::EdgeOffsetDialog(MainWindow *parent, SceneWidget *scene)
    : QDialog(parent),
      m_spinBox(0),
      m_sceneWidget(scene),
      m_selected(scene->selectedModel()),
      m_edgeOffset(scene->modelEdgeOffset())
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QLabel *label = new QLabel(tr("Model edge offset value"));
    m_spinBox = new QDoubleSpinBox();
    connect(m_spinBox, SIGNAL(valueChanged(double)), this, SLOT(setEdgeOffset(double)));
    m_spinBox->setValue(m_edgeOffset);
    m_spinBox->setSingleStep(0.1f);
    m_spinBox->setRange(0.0f, 2.0f);
    QHBoxLayout *subLayout = new QHBoxLayout();
    subLayout->addWidget(label);
    subLayout->addWidget(m_spinBox);
    mainLayout->addLayout(subLayout);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(buttons, SIGNAL(accepted()), this, SLOT(commit()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(rollback()));
    mainLayout->addWidget(buttons);
    setWindowTitle(tr("Edge offset dialog"));
    setLayout(mainLayout);
    m_sceneWidget->hideSelectedModelEdge();
}

EdgeOffsetDialog::~EdgeOffsetDialog()
{
}

void EdgeOffsetDialog::closeEvent(QCloseEvent * /* event */)
{
    m_sceneWidget->showSelectedModelEdge();
}

void EdgeOffsetDialog::setEdgeOffset(double value)
{
    m_selected->setEdgeOffset(value);
    m_sceneWidget->setModelEdgeOffset(m_edgeOffset);
    m_sceneWidget->updateMotion();
}

void EdgeOffsetDialog::commit()
{
    m_selected->setEdgeOffset(m_spinBox->value());
    close();
}

void EdgeOffsetDialog::rollback()
{
    m_selected->setEdgeOffset(m_edgeOffset);
    close();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qAppName()),
    m_undo(0),
    m_licenseWidget(0),
    m_sceneWidget(0),
    m_tabWidget(0),
    m_timelineTabWidget(0),
    m_transformWidget(0),
    m_boneMotionModel(0),
    m_faceMotionModel(0),
    m_sceneMotionModel(0),
    m_exportingVideoDialog(0),
    m_model(0),
    m_bone(0),
    m_position(0.0f, 0.0f, 0.0f),
    m_angle(0.0f, 0.0f, 0.0f),
    m_fovy(0.0f),
    m_distance(0.0f),
    m_currentFPS(-1)
{
    m_undo = new QUndoGroup(this);
    m_licenseWidget = new LicenseWidget();
    m_sceneWidget = new SceneWidget(&m_settings);
    m_boneMotionModel = new BoneMotionModel(m_undo, m_sceneWidget, this);
    m_faceMotionModel = new FaceMotionModel(m_undo, this);
    m_sceneMotionModel = new SceneMotionModel(m_undo, this);
    m_tabWidget = new TabWidget(&m_settings, m_boneMotionModel, m_faceMotionModel);
    m_timelineTabWidget = new TimelineTabWidget(&m_settings, m_boneMotionModel, m_faceMotionModel, m_sceneMotionModel);
    m_transformWidget = new TransformWidget(&m_settings, m_boneMotionModel, m_faceMotionModel);
    buildUI();
    connectWidgets();
    restoreGeometry(m_settings.value("mainWindow/geometry").toByteArray());
    restoreState(m_settings.value("mainWindow/state").toByteArray());
    setWindowTitle(QString("[*]%1").arg(qAppName()));
    statusBar()->show();
}

MainWindow::~MainWindow()
{
    delete m_licenseWidget;
    delete m_tabWidget;
    delete m_timelineTabWidget;
    delete m_transformWidget;
    delete m_menuBar;
}

bool MainWindow::validateLibraryVersion()
{
    if (!vpvl::isLibraryVersionCorrect(VPVL_VERSION)) {
        QMessageBox::warning(this,
                             tr("libvpvl version mismatch"),
                             tr("libvpvl's version is incorrect (expected: %1 actual: %2).\n"
                                "Please replace libvpvl to correct version or reinstall MMDAI.")
                             .arg(VPVL_VERSION_STRING).arg(vpvl::libraryVersionString()));
        return false;
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        m_settings.setValue("mainWindow/geometry", saveGeometry());
        m_settings.setValue("mainWindow/state", saveState());
        m_settings.setValue("mainWindow/visibleTabs", m_tabWidget->isVisible());
        m_settings.setValue("mainWindow/visibleTimeline", m_timelineTabWidget->isVisible());
        m_settings.setValue("mainWindow/visibleTransform", m_transformWidget->isVisible());
        m_settings.setValue("mainWindow/leftSplitterGeometry", m_leftSplitter->saveGeometry());
        m_settings.setValue("mainWindow/leftSplitterState", m_leftSplitter->saveState());
        m_settings.setValue("mainWindow/mainSplitterGeometry", m_mainSplitter->saveGeometry());
        m_settings.setValue("mainWindow/mainSplitterState", m_mainSplitter->saveState());
        qApp->sendEvent(m_sceneWidget, event);
        event->accept();
    }
    else {
        event->ignore();
    }
}

void MainWindow::selectModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        m_sceneWidget->setSelectedModel(m_sceneWidget->findModel(action->text()));
}

void MainWindow::setCurrentModel(vpvl::PMDModel *model)
{
    m_model = model;
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        m_sceneWidget->setEmptyMotion(m_sceneWidget->selectedModel());
        m_boneMotionModel->removeMotion();
        m_faceMotionModel->removeMotion();
    }
}

bool MainWindow::save()
{
    return saveAs();
}

bool MainWindow::saveAs()
{
    const QString &filename = openSaveDialog("mainWindow/lastVMDDirectory",
                                             tr("Save Motion as a VMD file"),
                                             tr("VMD file (*.vmd)"));
    return !filename.isEmpty() ? saveFile(filename) : false;
}

bool MainWindow::saveFile(const QString &filename)
{
    vpvl::VMDMotion motion;
    m_boneMotionModel->saveMotion(&motion);
    m_faceMotionModel->saveMotion(&motion);
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

bool MainWindow::maybeSave()
{
    if (m_boneMotionModel->isModified() || m_faceMotionModel->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this,
                                   qAppName(),
                                   tr("Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        switch (ret) {
        case QMessageBox::Save:
            return save();
        case QMessageBox::Cancel:
            return false;
        default:
            return true;
        }
    }
    return true;
}

void MainWindow::revertSelectedModel()
{
    m_sceneWidget->setSelectedModel(0);
}

void MainWindow::addModel(vpvl::PMDModel *model)
{
    QString name = internal::toQString(model);
    QAction *action = new QAction(name, this);
    action->setStatusTip(tr("Select a model %1").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(selectModel()));
    m_menuRetainModels->addAction(action);
}

void MainWindow::deleteModel(vpvl::PMDModel *model)
{
    QAction *actionToRemove = 0;
    QString name = internal::toQString(model);
    foreach (QAction *action, m_menuRetainModels->actions()) {
        if (action->text() == name) {
            actionToRemove = action;
            break;
        }
    }
    if (actionToRemove)
        m_menuRetainModels->removeAction(actionToRemove);
}

void MainWindow::addAsset(vpvl::Asset *asset)
{
    QString name = internal::toQString(asset);
    QAction *action = new QAction(name, this);
    action->setStatusTip(tr("Select an asset %1").arg(name));
    //connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentModel()));
    m_menuRetainAssets->addAction(action);
}

void MainWindow::deleteAsset(vpvl::Asset *asset)
{
    QAction *actionToRemove = 0;
    QString name = internal::toQString(asset);
    foreach (QAction *action, m_menuRetainAssets->actions()) {
        if (action->text() == name) {
            actionToRemove = action;
            break;
        }
    }
    if (actionToRemove)
        m_menuRetainAssets->removeAction(actionToRemove);
}

void MainWindow::buildUI()
{
    m_actionAddModel = new QAction(this);
    connect(m_actionAddModel, SIGNAL(triggered()), m_sceneWidget, SLOT(addModel()));
    m_actionAddAsset = new QAction(this);
    connect(m_actionAddAsset, SIGNAL(triggered()), m_sceneWidget, SLOT(addAsset()));
    m_actionNewMotion = new QAction(this);
    connect(m_actionNewMotion, SIGNAL(triggered()), this, SLOT(addNewMotion()));
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
    m_actionSaveMotion = new QAction(this);
    connect(m_actionSaveMotion, SIGNAL(triggered()), this, SLOT(saveAs()));
    m_actionExportImage = new QAction(this);
    connect(m_actionExportImage, SIGNAL(triggered()), this, SLOT(exportImage()));
    m_actionExportVideo = new QAction(this);
    connect(m_actionExportVideo, SIGNAL(triggered()), this, SLOT(exportVideo()));
    m_actionExit = new QAction(this);
    m_actionExit->setMenuRole(QAction::QuitRole);
    connect(m_actionExit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    m_actionPlay = new QAction(this);
    connect(m_actionPlay, SIGNAL(triggered()), m_sceneWidget, SLOT(play()));
    m_actionPause = new QAction(this);
    connect(m_actionPause, SIGNAL(triggered()), m_sceneWidget, SLOT(pause()));
    m_actionStop = new QAction(this);
    connect(m_actionStop, SIGNAL(triggered()), m_sceneWidget, SLOT(stop()));
    m_actionShowGrid = new QAction(this);
    m_actionShowGrid->setCheckable(true);
    m_actionShowGrid->setChecked(m_sceneWidget->isGridVisible());
    connect(m_actionShowGrid, SIGNAL(toggled(bool)), m_sceneWidget, SLOT(setGridVisible(bool)));
    m_actionShowBones = new QAction(this);
    m_actionShowBones->setCheckable(true);
    m_actionShowBones->setChecked(m_sceneWidget->isBoneWireframeVisible());
    connect(m_actionShowBones, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setBoneWireframeVisible(bool)));
    m_actionEnablePhysics = new QAction(this);
    m_actionEnablePhysics->setCheckable(true);
    m_actionEnablePhysics->setChecked(m_sceneWidget->isPhysicsEnabled());
    connect(m_actionEnablePhysics, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setPhysicsEnable(bool)));
    m_actionShowModelDialog = new QAction(this);
    m_actionShowModelDialog->setCheckable(true);
    m_actionShowModelDialog->setChecked(m_sceneWidget->showModelDialog());
    connect(m_actionShowModelDialog, SIGNAL(triggered(bool)), m_sceneWidget, SLOT(setShowModelDialog(bool)));

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

    m_actionRevertSelectedModel = new QAction(this);
    connect(m_actionRevertSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(revertSelectedModel()));
    m_actionDeleteSelectedModel = new QAction(this);
    connect(m_actionDeleteSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(deleteSelectedModel()));
    m_actionEdgeOffsetDialog = new QAction(this);
    connect(m_actionEdgeOffsetDialog, SIGNAL(triggered()), this, SLOT(openEdgeOffsetDialog()));
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
    connect(m_actionBoneXPosZero, SIGNAL(triggered()), this, SLOT(resetBoneX()));
    m_actionBoneYPosZero = new QAction(this);
    connect(m_actionBoneYPosZero, SIGNAL(triggered()), this, SLOT(resetBoneY()));
    m_actionBoneZPosZero = new QAction(this);
    connect(m_actionBoneZPosZero, SIGNAL(triggered()), this, SLOT(resetBoneZ()));
    m_actionBoneRotationZero = new QAction(this);
    connect(m_actionBoneRotationZero, SIGNAL(triggered()), this, SLOT(resetBoneRotation()));
    m_actionBoneResetAll = new QAction(this);
    connect(m_actionBoneResetAll, SIGNAL(triggered()), this, SLOT(resetAllBones()));
    m_actionBoneDialog = new QAction(this);
    connect(m_actionBoneDialog, SIGNAL(triggered()), this, SLOT(openBoneDialog()));

    m_actionRegisterFrame = new QAction(this);
    connect(m_actionRegisterFrame, SIGNAL(triggered()), m_timelineTabWidget, SLOT(addKeyFramesFromSelectedIndices()));
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
    m_actionUndoFrame = m_undo->createUndoAction(this);
    m_actionRedoFrame = m_undo->createRedoAction(this);

    m_actionViewTab = new QAction(this);
    connect(m_actionViewTab, SIGNAL(triggered()), m_tabWidget, SLOT(show()));
    m_actionViewTimeline = new QAction(this);
    connect(m_actionViewTimeline, SIGNAL(triggered()), m_timelineTabWidget, SLOT(show()));
    m_actionViewTransform = new QAction(this);
    connect(m_actionViewTransform, SIGNAL(triggered()), m_transformWidget, SLOT(show()));

    m_actionAbout = new QAction(this);
    connect(m_actionAbout, SIGNAL(triggered()), m_licenseWidget, SLOT(show()));
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
    m_menuFile->addAction(m_actionAddModel);
    m_menuFile->addAction(m_actionAddAsset);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionNewMotion);
    m_menuFile->addAction(m_actionInsertToAllModels);
    m_menuFile->addAction(m_actionInsertToSelectedModel);
    m_menuFile->addAction(m_actionSetCamera);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionLoadModelPose);
    m_menuFile->addAction(m_actionSaveModelPose);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionLoadAssetMetadata);
    m_menuFile->addAction(m_actionSaveAssetMetadata);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionSaveMotion);
    m_menuFile->addAction(m_actionExportImage);
    m_menuFile->addAction(m_actionExportVideo);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionExit);
    m_menuBar->addMenu(m_menuFile);
    m_menuProject = new QMenu(this);
    m_menuProject->addAction(m_actionPlay);
    m_menuProject->addAction(m_actionPause);
    m_menuProject->addAction(m_actionStop);
    m_menuProject->addSeparator();
    m_menuProject->addAction(m_actionShowGrid);
    m_menuProject->addAction(m_actionShowBones);
    m_menuProject->addAction(m_actionEnablePhysics);
    m_menuProject->addAction(m_actionShowModelDialog);
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
    m_menuModel->addAction(m_actionRevertSelectedModel);
    m_menuModel->addAction(m_actionDeleteSelectedModel);
    m_menuModel->addAction(m_actionEdgeOffsetDialog);
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
    m_menuFrame->addAction(m_actionInsertEmptyFrame);
    m_menuFrame->addAction(m_actionDeleteSelectedFrame);
    m_menuFrame->addSeparator();
    m_menuFrame->addAction(m_actionCopy);
    m_menuFrame->addAction(m_actionPaste);
    m_menuFrame->addAction(m_actionReversedPaste);
    m_menuFrame->addAction(m_actionUndoFrame);
    m_menuFrame->addAction(m_actionRedoFrame);
    m_menuBar->addMenu(m_menuFrame);
    m_menuView = new QMenu(this);
    //m_menuView->addAction(m_actionViewTab);
    //m_menuView->addAction(m_actionViewTimeline);
    m_menuView->addAction(m_actionViewTransform);
    m_menuBar->addMenu(m_menuView);
    m_menuHelp = new QMenu(this);
    m_menuHelp->addAction(m_actionAbout);
    m_menuHelp->addAction(m_actionAboutQt);
    m_menuBar->addMenu(m_menuHelp);

    connect(m_sceneWidget, SIGNAL(modelDidAdd(vpvl::PMDModel*)), this, SLOT(addModel(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(modelWillDelete(vpvl::PMDModel*)), this, SLOT(deleteModel(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(modelDidSelect(vpvl::PMDModel*)), this, SLOT(setCurrentModel(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(assetDidAdd(vpvl::Asset*)), this, SLOT(addAsset(vpvl::Asset*)));
    connect(m_sceneWidget, SIGNAL(assetWillDelete(vpvl::Asset*)), this, SLOT(deleteAsset(vpvl::Asset*)));

    bool visibleTransform = m_settings.value("mainWindow/visibleTransform", QVariant(false)).toBool();
    m_transformWidget->setVisible(visibleTransform);
    m_leftSplitter = new QSplitter(Qt::Vertical);
    m_leftSplitter->setStretchFactor(0, 1);
    m_leftSplitter->setStretchFactor(1, 0);
    m_leftSplitter->addWidget(m_timelineTabWidget);
    m_leftSplitter->addWidget(m_tabWidget);
    m_leftSplitter->restoreGeometry(m_settings.value("mainWindow/leftSplitterGeometry").toByteArray());
    m_leftSplitter->restoreState(m_settings.value("mainWindow/leftSplitterState").toByteArray());
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);
    m_mainSplitter->addWidget(m_leftSplitter);
    m_mainSplitter->addWidget(m_sceneWidget);
    m_mainSplitter->restoreGeometry(m_settings.value("mainWindow/mainSplitterGeometry").toByteArray());
    m_mainSplitter->restoreState(m_settings.value("mainWindow/mainSplitterState").toByteArray());
    setCentralWidget(m_mainSplitter);
    m_mainSplitter->setFocus();

    retranslate();
}

void MainWindow::retranslate()
{
    m_actionAddModel->setText(tr("Add model"));
    m_actionAddModel->setStatusTip(tr("Add a model to the scene."));
    m_actionAddModel->setShortcut(tr("Ctrl+Shift+M"));
    m_actionAddAsset->setText(tr("Add asset"));
    m_actionAddAsset->setStatusTip(tr("Add an asset to the scene."));
    m_actionAddAsset->setShortcut(tr("Ctrl+Shift+A"));
    m_actionAddAsset->setEnabled(vpvl::Asset::isSupported());
    m_actionNewMotion->setText(tr("New motion"));
    m_actionNewMotion->setStatusTip(tr("Insert a new motion to the selected model."));
    m_actionInsertToAllModels->setText(tr("Insert motion to all models"));
    m_actionInsertToAllModels->setStatusTip(tr("Insert a motion to the all models."));
    m_actionInsertToAllModels->setShortcut(tr("Ctrl+Shift+V"));
    m_actionInsertToSelectedModel->setText(tr("Insert motion to selected model"));
    m_actionInsertToSelectedModel->setStatusTip(tr("Insert a motion to the selected model."));
    m_actionInsertToSelectedModel->setShortcut(tr("Ctrl+Alt+Shift+V"));
    m_actionSaveMotion->setText(tr("Save motion as VMD"));
    m_actionSaveMotion->setStatusTip(tr("Export bone key frames and face key frames as a VMD."));
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
    m_actionSetCamera->setShortcut(tr("Ctrl+Shift+C"));
    m_actionExit->setText(tr("Exit"));
    m_actionExit->setStatusTip(tr("Exit this application."));
    m_actionExit->setShortcut(QKeySequence::Quit);
    m_actionPlay->setText(tr("Play"));
    m_actionPlay->setStatusTip(tr("Play current scene."));
    m_actionPause->setText(tr("Pause"));
    m_actionPause->setStatusTip(tr("Pause current scene."));
    m_actionStop->setText(tr("Stop"));
    m_actionStop->setStatusTip(tr("Stop current scene."));
    m_actionShowGrid->setText(tr("Show grid"));
    m_actionShowGrid->setStatusTip(tr("Show or hide scene grid."));
    m_actionShowGrid->setShortcut(tr("Ctrl+Shift+G"));
    m_actionShowBones->setText(tr("Show bone wireframe"));
    m_actionShowBones->setStatusTip(tr("Show or hide bone wireframe."));
    m_actionShowBones->setShortcut(tr("Ctrl+Shift+B"));
    m_actionEnablePhysics->setText(tr("Enable physics simulation"));
    m_actionEnablePhysics->setStatusTip(tr("Enable or disable physics simulation using Bullet."));
    m_actionEnablePhysics->setShortcut(tr("Ctrl+Shift+P"));
    m_actionShowModelDialog->setText(tr("Show model dialog"));
    m_actionShowModelDialog->setStatusTip(tr("Show or hide model dialog when the model is loaded."));
    m_actionZoomIn->setText(tr("Zoom in"));
    m_actionZoomIn->setStatusTip(tr("Zoom in the scene."));
    m_actionZoomIn->setShortcut(QKeySequence::ZoomIn);
    m_actionZoomOut->setText(tr("Zoom out"));
    m_actionZoomOut->setStatusTip(tr("Zoom out the scene."));
    m_actionZoomOut->setShortcut(QKeySequence::ZoomOut);
    m_actionRotateUp->setText(tr("Rotate up"));
    m_actionRotateUp->setStatusTip(tr("Rotate up the scene."));
    m_actionRotateUp->setShortcut(tr("Ctrl+Up"));
    m_actionRotateDown->setText(tr("Rotate down"));
    m_actionRotateDown->setStatusTip(tr("Rotate down the scene."));
    m_actionRotateDown->setShortcut(tr("Ctrl+Down"));
    m_actionRotateLeft->setText(tr("Rotate left"));
    m_actionRotateLeft->setStatusTip(tr("Rotate left the scene."));
    m_actionRotateLeft->setShortcut(tr("Ctrl+Left"));
    m_actionRotateRight->setText(tr("Rotate right"));
    m_actionRotateRight->setStatusTip(tr("Rotate right the scene."));
    m_actionRotateRight->setShortcut(tr("Ctrl+Right"));
    m_actionTranslateUp->setText(tr("Translate up"));
    m_actionTranslateUp->setStatusTip(tr("Translate up the scene."));
    m_actionTranslateUp->setShortcut(tr("Shift+Up"));
    m_actionTranslateDown->setText(tr("Translate down"));
    m_actionTranslateDown->setStatusTip(tr("Translate down the scene."));
    m_actionTranslateDown->setShortcut(tr("Shift+Down"));
    m_actionTranslateLeft->setText(tr("Translate left"));
    m_actionTranslateLeft->setStatusTip(tr("Translate left the scene."));
    m_actionTranslateLeft->setShortcut(tr("Shift+Left"));
    m_actionTranslateRight->setText(tr("Translate right"));
    m_actionTranslateRight->setStatusTip(tr("Translate right the scene."));
    m_actionTranslateRight->setShortcut(tr("Shift+Right"));
    m_actionResetCamera->setText(tr("Reset camera"));
    m_actionResetCamera->setStatusTip(tr("Reset camera perspective."));
    m_actionRevertSelectedModel->setText(tr("Revert selected model"));
    m_actionRevertSelectedModel->setStatusTip(tr("Revert the selected model."));
    m_actionDeleteSelectedModel->setText(tr("Delete selected model"));
    m_actionDeleteSelectedModel->setStatusTip(tr("Delete the selected model from the scene."));
    m_actionDeleteSelectedModel->setShortcut(tr("Ctrl+Shift+Backspace"));
    m_actionEdgeOffsetDialog->setText(tr("Open model edge offset dialog"));
    m_actionEdgeOffsetDialog->setStatusTip(tr("Open a dialog to change edge offset of selected model."));
    m_actionEdgeOffsetDialog->setShortcut(tr("Ctrl+Shift+E"));
    m_actionTranslateModelUp->setText(tr("Translate selected model up"));
    m_actionTranslateModelUp->setStatusTip(tr("Translate the selected model up."));
    m_actionTranslateModelUp->setShortcut(tr("Ctrl+Shift+Up"));
    m_actionTranslateModelDown->setText(tr("Translate selected model down"));
    m_actionTranslateModelDown->setStatusTip(tr("Translatethe the selected model down."));
    m_actionTranslateModelDown->setShortcut(tr("Ctrl+Shift+Down"));
    m_actionTranslateModelLeft->setText(tr("Translate selected model left"));
    m_actionTranslateModelLeft->setStatusTip(tr("Translate the selected model left."));
    m_actionTranslateModelLeft->setShortcut(tr("Ctrl+Shift+Left"));
    m_actionTranslateModelRight->setText(tr("Translate selected model right"));
    m_actionTranslateModelRight->setStatusTip(tr("Translate the selected model right."));
    m_actionTranslateModelRight->setShortcut(tr("Ctrl+Shift+Right"));
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
    m_actionRegisterFrame->setShortcut(tr("Ctrl+E"));
    m_actionInsertEmptyFrame->setText(tr("Insert empty keyframe"));
    m_actionInsertEmptyFrame->setStatusTip(tr("Insert an empty keyframe to the selected keyframe."));
    m_actionInsertEmptyFrame->setShortcut(tr("Ctrl+I"));
    m_actionDeleteSelectedFrame->setText(tr("Delete selected keyframe"));
    m_actionDeleteSelectedFrame->setStatusTip(tr("Delete a selected keyframe."));
    m_actionDeleteSelectedFrame->setShortcut(tr("Ctrl+K"));
    m_actionCopy->setText(tr("Copy"));
    m_actionCopy->setStatusTip(tr("Copy a selected keyframe."));
    m_actionCopy->setShortcut(QKeySequence::Copy);
    m_actionPaste->setText(tr("Paste"));
    m_actionPaste->setStatusTip(tr("Paste a selected keyframe."));
    m_actionPaste->setShortcut(QKeySequence::Paste);
    m_actionReversedPaste->setText(tr("Paste with reversed"));
    m_actionReversedPaste->setStatusTip(tr("Paste a selected keyframe with reversed."));
    m_actionReversedPaste->setShortcut(tr("Alt+Ctrl+V"));
    m_actionUndoFrame->setShortcut(QKeySequence::Undo);
    m_actionRedoFrame->setShortcut(QKeySequence::Redo);
    m_actionViewTab->setText(tr("Tab"));
    m_actionViewTab->setStatusTip(tr("Open tab window."));
    m_actionViewTimeline->setText(tr("Timeline"));
    m_actionViewTimeline->setStatusTip(tr("Open timeline window."));
    m_actionViewTransform->setText(tr("Transform"));
    m_actionViewTransform->setStatusTip(tr("Open transform window."));
    m_actionAbout->setText(tr("About"));
    m_actionAbout->setStatusTip(tr("About this application."));
    m_actionAbout->setShortcut(tr("Alt+Q, Alt+/"));
    m_actionAboutQt->setText(tr("About Qt"));
    m_actionAboutQt->setStatusTip(tr("About Qt."));
    m_menuFile->setTitle(tr("&File"));
    m_menuProject->setTitle(tr("&Project"));
    m_menuScene->setTitle(tr("&Scene"));
    m_menuModel->setTitle(tr("&Model"));
    m_menuBone->setTitle(tr("&Bone"));
    m_menuFrame->setTitle(tr("Frame"));
    m_menuView->setTitle(tr("&View"));
    m_menuRetainAssets->setTitle(tr("Select asset"));
    m_menuRetainModels->setTitle(tr("Select model"));
    m_menuHelp->setTitle(tr("&Help"));
}

void MainWindow::connectWidgets()
{
    connect(m_sceneWidget, SIGNAL(modelDidSelect(vpvl::PMDModel*)), m_boneMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(modelWillDelete(vpvl::PMDModel*)), m_boneMotionModel, SLOT(removeModel()));
    connect(m_sceneWidget, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*)), m_boneMotionModel,SLOT(loadMotion(vpvl::VMDMotion*,vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(modelDidMakePose(VPDFile*,vpvl::PMDModel*)), m_timelineTabWidget, SLOT(loadPose(VPDFile*,vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(handleDidMove(int,float)), m_boneMotionModel, SLOT(translate(int,float)));
    connect(m_sceneWidget, SIGNAL(handleDidRotate(int,float)), m_boneMotionModel, SLOT(rotate(int,float)));
    connect(m_transformWidget, SIGNAL(boneDidRegister(vpvl::Bone*)), m_timelineTabWidget, SLOT(addBoneKeyFrameAtCurrentFrameIndex(vpvl::Bone*)));
    connect(m_sceneWidget, SIGNAL(modelDidSelect(vpvl::PMDModel*)), m_faceMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(modelWillDelete(vpvl::PMDModel*)), m_faceMotionModel, SLOT(removeModel()));
    connect(m_sceneWidget, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*)), m_faceMotionModel, SLOT(loadMotion(vpvl::VMDMotion*,vpvl::PMDModel*)));
    connect(m_transformWidget, SIGNAL(faceDidRegister(vpvl::Face*)), m_timelineTabWidget, SLOT(addFaceKeyFrameAtCurrentFrameIndex(vpvl::Face*)));
    connect(m_tabWidget->cameraPerspectiveWidget(), SIGNAL(cameraPerspectiveDidChange(vpvl::Vector3*,vpvl::Vector3*,float*,float*)), m_sceneWidget, SLOT(setCameraPerspective(vpvl::Vector3*,vpvl::Vector3*,float*,float*)));
    connect(m_timelineTabWidget, SIGNAL(currentTabDidChange(int)), m_tabWidget->interpolationWidget(), SLOT(setMode(int)));
    connect(m_sceneWidget, SIGNAL(modelWillDelete(vpvl::PMDModel*)), m_tabWidget->interpolationWidget(), SLOT(disable()));
    connect(m_timelineTabWidget, SIGNAL(motionDidSeek(float)),  m_sceneWidget, SLOT(seekMotion(float)));
    connect(m_boneMotionModel, SIGNAL(motionDidModify(bool)), this, SLOT(setWindowModified(bool)));
    connect(m_faceMotionModel, SIGNAL(motionDidModify(bool)), this, SLOT(setWindowModified(bool)));
    connect(m_boneMotionModel, SIGNAL(bonesDidSelect(QList<vpvl::Bone*>)), m_sceneWidget, SLOT(setBones(QList<vpvl::Bone*>)));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl::PMDModel*)), m_boneMotionModel, SLOT(markAsNew(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl::PMDModel*)), m_faceMotionModel, SLOT(markAsNew(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(assetDidAdd(vpvl::Asset*)), m_tabWidget->assetWidget(), SLOT(addAsset(vpvl::Asset*)));
    connect(m_sceneWidget, SIGNAL(assetWillDelete(vpvl::Asset*)), m_tabWidget->assetWidget(), SLOT(removeAsset(vpvl::Asset*)));
    connect(m_tabWidget->assetWidget(), SIGNAL(assetDidRemove(vpvl::Asset*)), m_sceneWidget, SLOT(deleteAsset(vpvl::Asset*)));
    connect(m_sceneWidget, SIGNAL(modelDidAdd(vpvl::PMDModel*)), m_tabWidget->assetWidget(), SLOT(addModel(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(modelWillDelete(vpvl::PMDModel*)), m_tabWidget->assetWidget(), SLOT(removeModel(vpvl::PMDModel*)));
    connect(m_boneMotionModel, SIGNAL(motionDidUpdate(vpvl::PMDModel*)), m_sceneWidget, SLOT(updateMotion()));
    connect(m_faceMotionModel, SIGNAL(motionDidUpdate(vpvl::PMDModel*)), m_sceneWidget, SLOT(updateMotion()));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl::PMDModel*)), m_timelineTabWidget, SLOT(setCurrentFrameIndexZero()));
    connect(m_sceneWidget, SIGNAL(boneDidSelect(QList<vpvl::Bone*>)), m_boneMotionModel, SLOT(selectBones(QList<vpvl::Bone*>)));
    connect(m_sceneWidget, SIGNAL(globalTransformDidSelect()), m_boneMotionModel, SLOT(setGlobalTransformMode()));
    connect(m_sceneWidget, SIGNAL(localTransformDidSelect()), m_boneMotionModel, SLOT(setLocalTransformMode()));
    connect(m_tabWidget->faceWidget(), SIGNAL(faceDidRegister(vpvl::Face*)), m_timelineTabWidget, SLOT(addFaceKeyFrameAtCurrentFrameIndex(vpvl::Face*)));
    connect(m_sceneWidget, SIGNAL(cameraPerspectiveDidSet(vpvl::Vector3,vpvl::Vector3,float,float)),
            m_tabWidget->cameraPerspectiveWidget(), SLOT(setCameraPerspective(vpvl::Vector3,vpvl::Vector3,float,float)));
    connect(m_sceneWidget, SIGNAL(modelDidAdd(vpvl::PMDModel*)), m_timelineTabWidget, SLOT(notifyCurrentTabIndex()));
    connect(m_boneMotionModel, SIGNAL(boneFramesDidSelect(QList<BoneMotionModel::KeyFramePtr>)),
            m_tabWidget->interpolationWidget(), SLOT(setBoneKeyFrames(QList<BoneMotionModel::KeyFramePtr>)));
    connect(m_sceneWidget, SIGNAL(cameraMotionDidSet(vpvl::VMDMotion*)), m_sceneMotionModel, SLOT(loadMotion(vpvl::VMDMotion*)));
}

void MainWindow::insertMotionToAllModels()
{
    if (maybeSave())
        m_sceneWidget->insertMotionToAllModels();
}

void MainWindow::insertMotionToSelectedModel()
{
    if (maybeSave())
        m_sceneWidget->insertMotionToSelectedModel();
}

void MainWindow::deleteSelectedModel()
{
    if (maybeSave())
        m_sceneWidget->deleteSelectedModel();
}

void MainWindow::saveModelPose()
{
    const QString &filename = openSaveDialog("mainWindow/lastVPDDirectory",
                                             tr("Save model pose as a VPD file"),
                                             tr("VPD file (*.vpd)"));
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

void MainWindow::resetBoneX()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kX);
    }
    else {
        QMessageBox::warning(this, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to reset X position of the bone"));
    }
}

void MainWindow::resetBoneY()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kY);
    }
    else {
        QMessageBox::warning(this, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to reset Y position of the bone"));
    }
}

void MainWindow::resetBoneZ()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kZ);
    }
    else {
        QMessageBox::warning(this, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to reset Z position of the bone"));
    }
}

void MainWindow::resetBoneRotation()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kRotation);
    }
    else {
        QMessageBox::warning(this, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to reset rotation of the bone"));
    }
}

void MainWindow::resetAllBones()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetAllBones();
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."), tr("Select a model to reset bones"));
    }
}

void MainWindow::openBoneDialog()
{
    if (m_boneMotionModel->isBoneSelected()) {
        BoneDialog *dialog = new BoneDialog(m_boneMotionModel, this);
        dialog->exec();
    }
    else {
        QMessageBox::warning(this, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to open this dialog"));
    }
}

void MainWindow::saveAssetMetadata()
{
    m_sceneWidget->saveMetadataFromAsset(m_tabWidget->assetWidget()->currentAsset());
}

void MainWindow::exportImage()
{
    const QString &filename = openSaveDialog("mainWindow/lastImageDirectory",
                                             tr("Export scene as image"),
                                             tr("Image (*bmp, *.jpg, *.png)"));
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
#ifdef OPENCV_FOUND
    if (m_sceneWidget->scene()->maxFrameIndex() > 0) {
        delete m_exportingVideoDialog;
        m_exportingVideoDialog = new ExportVideoDialog(this, m_sceneWidget);
        m_exportingVideoDialog->show();
    }
    else {
        QMessageBox::warning(this, tr("No motion to export."),
                             tr("Create or load a motion."));
    }
#else
    QMessageBox::warning(this, tr("Exporting video feature is not supported."),
                         tr("Exporting video is disabled because OpenCV is not linked."));
#endif
}

void MainWindow::startExportingVideo()
{
#ifdef OPENCV_FOUND
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
                                             tr("Export scene as video"),
                                             tr("Video (*.avi)"));
    if (!filename.isEmpty()) {
        QProgressDialog *progress = new QProgressDialog(this);
        progress->setCancelButtonText(tr("Cancel"));
        progress->setWindowModality(Qt::WindowModal);
#ifdef Q_OS_MACX
        int fourcc = CV_FOURCC('p', 'n', 'g', ' ');
#else
        int fourcc = CV_FOURCC('D', 'I', 'B', ' ');
#endif
        int fps = m_sceneWidget->preferredFPS();
        int width = m_exportingVideoDialog->sceneWidth();
        int height = m_exportingVideoDialog->sceneHeight();
        cv::VideoWriter writer(filename.toUtf8().constData(), fourcc, 29.97, cv::Size(width, height));
        m_sceneWidget->setPreferredFPS(30);
        if (writer.isOpened()) {
            const vpvl::Scene *scene = m_sceneWidget->scene();
            const QString &format = tr("Exporting frame %1 of %2...");
            int maxRangeIndex = toIndex - fromIndex;
            progress->setRange(0, maxRangeIndex);
            const QRect mainGeomtry = geometry();
            const QSize minSize = minimumSize(), maxSize = maximumSize(),
                    videoSize = QSize(width, height), sceneSize = m_sceneWidget->size();
            QSizePolicy policy = sizePolicy();
            int handleWidth = m_mainSplitter->handleWidth();
            m_leftSplitter->hide();
            m_mainSplitter->setHandleWidth(0);
            statusBar()->hide();
            resize(videoSize);
            setMinimumSize(videoSize);
            setMaximumSize(videoSize);
            adjustSize();
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            vpvl::PMDModel *selected = m_sceneWidget->selectedModel();
            bool visibleGrid = m_sceneWidget->isGridVisible();
            m_sceneWidget->stop();
            m_sceneWidget->seekMotion(0.0f);
            m_sceneWidget->advanceMotion(fromIndex);
            m_sceneWidget->setGridVisible(m_exportingVideoDialog->includesGrid());
            m_sceneWidget->setHandlesVisible(false);
            m_sceneWidget->setInfoPanelVisible(false);
            m_sceneWidget->setSelectedModel(0);
            m_sceneWidget->resize(videoSize);
            m_sceneWidget->updateGL();
            progress->setLabelText(format.arg(0).arg(maxRangeIndex));
            while (!scene->isMotionReachedTo(toIndex)) {
                if (progress->wasCanceled())
                    break;
                QImage image = m_sceneWidget->grabFrameBuffer();
                if (image.width() != width || image.height() != height)
                    image = image.scaled(width, height);
                cv::Mat mat(height, width, CV_8UC4, image.bits(), image.bytesPerLine());
                cv::Mat mat2(mat.rows, mat.cols, CV_8UC3);
                int fromTo[] = { 0, 0, 1, 1, 2, 2 };
                cv::mixChannels(&mat, 1, &mat2, 1, fromTo, 3);
                writer << mat2;
                int value = progress->value() + 1;
                progress->setValue(value);
                progress->setLabelText(format.arg(value).arg(maxRangeIndex));
                m_sceneWidget->advanceMotion(1.0f);
                m_sceneWidget->resize(videoSize);
            }
            m_sceneWidget->setGridVisible(visibleGrid);
            m_sceneWidget->setHandlesVisible(true);
            m_sceneWidget->setInfoPanelVisible(true);
            m_sceneWidget->setSelectedModel(selected);
            m_sceneWidget->setPreferredFPS(fps);
            m_sceneWidget->resize(sceneSize);
            m_mainSplitter->setHandleWidth(handleWidth);
            m_leftSplitter->show();
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
#endif
}

void MainWindow::addNewMotion()
{
    if (maybeSave()) {
        vpvl::PMDModel *model = m_sceneWidget->selectedModel();
        m_sceneWidget->deleteMotion(m_boneMotionModel->currentMotion(), model);
        m_boneMotionModel->removeMotion();
        m_faceMotionModel->removeMotion();
        m_sceneMotionModel->removeMotion();
        m_sceneWidget->setEmptyMotion(model);
        m_boneMotionModel->markAsNew(model);
        m_faceMotionModel->markAsNew(model);
        m_sceneMotionModel->setModified(false);
        m_sceneWidget->deleteCameraMotion();
    }
}

void MainWindow::openEdgeOffsetDialog()
{
    if (m_sceneWidget->selectedModel()) {
        EdgeOffsetDialog *dialog = new EdgeOffsetDialog(this, m_sceneWidget);
        dialog->exec();
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."),
                             tr("Select a model to chage edge offset value."));
    }
}

const QString MainWindow::openSaveDialog(const QString &name, const QString &desc, const QString &exts)
{
    const QString path = m_settings.value(name).toString();
    const QString fileName = QFileDialog::getSaveFileName(this, desc, path, exts);
    if (!fileName.isEmpty()) {
        QDir dir(fileName);
        dir.cdUp();
        m_settings.setValue(name, dir.absolutePath());
    }
    return fileName;
}

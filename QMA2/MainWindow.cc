#include "MainWindow.h"

#include "AssetWidget.h"
#include "BoneDialog.h"
#include "BoneMotionModel.h"
#include "CameraPerspectiveWidget.h"
#include "FaceMotionModel.h"
#include "FaceWidget.h"
#include "InterpolationWidget.h"
#include "LicenseWidget.h"
#include "SceneWidget.h"
#include "TabWidget.h"
#include "TimelineTabWidget.h"
#include "TransformWidget.h"
#include "VPDFile.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

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
    m_model(0),
    m_bone(0),
    m_position(0.0f, 0.0f, 0.0f),
    m_angle(0.0f, 0.0f, 0.0f),
    m_fovy(0.0f),
    m_distance(0.0f),
    m_currentFPS(0)
{
    m_undo = new QUndoGroup(this);
    m_licenseWidget = new LicenseWidget();
    m_sceneWidget = new SceneWidget(&m_settings);
    m_boneMotionModel = new BoneMotionModel(m_undo, m_sceneWidget, this);
    m_faceMotionModel = new FaceMotionModel(m_undo, this);
    m_tabWidget = new TabWidget(&m_settings, m_boneMotionModel, m_faceMotionModel);
    m_timelineTabWidget = new TimelineTabWidget(&m_settings, m_boneMotionModel, m_faceMotionModel);
    m_transformWidget = new TransformWidget(&m_settings, m_boneMotionModel, m_faceMotionModel);
    buildUI();
    connectWidgets();
    restoreGeometry(m_settings.value("mainWindow/geometry").toByteArray());
    restoreState(m_settings.value("mainWindow/state").toByteArray());
    setCentralWidget(m_sceneWidget);
    setWindowTitle(buildWindowTitle());
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
        m_boneMotionModel->deleteMotion();
        m_faceMotionModel->deleteMotion();
    }
}

bool MainWindow::save()
{
    return saveAs();
}

bool MainWindow::saveAs()
{
    const QString name = "mainWindow/lastVMDDirectory";
    const QString path = m_settings.value(name).toString();
    const QString filename = QFileDialog::getSaveFileName(this,
                                                          tr("Save Motion as a VMD file"),
                                                          path,
                                                          tr("VMD file (*.vmd)"));
    bool ret = false;
    if (!filename.isEmpty()) {
        ret = saveFile(filename);
        QDir dir(filename);
        dir.cdUp();
        m_settings.setValue(name, dir.absolutePath());
    }
    return ret;
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
    QString name(asset->name());
    QAction *action = new QAction(name, this);
    action->setStatusTip(tr("Select an asset %1").arg(name));
    //connect(action, SIGNAL(triggered()), this, SLOT(selectCurrentModel()));
    m_menuRetainAssets->addAction(action);
}

void MainWindow::deleteAsset(vpvl::Asset *asset)
{
    QAction *actionToRemove = 0;
    QString name(asset->name());
    foreach (QAction *action, m_menuRetainAssets->actions()) {
        if (action->text() == name) {
            actionToRemove = action;
            break;
        }
    }
    if (actionToRemove)
        m_menuRetainAssets->removeAction(actionToRemove);
}

void MainWindow::setCurrentFPS(int value)
{
    m_currentFPS = value;
    updateInformation();
}

void MainWindow::setModel(vpvl::PMDModel *value)
{
    m_model = value;
    updateInformation();
}

void MainWindow::setBones(const QList<vpvl::Bone *> &bones)
{
    m_bone = bones.isEmpty() ? 0 : bones.last();
    updateInformation();
}

void MainWindow::setCameraPerspective(const vpvl::Vector3 &pos,
                                      const vpvl::Vector3 &angle,
                                      float fovy,
                                      float distance)
{
    m_position = pos;
    m_angle = angle;
    m_fovy = fovy;
    m_distance = distance;
    updateInformation();
}

void MainWindow::updateInformation()
{
    setWindowTitle(buildWindowTitle());
}

void MainWindow::buildUI()
{

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
    m_actionSaveMotion = new QAction(this);
    connect(m_actionSaveMotion, SIGNAL(triggered()), this, SLOT(saveAs()));
    m_actionExit = new QAction(this);
    m_actionExit->setMenuRole(QAction::QuitRole);
    connect(m_actionExit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    m_actionPlay = new QAction(this);
    connect(m_actionPlay, SIGNAL(triggered()), m_sceneWidget, SLOT(play()));
    m_actionPause = new QAction(this);
    connect(m_actionPause, SIGNAL(triggered()), m_sceneWidget, SLOT(pause()));
    m_actionStop = new QAction(this);
    connect(m_actionStop, SIGNAL(triggered()), m_sceneWidget, SLOT(stop()));

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
    m_actionShowBones = new QAction(this);

    m_actionRevertSelectedModel = new QAction(this);
    connect(m_actionRevertSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(revertSelectedModel()));
    m_actionDeleteSelectedModel = new QAction(this);
    connect(m_actionDeleteSelectedModel, SIGNAL(triggered()), m_sceneWidget, SLOT(deleteSelectedModel()));

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
    m_menuFile->addAction(m_actionInsertToAllModels);
    m_menuFile->addAction(m_actionInsertToSelectedModel);
    m_menuFile->addAction(m_actionSetCamera);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionLoadModelPose);
    m_menuFile->addAction(m_actionSaveModelPose);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionSaveMotion);
    m_menuFile->addSeparator();
    m_menuFile->addAction(m_actionExit);
    m_menuBar->addMenu(m_menuFile);
    m_menuProject = new QMenu(this);
    m_menuProject->addAction(m_actionPlay);
    m_menuProject->addAction(m_actionPause);
    m_menuProject->addAction(m_actionStop);
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
    m_menuFrame->addAction(m_actionInsertEmptyFrame);
    m_menuFrame->addAction(m_actionDeleteSelectedFrame);
    m_menuFrame->addSeparator();
    m_menuFrame->addAction(m_actionUndoFrame);
    m_menuFrame->addAction(m_actionRedoFrame);
    m_menuBar->addMenu(m_menuFrame);
    m_menuView = new QMenu(this);
    m_menuView->addAction(m_actionViewTab);
    m_menuView->addAction(m_actionViewTimeline);
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
    connect(m_sceneWidget, SIGNAL(fpsDidUpdate(int)), this, SLOT(updateFPS(int)));

    bool visibleTabs = m_settings.value("mainWindow/visibleTabs", QVariant(false)).toBool();
    bool visibleTimeline = m_settings.value("mainWindow/visibleTimeline", QVariant(false)).toBool();
    bool visibleTransform = m_settings.value("mainWindow/visibleTransform", QVariant(false)).toBool();
    m_tabWidget->setVisible(visibleTabs);
    m_timelineTabWidget->setVisible(visibleTimeline);
    m_transformWidget->setVisible(visibleTransform);

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
    m_actionInsertToAllModels->setText(tr("Insert to all models"));
    m_actionInsertToAllModels->setStatusTip(tr("Insert a motion to the all models."));
    m_actionInsertToAllModels->setShortcut(tr("Ctrl+Shift+V"));
    m_actionInsertToSelectedModel->setText(tr("Insert to selected model"));
    m_actionInsertToSelectedModel->setStatusTip(tr("Insert a motion to the selected model."));
    m_actionInsertToSelectedModel->setShortcut(tr("Ctrl+Alt+Shift+V"));
    m_actionSaveMotion->setText(tr("Save motion as VMD"));
    m_actionSaveMotion->setStatusTip(tr("Export bone key frames and face key frames as a VMD."));
    m_actionLoadModelPose->setText(tr("Load model pose"));
    m_actionLoadModelPose->setStatusTip(tr("Load a model pose to the selected model."));
    m_actionSaveModelPose->setText(tr("Save model pose"));
    m_actionSaveModelPose->setStatusTip(tr("Save selected frame bones as a model pose file."));
    m_actionSetCamera->setText(tr("Set camera motion"));
    m_actionSetCamera->setStatusTip(tr("Set a camera motion to the scene."));
    m_actionSetCamera->setShortcut(tr("Ctrl+Shift+C"));
    m_actionExit->setText(tr("Exit"));
    m_actionExit->setStatusTip(tr("Exit this application."));
    m_actionExit->setShortcut(tr("Ctrl+Q"));
    m_actionPlay->setText(tr("Play"));
    m_actionPlay->setStatusTip(tr("Play current scene."));
    m_actionPause->setText(tr("Pause"));
    m_actionPause->setStatusTip(tr("Pause current scene."));
    m_actionStop->setText(tr("Stop"));
    m_actionStop->setStatusTip(tr("Stop current scene."));
    m_actionZoomIn->setText(tr("Zoom in"));
    m_actionZoomIn->setStatusTip(tr("Zoom in the scene."));
    m_actionZoomIn->setShortcut(tr("+"));
    m_actionZoomOut->setText(tr("Zoom out"));
    m_actionZoomOut->setStatusTip(tr("Zoom out the scene."));
    m_actionZoomOut->setShortcut(tr("-"));
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
    m_actionInsertEmptyFrame->setText(tr("Insert empty keyframe"));
    m_actionInsertEmptyFrame->setStatusTip(tr("Insert an empty keyframe to the selected keyframe."));
    m_actionDeleteSelectedFrame->setText(tr("Delete selected keyframe"));
    m_actionDeleteSelectedFrame->setStatusTip(tr("Delete a selected keyframe."));
    m_actionUndoFrame->setShortcut(tr("Ctrl+Z"));
    m_actionRedoFrame->setShortcut(tr("Ctrl+Shift+Z"));
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
    connect(m_sceneWidget, SIGNAL(modelWillDelete(vpvl::PMDModel*)), m_boneMotionModel, SLOT(deleteModel()));
    connect(m_sceneWidget, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*)), m_boneMotionModel,SLOT(loadMotion(vpvl::VMDMotion*,vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(modelDidMakePose(VPDFile*,vpvl::PMDModel*)), m_timelineTabWidget, SLOT(loadPose(VPDFile*,vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(handleDidMove(int,float)), m_boneMotionModel, SLOT(translate(int,float)));
    connect(m_sceneWidget, SIGNAL(handleDidRotate(int,float)), m_boneMotionModel, SLOT(rotate(int,float)));
    connect(m_transformWidget, SIGNAL(boneDidRegister(vpvl::Bone*)), m_timelineTabWidget, SLOT(setFrameAtCurrentIndex(vpvl::Bone*)));
    connect(m_transformWidget, SIGNAL(bonesDidSelect(QList<vpvl::Bone*>)), m_sceneWidget, SLOT(setBones(QList<vpvl::Bone*>)));
    connect(m_sceneWidget, SIGNAL(modelDidSelect(vpvl::PMDModel*)), m_faceMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(modelWillDelete(vpvl::PMDModel*)), m_faceMotionModel, SLOT(deleteModel()));
    connect(m_sceneWidget, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*)), m_faceMotionModel, SLOT(loadMotion(vpvl::VMDMotion*,vpvl::PMDModel*)));
    connect(m_transformWidget, SIGNAL(faceDidRegister(vpvl::Face*)), m_timelineTabWidget, SLOT(setFrameAtCurrentIndex(vpvl::Face*)));
    connect(m_sceneWidget, SIGNAL(fpsDidUpdate(int)), this, SLOT(setCurrentFPS(int)));
    connect(m_sceneWidget, SIGNAL(cameraPerspectiveDidSet(vpvl::Vector3,vpvl::Vector3,float,float)), this, SLOT(setCameraPerspective(vpvl::Vector3,vpvl::Vector3,float,float)));
    connect(m_tabWidget->cameraPerspectiveWidget(), SIGNAL(cameraPerspectiveDidChange(vpvl::Vector3*,vpvl::Vector3*,float*,float*)), m_sceneWidget, SLOT(setCameraPerspective(vpvl::Vector3*,vpvl::Vector3*,float*,float*)));
    //connect(m_timelineTabWidget, SIGNAL(currentTabDidChange(QString)), m_tabWidget->interpolationWidget(), SLOT(setMode(QString)));
    //connect(m_sceneWidget, SIGNAL(modelDidDelete(vpvl::PMDModel*)), m_tabWidget->interpolationWidget(), SLOT(disable()));
    connect(m_timelineTabWidget, SIGNAL(motionDidSeek(float)),  m_sceneWidget, SLOT(seekMotion(float)));
    connect(m_boneMotionModel, SIGNAL(motionDidModify(bool)), this, SLOT(setWindowModified(bool)));
    connect(m_faceMotionModel, SIGNAL(motionDidModify(bool)), this, SLOT(setWindowModified(bool)));
    connect(m_transformWidget, SIGNAL(bonesDidSelect(QList<vpvl::Bone*>)), m_sceneWidget, SLOT(setBones(QList<vpvl::Bone*>)));
    connect(m_transformWidget, SIGNAL(bonesDidSelect(QList<vpvl::Bone*>)), this, SLOT(setBones(QList<vpvl::Bone*>)));
    connect(m_sceneWidget, SIGNAL(sceneDidPlay()), this, SLOT(startSceneUpdate()));
    connect(m_sceneWidget, SIGNAL(sceneDidPause()), this, SLOT(stopSceneUpdate()));
    connect(m_sceneWidget, SIGNAL(sceneDidStop()), this, SLOT(stopSceneUpdate()));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl::PMDModel*)), m_boneMotionModel, SLOT(markAsNew(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(newMotionDidSet(vpvl::PMDModel*)), m_faceMotionModel, SLOT(markAsNew(vpvl::PMDModel*)));
    connect(m_sceneWidget, SIGNAL(assetDidAdd(vpvl::Asset*)), m_tabWidget->assetWidget(), SLOT(addAsset(vpvl::Asset*)));
    connect(m_sceneWidget, SIGNAL(assetWillDelete(vpvl::Asset*)), m_tabWidget->assetWidget(), SLOT(removeAsset(vpvl::Asset*)));
    connect(m_tabWidget->assetWidget(), SIGNAL(assetDidRemove(vpvl::Asset*)), m_sceneWidget, SLOT(deleteAsset(vpvl::Asset*)));
}

void MainWindow::startSceneUpdate()
{
    setWindowTitle(buildWindowTitle(0));
}

void MainWindow::stopSceneUpdate()
{
    setWindowTitle(buildWindowTitle());
}

void MainWindow::updateFPS(int fps)
{
    setWindowTitle(buildWindowTitle(fps));
}

const QString MainWindow::buildWindowTitle()
{
    QString title = qAppName();
    if (m_model)
        title += " - " + internal::toQString(m_model);
    if (m_bone)
        title += " - " + internal::toQString(m_bone);
    return title + "[*]";
}

const QString MainWindow::buildWindowTitle(int fps)
{
    return buildWindowTitle() + tr(": Rendering Scene (FPS: %1)").arg(fps);
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
    const QString name = "mainWindow/lastVPDDirectory";
    const QString path = m_settings.value(name).toString();
    const QString filename = QFileDialog::getSaveFileName(this,
                                                          tr("Save model pose as a VPD file"),
                                                          path,
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
        QDir dir(filename);
        dir.cdUp();
        m_settings.setValue(name, dir.absolutePath());
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

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "BoneDialog.h"
#include "BoneMotionModel.h"
#include "CameraPerspectiveWidget.h"
#include "FaceMotionModel.h"
#include "FaceWidget.h"
#include "InterpolationWidget.h"
#include "LicenseWidget.h"
#include "PlayerWidget.h"
#include "TabWidget.h"
#include "TimelineTabWidget.h"
#include "TransformWidget.h"
#include "VPDFile.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qAppName()),
    m_undo(0),
    m_licenseWidget(0),
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
    m_boneMotionModel = new BoneMotionModel(m_undo, this);
    m_faceMotionModel = new FaceMotionModel(m_undo, this);
    m_licenseWidget = new LicenseWidget();
    m_tabWidget = new TabWidget(&m_settings, m_boneMotionModel, m_faceMotionModel);
    m_timelineTabWidget = new TimelineTabWidget(&m_settings, m_boneMotionModel, m_faceMotionModel);
    m_transformWidget = new TransformWidget(&m_settings, m_boneMotionModel, m_faceMotionModel);
    buildUI();
    connectWidgets();
    restoreGeometry(m_settings.value("mainWindow/geometry").toByteArray());
    restoreState(m_settings.value("mainWindow/state").toByteArray());
}

MainWindow::~MainWindow()
{
    delete m_licenseWidget;
    delete m_tabWidget;
    delete m_timelineTabWidget;
    delete m_transformWidget;
    /* for QMenu limitation see http://doc.qt.nokia.com/latest/mac-differences.html#menu-actions */
#ifdef Q_OS_MACX
    delete ui->menuBar;
#endif
    delete ui;
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
    if (action) {
        vpvl::PMDModel *model = ui->scene->findModel(action->text());
        ui->scene->setSelectedModel(model);
    }
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        ui->scene->setEmptyMotion();
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
    ui->scene->setSelectedModel(0);
}

void MainWindow::addModel(vpvl::PMDModel *model)
{
    QString name = internal::toQString(model);
    QAction *action = new QAction(name, this);
    action->setStatusTip(tr("Select a model %1").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(selectModel()));
    ui->menuSelectModel->addAction(action);
}

void MainWindow::deleteModel(vpvl::PMDModel *model)
{
    QAction *actionToRemove = 0;
    QString name = internal::toQString(model);
    foreach (QAction *action, ui->menuSelectModel->actions()) {
        if (action->text() == name) {
            actionToRemove = action;
            break;
        }
    }
    if (actionToRemove)
        ui->menuSelectModel->removeAction(actionToRemove);
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

void MainWindow::setBone(vpvl::Bone *value)
{
    m_bone = value;
    updateInformation();
}

void MainWindow::setCameraPerspective(const btVector3 &pos,
                                      const btVector3 &angle,
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
}

void MainWindow::buildUI()
{
    ui->setupUi(this);
    ui->scene->setSettings(&m_settings);
    /* for QMenu limitation see http://doc.qt.nokia.com/latest/mac-differences.html#menu-actions */
#ifdef Q_OS_MACX
    ui->menuBar->setParent(0);
#endif
    QMenu *menuFrame = ui->menuFrame;
    menuFrame->addSeparator();
    QAction *undoAction = m_undo->createUndoAction(this);
    undoAction->setShortcut(tr("Ctrl+Z"));
    menuFrame->addAction(undoAction);
    QAction *redoAction = m_undo->createRedoAction(this);
    redoAction->setShortcut(tr("Ctrl+Shift+Z"));
    menuFrame->addAction(redoAction);
    bool visibleTabs = m_settings.value("mainWindow/visibleTabs", QVariant(false)).toBool();
    bool visibleTimeline = m_settings.value("mainWindow/visibleTimeline", QVariant(false)).toBool();
    bool visibleTransform = m_settings.value("mainWindow/visibleTransform", QVariant(false)).toBool();
    ui->actionTabs->setChecked(visibleTabs);
    ui->actionTimeline->setChecked(visibleTimeline);
    ui->actionTransform->setChecked(visibleTransform);
    m_tabWidget->setVisible(visibleTabs);
    m_timelineTabWidget->setVisible(visibleTimeline);
    m_transformWidget->setVisible(visibleTransform);
}

void MainWindow::connectWidgets()
{
    connect(ui->scene, SIGNAL(modelDidAdd(vpvl::PMDModel*)),
            this, SLOT(addModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(modelDidDelete(vpvl::PMDModel*)),
            this, SLOT(deleteModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            m_boneMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(modelDidDelete(vpvl::PMDModel*)),
            m_boneMotionModel, SLOT(deleteModel()));
    connect(ui->scene, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*)),
            m_boneMotionModel,SLOT(loadMotion(vpvl::VMDMotion*,vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(modelDidMakePose(VPDFile*,vpvl::PMDModel*)),
            m_timelineTabWidget, SLOT(loadPose(VPDFile*,vpvl::PMDModel*)));
    connect(m_transformWidget, SIGNAL(boneDidRegister(vpvl::Bone*)),
            m_timelineTabWidget, SLOT(setFrameAtCurrentIndex(vpvl::Bone*)));
    connect(ui->scene, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            m_faceMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(modelDidDelete(vpvl::PMDModel*)),
            m_faceMotionModel, SLOT(deleteModel()));
    connect(ui->scene, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*)),
            m_faceMotionModel, SLOT(loadMotion(vpvl::VMDMotion*,vpvl::PMDModel*)));
    connect(m_transformWidget, SIGNAL(faceDidRegister(vpvl::Face*)),
            m_timelineTabWidget, SLOT(setFrameAtCurrentIndex(vpvl::Face*)));
    connect(ui->scene, SIGNAL(fpsDidUpdate(int)),
            this, SLOT(setCurrentFPS(int)));
    connect(ui->scene, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            this, SLOT(setModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(cameraPerspectiveDidSet(btVector3,btVector3,float,float)),
            this, SLOT(setCameraPerspective(btVector3,btVector3,float,float)));
    connect(m_tabWidget->cameraPerspectiveWidget(), SIGNAL(cameraPerspectiveDidChange(btVector3*,btVector3*,float*,float*)),
            ui->scene, SLOT(setCameraPerspective(btVector3*,btVector3*,float*,float*)));
    //connect(m_timelineTabWidget, SIGNAL(currentTabDidChange(QString)),
    //        m_tabWidget->interpolationWidget(), SLOT(setMode(QString)));
    //connect(ui->scene, SIGNAL(modelDidDelete(vpvl::PMDModel*)),
    //        m_tabWidget->interpolationWidget(), SLOT(disable()));
    connect(m_timelineTabWidget, SIGNAL(motionDidSeek(float)),
            ui->scene, SLOT(seekMotion(float)));
    connect(m_boneMotionModel, SIGNAL(motionDidModify(bool)),
            this, SLOT(setWindowModified(bool)));
    connect(m_faceMotionModel, SIGNAL(motionDidModify(bool)),
            this, SLOT(setWindowModified(bool)));
    connect(m_transformWidget, SIGNAL(bonesDidSelect(QList<vpvl::Bone*>)),
            ui->scene, SLOT(setBones(QList<vpvl::Bone*>)));
}

void MainWindow::on_actionAbout_triggered()
{
    m_licenseWidget->show();
}

void MainWindow::on_actionAboutQt_triggered()
{
    qApp->aboutQt();
}

void MainWindow::on_actionAddModel_triggered()
{
    ui->scene->addModel();
}

void MainWindow::on_actionAddAsset_triggered()
{
    ui->scene->addAsset();
}

void MainWindow::on_actionInsertToAllModels_triggered()
{
    if (maybeSave())
        ui->scene->insertMotionToAllModels();
}

void MainWindow::on_actionInsertToSelectedModel_triggered()
{
    if (maybeSave())
        ui->scene->insertMotionToSelectedModel();
}

void MainWindow::on_actionSetCamera_triggered()
{
    ui->scene->setCamera();
}

void MainWindow::on_actionExit_triggered()
{
    qApp->closeAllWindows();
}

void MainWindow::on_actionZoomIn_triggered()
{
    ui->scene->zoomIn();
}

void MainWindow::on_actionZoomOut_triggered()
{
    ui->scene->zoomOut();
}

void MainWindow::on_actionRotateUp_triggered()
{
    ui->scene->rotateUp();
}

void MainWindow::on_actionRotateDown_triggered()
{
    ui->scene->rotateDown();
}

void MainWindow::on_actionRotateLeft_triggered()
{
    ui->scene->rotateLeft();
}

void MainWindow::on_actionRotateRight_triggered()
{
    ui->scene->rotateRight();
}

void MainWindow::on_actionTranslateUp_triggered()
{
    ui->scene->translateUp();
}

void MainWindow::on_actionTranslateDown_triggered()
{
    ui->scene->translateDown();
}

void MainWindow::on_actionTranslateLeft_triggered()
{
    ui->scene->translateLeft();
}

void MainWindow::on_actionTranslateRight_triggered()
{
    ui->scene->translateRight();
}

void MainWindow::on_actionResetCamera_triggered()
{
    ui->scene->resetCamera();
}

void MainWindow::on_actionRevertSelectedModel_triggered()
{
    ui->scene->setSelectedModel(0);
}

void MainWindow::on_actionDeleteSelectedModel_triggered()
{
    if (maybeSave())
        ui->scene->deleteSelectedModel();
}

void MainWindow::on_actionLoadModelPose_triggered()
{
    ui->scene->setModelPose();
}

void MainWindow::on_actionSaveModelPose_triggered()
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
            m_timelineTabWidget->savePose(&pose, ui->scene->selectedModel());
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

void MainWindow::on_actionBoneXPositionZero_triggered()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kX);
    }
    else {
        QMessageBox::warning(this, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to reset X position of the bone"));
    }
}

void MainWindow::on_actionBoneYPositionZero_triggered()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kY);
    }
    else {
            QMessageBox::warning(this, tr("The model or the bone is not selected."),
                                 tr("Select a model or a bone to reset Y position of the bone"));
    }
}

void MainWindow::on_actionBoneZPositionZero_triggered()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kZ);
    }
    else {
        QMessageBox::warning(this, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to reset Z position of the bone"));
    }
}

void MainWindow::on_actionBoneRotationZero_triggered()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kRotation);
    }
    else {
        QMessageBox::warning(this, tr("The model or the bone is not selected."),
                                       tr("Select a model or a bone to reset rotation of the bone"));
    }
}

void MainWindow::on_actionBoneResetAll_triggered()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetAllBones();
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."), tr("Select a model to reset bones"));
    }
}

void MainWindow::on_actionTimeline_triggered(bool value)
{
    m_timelineTabWidget->setVisible(value);
}

void MainWindow::on_actionTransform_triggered(bool value)
{
    m_transformWidget->setVisible(value);
}

void MainWindow::on_actionTabs_triggered(bool value)
{
    m_tabWidget->setVisible(value);
}

void MainWindow::on_actionBoneDialog_triggered()
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

void MainWindow::on_actionExportVMD_triggered()
{
    saveAs();
}

void MainWindow::on_actionNewMotion_triggered()
{
    newFile();
}

void MainWindow::on_actionInsertEmptyFrame_triggered()
{
    m_timelineTabWidget->insertFrame();
}

void MainWindow::on_actionDeleteSelectedFrame_triggered()
{
    m_timelineTabWidget->deleteFrame();
}

void MainWindow::on_actionShowBones_triggered(bool value)
{
    ui->scene->setDisplayBones(value);
}

void MainWindow::on_actionPlay_triggered()
{
    ui->scene->play();
}

void MainWindow::on_actionStop_triggered()
{
    ui->scene->stop();
}

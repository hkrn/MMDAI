#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "BoneMotionModel.h"
#include "CameraPerspectiveWidget.h"
#include "FaceMotionModel.h"
#include "FaceWidget.h"
#include "TabWidget.h"
#include "TimelineWidget.h"
#include "TransformWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings(QSettings::IniFormat, QSettings::UserScope, "MMDAI", "QMA2"),
    m_model(0),
    m_bone(0),
    m_position(0.0f, 0.0f, 0.0f),
    m_angle(0.0f, 0.0f, 0.0f),
    m_fovy(0.0f),
    m_distance(0.0f),
    m_currentFPS(0)
{
    m_boneMotionModel = new BoneMotionModel(this);
    m_faceMotionModel = new FaceMotionModel(this);
    m_tabWidget = new TabWidget(&m_settings);
    m_timelineWidget = new TimelineWidget(&m_settings, m_boneMotionModel, m_faceMotionModel);
    m_transformWidget = new TransformWidget(&m_settings, m_boneMotionModel, m_faceMotionModel);
    ui->setupUi(this);
    ui->scene->setSettings(&m_settings);
    /* for QMenu limitation see http://doc.qt.nokia.com/latest/mac-differences.html#menu-actions */
#ifdef Q_OS_MACX
    ui->menuBar->setParent(0);
#endif
    connectWidgets();
    restoreGeometry(m_settings.value("mainWindow/geometry").toByteArray());
    restoreState(m_settings.value("mainWindow/state").toByteArray());
}

MainWindow::~MainWindow()
{
    delete m_tabWidget;
    delete m_timelineWidget;
    delete m_transformWidget;
    /* for QMenu limitation see http://doc.qt.nokia.com/latest/mac-differences.html#menu-actions */
#ifdef Q_OS_MACX
    delete ui->menuBar;
#endif
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_settings.setValue("mainWindow/geometry", saveGeometry());
    m_settings.setValue("mainWindow/state", saveState());
    event->accept();
}

void MainWindow::selectModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        vpvl::PMDModel *model = ui->scene->models().value(action->text());
        ui->scene->setSelectedModel(model);
    }
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

void MainWindow::connectWidgets()
{
    connect(ui->scene, SIGNAL(modelDidAdd(vpvl::PMDModel*)),
            this, SLOT(addModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(modelDidDelete(vpvl::PMDModel*)),
            this, SLOT(deleteModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            m_boneMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*)),
            m_boneMotionModel,SLOT(loadMotion(vpvl::VMDMotion*,vpvl::PMDModel*)));
    connect(m_transformWidget, SIGNAL(boneDidRegister(vpvl::Bone*)),
            m_timelineWidget, SLOT(registerKeyFrame(vpvl::Bone*)));
    connect(ui->scene, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            m_faceMotionModel, SLOT(setPMDModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(motionDidAdd(vpvl::VMDMotion*,vpvl::PMDModel*)),
            m_faceMotionModel, SLOT(loadMotion(vpvl::VMDMotion*,vpvl::PMDModel*)));
    connect(m_transformWidget, SIGNAL(faceDidRegister(vpvl::Face*)),
            m_timelineWidget, SLOT(registerKeyFrame(vpvl::Face*)));
    connect(ui->scene, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            m_tabWidget->faceWidget(), SLOT(setModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(fpsDidUpdate(int)),
            this, SLOT(setCurrentFPS(int)));
    connect(ui->scene, SIGNAL(modelDidSelect(vpvl::PMDModel*)),
            this, SLOT(setModel(vpvl::PMDModel*)));
    connect(ui->scene, SIGNAL(cameraPerspectiveDidSet(btVector3,btVector3,float,float)),
            this, SLOT(setCameraPerspective(btVector3,btVector3,float,float)));
    connect(m_tabWidget->cameraPerspectiveWidget(), SIGNAL(cameraPerspectiveDidChange(btVector3*,btVector3*,float*,float*)),
            ui->scene, SLOT(setCameraPerspective(btVector3*,btVector3*,float*,float*)));
}

void MainWindow::on_actionAbout_triggered()
{
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
    ui->scene->insertMotionToAllModels();
}

void MainWindow::on_actionInsertToSelectedModel_triggered()
{
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
    ui->scene->deleteSelectedModel();
}

void MainWindow::on_actionSetModelPose_triggered()
{
    ui->scene->setModelPose();
}

void MainWindow::on_actionBoneXCoordinateZero_triggered()
{
    m_boneMotionModel->resetBone(BoneMotionModel::kX);
}

void MainWindow::on_actionBoneYCoordinateZero_triggered()
{
    m_boneMotionModel->resetBone(BoneMotionModel::kY);
}

void MainWindow::on_actionBoneZCoordinateZero_triggered()
{
    m_boneMotionModel->resetBone(BoneMotionModel::kZ);
}

void MainWindow::on_actionBoneRotationZero_triggered()
{
    m_boneMotionModel->resetBone(BoneMotionModel::kRotation);
}

void MainWindow::on_actionBoneResetAll_triggered()
{
    ui->scene->resetAllBones();
}

void MainWindow::on_actionTimeline_triggered()
{
    m_timelineWidget->setVisible(true);
}

void MainWindow::on_actionTransform_triggered()
{
    m_transformWidget->setVisible(true);
}

void MainWindow::on_actionTabs_triggered()
{
    m_tabWidget->setVisible(true);
}

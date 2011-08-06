#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QSettings>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <LinearMath/btVector3.h>

namespace vpvl {
class Bone;
class PMDModel;
}

namespace Ui {
class MainWindow;
}

class BoneMotionModel;
class FaceMotionModel;
class TabWidget;
class TimelineTabWidget;
class TransformWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool validateLibraryVersion();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void selectModel();
    void revertSelectedModel();
    void addModel(vpvl::PMDModel *model);
    void deleteModel(vpvl::PMDModel *model);
    void setCurrentFPS(int value);
    void setModel(vpvl::PMDModel *value);
    void setBone(vpvl::Bone *value);
    void setCameraPerspective(const btVector3 &pos, const btVector3 &angle, float fovy, float distance);

    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();
    void on_actionAddModel_triggered();
    void on_actionAddAsset_triggered();
    void on_actionInsertToAllModels_triggered();
    void on_actionInsertToSelectedModel_triggered();
    void on_actionSetCamera_triggered();
    void on_actionExit_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_actionRotateUp_triggered();
    void on_actionRotateDown_triggered();
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();
    void on_actionTranslateUp_triggered();
    void on_actionTranslateDown_triggered();
    void on_actionTranslateLeft_triggered();
    void on_actionTranslateRight_triggered();
    void on_actionResetCamera_triggered();
    void on_actionRevertSelectedModel_triggered();
    void on_actionDeleteSelectedModel_triggered();
    void on_actionSetModelPose_triggered();
    void on_actionBoneXPositionZero_triggered();
    void on_actionBoneYPositionZero_triggered();
    void on_actionBoneZPositionZero_triggered();
    void on_actionBoneRotationZero_triggered();
    void on_actionBoneResetAll_triggered();
    void on_actionTimeline_triggered();
    void on_actionTransform_triggered();
    void on_actionTabs_triggered();
    void on_actionBoneDialog_triggered();
    void on_actionExportVMD_triggered();

private:
    void connectWidgets();
    void updateInformation();
    Ui::MainWindow *ui;
    QSettings m_settings;
    TabWidget *m_tabWidget;
    TimelineTabWidget *m_timelineTabWidget;
    TransformWidget *m_transformWidget;
    BoneMotionModel *m_boneMotionModel;
    FaceMotionModel *m_faceMotionModel;

    vpvl::PMDModel *m_model;
    vpvl::Bone *m_bone;
    btVector3 m_position;
    btVector3 m_angle;
    float m_fovy;
    float m_distance;
    int m_currentFPS;
};

#endif // MAINWINDOW_H

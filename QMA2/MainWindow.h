#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QSettings>
#include <QtGui/QMainWindow>
#include <vpvl/Common.h>

namespace vpvl {
class Asset;
class Bone;
class PMDModel;
}

namespace Ui {
class MainWindow;
}

class BoneMotionModel;
class FaceMotionModel;
class LicenseWidget;
class SceneWidget;
class TabWidget;
class TimelineTabWidget;
class TransformWidget;
class QUndoGroup;

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
    void newFile();
    bool save();
    bool saveAs();
    bool saveFile(const QString &filename);
    bool maybeSave();
    void selectModel();
    void setCurrentModel(vpvl::PMDModel *model);
    void revertSelectedModel();
    void addModel(vpvl::PMDModel *model);
    void deleteModel(vpvl::PMDModel *model);
    void addAsset(vpvl::Asset *asset);
    void deleteAsset(vpvl::Asset *asset);
    void setCurrentFPS(int value);
    void setModel(vpvl::PMDModel *value);
    void setBones(const QList<vpvl::Bone*> &bones);
    void setCameraPerspective(const vpvl::Vector3 &pos, const vpvl::Vector3 &angle, float fovy, float distance);
    void insertMotionToAllModels();
    void insertMotionToSelectedModel();
    void deleteSelectedModel();
    void saveModelPose();
    void resetBoneX();
    void resetBoneY();
    void resetBoneZ();
    void resetBoneRotation();
    void resetAllBones();
    void openBoneDialog();
    void saveAssetMetadata();

    void startSceneUpdate();
    void stopSceneUpdate();
    void updateFPS(int fps);
    const QString buildWindowTitle();
    const QString buildWindowTitle(int fps);

private:
    void buildUI();
    void retranslate();
    void connectWidgets();
    void updateInformation();

    QSettings m_settings;
    QUndoGroup *m_undo;
    LicenseWidget *m_licenseWidget;
    SceneWidget *m_sceneWidget;
    TabWidget *m_tabWidget;
    TimelineTabWidget *m_timelineTabWidget;
    TransformWidget *m_transformWidget;
    BoneMotionModel *m_boneMotionModel;
    FaceMotionModel *m_faceMotionModel;

    vpvl::PMDModel *m_model;
    vpvl::Bone *m_bone;
    vpvl::Vector3 m_position;
    vpvl::Vector3 m_angle;
    float m_fovy;
    float m_distance;
    int m_currentFPS;

    QAction *m_actionAddModel;
    QAction *m_actionAddAsset;
    QAction *m_actionInsertToAllModels;
    QAction *m_actionInsertToSelectedModel;
    QAction *m_actionSaveMotion;
    QAction *m_actionLoadModelPose;
    QAction *m_actionSaveModelPose;
    QAction *m_actionLoadAssetMetadata;
    QAction *m_actionSaveAssetMetadata;
    QAction *m_actionSetCamera;
    QAction *m_actionExit;
    QAction *m_actionAbout;
    QAction *m_actionAboutQt;
    QAction *m_actionPlay;
    QAction *m_actionPause;
    QAction *m_actionStop;
    QAction *m_actionShowGrid;
    QAction *m_actionShowBones;
    QAction *m_actionEnablePhysics;
    QAction *m_actionZoomIn;
    QAction *m_actionZoomOut;
    QAction *m_actionRotateUp;
    QAction *m_actionRotateDown;
    QAction *m_actionRotateLeft;
    QAction *m_actionRotateRight;
    QAction *m_actionTranslateUp;
    QAction *m_actionTranslateDown;
    QAction *m_actionTranslateLeft;
    QAction *m_actionTranslateRight;
    QAction *m_actionResetCamera;
    QAction *m_actionRevertSelectedModel;
    QAction *m_actionDeleteSelectedModel;
    QAction *m_actionBoneXPosZero;
    QAction *m_actionBoneYPosZero;
    QAction *m_actionBoneZPosZero;
    QAction *m_actionBoneRotationZero;
    QAction *m_actionBoneResetAll;
    QAction *m_actionBoneDialog;
    QAction *m_actionInsertEmptyFrame;
    QAction *m_actionDeleteSelectedFrame;
    QAction *m_actionUndoFrame;
    QAction *m_actionRedoFrame;
    QAction *m_actionViewTab;
    QAction *m_actionViewTimeline;
    QAction *m_actionViewTransform;
    QMenuBar *m_menuBar;
    QMenu *m_menuFile;
    QMenu *m_menuProject;
    QMenu *m_menuScene;
    QMenu *m_menuModel;
    QMenu *m_menuBone;
    QMenu *m_menuFrame;
    QMenu *m_menuView;
    QMenu *m_menuRetainModels;
    QMenu *m_menuRetainAssets;
    QMenu *m_menuHelp;
};

#endif // MAINWINDOW_H

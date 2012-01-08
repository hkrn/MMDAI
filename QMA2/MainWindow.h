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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QSettings>
#include <QtCore/QUuid>
#include <QtGui/QDialog>
#include <QtGui/QMainWindow>
#include <vpvl/Common.h>

namespace internal {
class Player;
}

namespace vpvl {
class Asset;
class Bone;
class PMDModel;
}

class BoneMotionModel;
class BoneUIDelegate;
class ExportVideoDialog;
class FaceMotionModel;
class LicenseWidget;
class LoggerWidget;
class PlaySettingDialog;
class SceneMotionModel;
class SceneWidget;
class TabWidget;
class TimelineTabWidget;
class QCheckBox;
class QDoubleSpinBox;
class QPushButton;
class QSpinBox;
class QSplitter;
class QUndoGroup;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const int kMaxRecentFiles = 10;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void connectSceneLoader();
    void newMotionFile();
    void newProjectFile();
    void loadProject();
    void saveMotion();
    void saveMotionAs();
    void saveProject();
    void saveProjectAs();
    void selectModel();
    void setCurrentModel(vpvl::PMDModel *model);
    void revertSelectedModel();
    void openRecentFile();
    void addRecentFile(const QString &filename);
    void updateRecentFiles();
    void clearRecentFiles();
    void addModel(vpvl::PMDModel *model, const QUuid &uuid);
    void deleteModel(vpvl::PMDModel *model, const QUuid &uuid);
    void addAsset(vpvl::Asset *asset, const QUuid &uuid);
    void deleteAsset(vpvl::Asset *asset, const QUuid &uuid);
    void insertMotionToAllModels();
    void insertMotionToSelectedModel();
    void deleteSelectedModel();
    void saveModelPose();
    void saveAssetMetadata();
    void exportImage();
    void exportVideo();
    void startExportingVideo();
    void addNewMotion();
    void openEdgeOffsetDialog();
    void startPlayingScene();
    void openPlaySettingDialog();
    void selectNextModel();
    void selectPreviousModel();
    void showLicenseWidget();

private:
    bool saveMotionAs(QString &filename);
    bool saveMotionFile(const QString &filename);
    bool saveProjectAs(QString &filename);
    bool saveProjectFile(const QString &filename);
    bool maybeSaveMotion();
    bool maybeSaveProject();
    bool confirmSave(bool condition, bool &cancel);
    void buildUI();
    void bindActions();
    void retranslate();
    void connectWidgets();
    void updateInformation();
    void updateWindowTitle();
    const QString openSaveDialog(const QString &name,
                                 const QString &desc,
                                 const QString &exts,
                                 const QString &defaultFilename);

    QSettings m_settings;
    QUndoGroup *m_undo;
    LicenseWidget *m_licenseWidget;
    LoggerWidget *m_loggerWidget;
    SceneWidget *m_sceneWidget;
    TabWidget *m_tabWidget;
    TimelineTabWidget *m_timelineTabWidget;
    BoneMotionModel *m_boneMotionModel;
    FaceMotionModel *m_faceMotionModel;
    SceneMotionModel *m_sceneMotionModel;
    ExportVideoDialog *m_exportingVideoDialog;
    PlaySettingDialog *m_playSettingDialog;
    BoneUIDelegate *m_boneUIDelegate;
    internal::Player *m_player;
    QString m_currentProjectFilename;
    QString m_currentMotionFilename;

    vpvl::PMDModel *m_model;
    vpvl::Bone *m_bone;
    vpvl::Vector3 m_position;
    vpvl::Vector3 m_angle;
    float m_fovy;
    float m_distance;
    int m_currentFPS;

    QSplitter *m_mainSplitter;
    QSplitter *m_leftSplitter;
    QAction *m_actionRecentFiles[kMaxRecentFiles];
    QAction *m_actionClearRecentFiles;
    QAction *m_actionNewProject;
    QAction *m_actionNewMotion;
    QAction *m_actionLoadProject;
    QAction *m_actionAddModel;
    QAction *m_actionAddAsset;
    QAction *m_actionInsertToAllModels;
    QAction *m_actionInsertToSelectedModel;
    QAction *m_actionSetCamera;
    QAction *m_actionSaveProject;
    QAction *m_actionSaveProjectAs;
    QAction *m_actionSaveMotion;
    QAction *m_actionSaveMotionAs;
    QAction *m_actionLoadModelPose;
    QAction *m_actionSaveModelPose;
    QAction *m_actionLoadAssetMetadata;
    QAction *m_actionSaveAssetMetadata;
    QAction *m_actionExportImage;
    QAction *m_actionExportVideo;
    QAction *m_actionExit;
    QAction *m_actionAbout;
    QAction *m_actionAboutQt;
    QAction *m_actionPlay;
    QAction *m_actionPlaySettings;
    QAction *m_actionShowGrid;
    QAction *m_actionShowBones;
    QAction *m_actionEnablePhysics;
    QAction *m_actionShowModelDialog;
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
    QAction *m_actionSelectNextModel;
    QAction *m_actionSelectPreviousModel;
    QAction *m_actionRevertSelectedModel;
    QAction *m_actionDeleteSelectedModel;
    QAction *m_actionEdgeOffsetDialog;
    QAction *m_actionTranslateModelUp;
    QAction *m_actionTranslateModelDown;
    QAction *m_actionTranslateModelLeft;
    QAction *m_actionTranslateModelRight;
    QAction *m_actionResetModelPosition;
    QAction *m_actionBoneXPosZero;
    QAction *m_actionBoneYPosZero;
    QAction *m_actionBoneZPosZero;
    QAction *m_actionBoneRotationZero;
    QAction *m_actionBoneResetAll;
    QAction *m_actionBoneDialog;
    QAction *m_actionRegisterFrame;
    QAction *m_actionSelectAllFrames;
    QAction *m_actionSelectFrameDialog;
    QAction *m_actionFrameWeightDialog;
    QAction *m_actionInsertEmptyFrame;
    QAction *m_actionDeleteSelectedFrame;
    QAction *m_actionNextFrame;
    QAction *m_actionPreviousFrame;
    QAction *m_actionCopy;
    QAction *m_actionPaste;
    QAction *m_actionReversedPaste;
    QAction *m_actionUndoFrame;
    QAction *m_actionRedoFrame;
    QAction *m_actionViewLogMessage;
    QAction *m_actionEnableMoveGesture;
    QAction *m_actionEnableRotateGesture;
    QAction *m_actionEnableScaleGesture;
    QAction *m_actionAddModelOnToolBar;
    QAction *m_actionAddAssetOnToolBar;
    QAction *m_actionSelectModelOnToolBar;
    QAction *m_actionCreateMotionOnToolBar;
    QAction *m_actionInsertMotionOnToolBar;
    QAction *m_actionDeleteModelOnToolBar;
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
    QMenu *m_menuRecentFiles;
    QMenu *m_menuHelp;

    Q_DISABLE_COPY(MainWindow)
};

#endif // MAINWINDOW_H

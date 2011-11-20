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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QSettings>
#include <QtGui/QDialog>
#include <QtGui/QMainWindow>
#include <vpvl/Common.h>

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
class PlaySettingDialog;
class SceneMotionModel;
class SceneWidget;
class TabWidget;
class TimelineTabWidget;
class TransformWidget;
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

private:
    void buildUI();
    void retranslate();
    void connectWidgets();
    void updateInformation();
    const QString openSaveDialog(const QString &name, const QString &desc, const QString &exts);

    QSettings m_settings;
    QUndoGroup *m_undo;
    LicenseWidget *m_licenseWidget;
    SceneWidget *m_sceneWidget;
    TabWidget *m_tabWidget;
    TimelineTabWidget *m_timelineTabWidget;
    TransformWidget *m_transformWidget;
    BoneMotionModel *m_boneMotionModel;
    FaceMotionModel *m_faceMotionModel;
    SceneMotionModel *m_sceneMotionModel;
    ExportVideoDialog *m_exportingVideoDialog;
    PlaySettingDialog *m_playSettingDialog;
    BoneUIDelegate *m_boneUIDelegate;

    vpvl::PMDModel *m_model;
    vpvl::Bone *m_bone;
    vpvl::Vector3 m_position;
    vpvl::Vector3 m_angle;
    float m_fovy;
    float m_distance;
    int m_currentFPS;

    QSplitter *m_mainSplitter;
    QSplitter *m_leftSplitter;
    QAction *m_actionAddModel;
    QAction *m_actionAddAsset;
    QAction *m_actionNewMotion;
    QAction *m_actionInsertToAllModels;
    QAction *m_actionInsertToSelectedModel;
    QAction *m_actionSetCamera;
    QAction *m_actionSaveMotion;
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
    QAction *m_actionInsertEmptyFrame;
    QAction *m_actionDeleteSelectedFrame;
    QAction *m_actionNextFrame;
    QAction *m_actionPreviousFrame;
    QAction *m_actionCopy;
    QAction *m_actionPaste;
    QAction *m_actionReversedPaste;
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

    Q_DISABLE_COPY(MainWindow)
};

#endif // MAINWINDOW_H

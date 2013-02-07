/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#ifndef VPVM_MAINWINDOW_H
#define VPVM_MAINWINDOW_H

#include <vpvl2/Common.h>
#include <vpvl2/extensions/icu4c/Encoding.h>
#include <vpvl2/qt/RenderContext.h>

#include <QSettings>
#include <QUuid>
#include <QDialog>
#include <QMainWindow>

class QCheckBox;
class QDoubleSpinBox;
class QProgressDialog;
class QPushButton;
class QSpinBox;
class QSplitter;
class QUndoGroup;

namespace vpvl2 {
class Factory;
class IBone;
class IEncoding;
class IModel;
class IMotion;
}

namespace vpvm
{

using namespace vpvl2;
using namespace vpvl2::qt;
class AVFactory;
class BoneMotionModel;
class BoneUIDelegate;
class ExportVideoDialog;
class MorphMotionModel;
class LicenseWidget;
class LoggerWidget;
class ModelTabWidget;
class Player;
class PlaySettingDialog;
class SceneMotionModel;
class SceneWidget;
class ScenePlayer;
class TabWidget;
class TimelineTabWidget;

class IAudioDecoder;
class IVideoEncoder;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const int kMaxRecentFiles = 10;

    explicit MainWindow(const Encoding::Dictionary *dictionary, QWidget *parent = 0);
    ~MainWindow();

signals:
    void sceneDidRendered(const QImage &image);

protected:
    void closeEvent(QCloseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void bindSceneLoader();
    void newMotionFile();
    void newProjectFile();
    void loadProject();
    void saveMotion();
    void saveMotionAs();
    void saveCameraMotionAs();
    void saveProject();
    void saveProjectAs();
    void selectModel();
    void setCurrentModel(IModelSharedPtr model);
    void revertSelectedModel();
    void openRecentFile();
    void addRecentFile(const QString &filename, bool didLoad);
    void updateRecentFiles();
    void clearRecentFiles();
    void addModel(IModelSharedPtr model, const QUuid &uuid);
    void deleteModel(IModelSharedPtr model, const QUuid &uuid);
    void addAsset(IModelSharedPtr asset, const QUuid &uuid);
    void deleteAsset(IModelSharedPtr asset, const QUuid &uuid);
    void insertMotionToAllModels();
    void insertMotionToSelectedModel();
    void deleteSelectedModel();
    void saveModelPose();
    void saveAssetMetadata();
    void exportImage();
    void exportVideo();
    void invokeImageExporter();
    void invokeVideoEncoder();
    void addNewMotion();
    void invokePlayer();
    void openPlaySettingDialog();
    void selectNextModel();
    void selectPreviousModel();
    void showLicenseWidget();
    void openGravitySettingDialog();
    void openRenderOrderDialog();
    void openScreenColorDialog();
    void openShadowMapDialog();
    void openBackgroundImageDialog();
    void openUndoView();
    void enableSelectingBonesAndMorphs();
    void resetSceneToMotionModels();
    void openProgress(const QString &title, bool cancellable);
    void updateProgress(int value, int max, const QString &text);
    void updateProgressTitle(const QString &title);
    void closeProgress();

private:
    struct WindowState;
    bool saveMotionAs(QString &filename);
    bool saveMotionFile(const QString &filename);
    bool saveMotionFile(const QString &filename, IMotion *motion);
    bool saveProjectAs(QString &filename);
    bool saveProjectFile(const QString &filename);
    bool maybeSaveMotion();
    bool maybeSaveProject();
    bool confirmSave(bool condition, bool &cancel);
    void createActionsAndMenus();
    void createPlayerSettingDialog();
    void createExportSettingDialog();
    void createScenePlayer();
    void bindActions();
    void bindWidgets();
    void retranslate();
    void updateInformation();
    void updateWindowTitle();
    void saveWindowStateAndResize(const QSize &videoSize, WindowState &state);
    void restoreWindowState(const WindowState &state);
    void waitAudioThread();
    void waitVideoThread();

    QScopedPointer<IEncoding> m_encoding;
    QScopedPointer<Factory> m_factory;
    QSettings m_settings;
    QScopedPointer<QUndoGroup> m_undo;
    QScopedPointer<LicenseWidget> m_licenseWidget;
    QScopedPointer<SceneWidget> m_sceneWidget;
    QScopedPointer<TabWidget> m_sceneTabWidget;
    QScopedPointer<BoneMotionModel> m_boneMotionModel;
    QScopedPointer<MorphMotionModel> m_morphMotionModel;
    QScopedPointer<SceneMotionModel> m_sceneMotionModel;
    QScopedPointer<ModelTabWidget> m_modelTabWidget;
    QScopedPointer<TimelineTabWidget> m_timelineTabWidget;
    QScopedPointer<ExportVideoDialog> m_exportingVideoDialog;
    QScopedPointer<PlaySettingDialog> m_playSettingDialog;
    QScopedPointer<BoneUIDelegate> m_boneUIDelegate;
    QScopedPointer<AVFactory> m_avFactory;
    QScopedPointer<IAudioDecoder> m_audioDecoder;
    QScopedPointer<IVideoEncoder> m_videoEncoder;
    QScopedPointer<ScenePlayer> m_player;
    QScopedPointer<QDockWidget> m_timelineDockWidget;
    QScopedPointer<QDockWidget> m_sceneDockWidget;
    QScopedPointer<QDockWidget> m_modelDockWidget;
    QScopedPointer<QToolBar> m_mainToolBar;
    QList<QAction *> m_actionRecentFiles;
    QScopedPointer<QAction> m_actionClearRecentFiles;
    QScopedPointer<QAction> m_actionNewProject;
    QScopedPointer<QAction> m_actionNewMotion;
    QScopedPointer<QAction> m_actionLoadProject;
    QScopedPointer<QAction> m_actionAddModel;
    QScopedPointer<QAction> m_actionAddAsset;
    QScopedPointer<QAction> m_actionInsertToAllModels;
    QScopedPointer<QAction> m_actionInsertToSelectedModel;
    QScopedPointer<QAction> m_actionSetCamera;
    QScopedPointer<QAction> m_actionSaveProject;
    QScopedPointer<QAction> m_actionSaveProjectAs;
    QScopedPointer<QAction> m_actionSaveMotion;
    QScopedPointer<QAction> m_actionSaveMotionAs;
    QScopedPointer<QAction> m_actionSaveCameraMotionAs;
    QScopedPointer<QAction> m_actionLoadModelPose;
    QScopedPointer<QAction> m_actionSaveModelPose;
    QScopedPointer<QAction> m_actionLoadAssetMetadata;
    QScopedPointer<QAction> m_actionSaveAssetMetadata;
    QScopedPointer<QAction> m_actionExportImage;
    QScopedPointer<QAction> m_actionExportVideo;
    QScopedPointer<QAction> m_actionExit;
    QScopedPointer<QAction> m_actionAbout;
    QScopedPointer<QAction> m_actionAboutQt;
    QScopedPointer<QAction> m_actionPlay;
    QScopedPointer<QAction> m_actionPlaySettings;
    QScopedPointer<QAction> m_actionOpenGravitySettingsDialog;
    QScopedPointer<QAction> m_actionOpenRenderOrderDialog;
    QScopedPointer<QAction> m_actionOpenScreenColorDialog;
    QScopedPointer<QAction> m_actionOpenShadowMapDialog;
    QScopedPointer<QAction> m_actionEnablePhysics;
    QScopedPointer<QAction> m_actionShowGrid;
    QScopedPointer<QAction> m_actionSetBackgroundImage;
    QScopedPointer<QAction> m_actionClearBackgroundImage;
    QScopedPointer<QAction> m_actionOpenBackgroundImageDialog;
    QScopedPointer<QAction> m_actionZoomIn;
    QScopedPointer<QAction> m_actionZoomOut;
    QScopedPointer<QAction> m_actionRotateUp;
    QScopedPointer<QAction> m_actionRotateDown;
    QScopedPointer<QAction> m_actionRotateLeft;
    QScopedPointer<QAction> m_actionRotateRight;
    QScopedPointer<QAction> m_actionTranslateUp;
    QScopedPointer<QAction> m_actionTranslateDown;
    QScopedPointer<QAction> m_actionTranslateLeft;
    QScopedPointer<QAction> m_actionTranslateRight;
    QScopedPointer<QAction> m_actionResetCamera;
    QScopedPointer<QAction> m_actionSelectNextModel;
    QScopedPointer<QAction> m_actionSelectPreviousModel;
    QScopedPointer<QAction> m_actionRevertSelectedModel;
    QScopedPointer<QAction> m_actionDeleteSelectedModel;
    QScopedPointer<QAction> m_actionTranslateModelUp;
    QScopedPointer<QAction> m_actionTranslateModelDown;
    QScopedPointer<QAction> m_actionTranslateModelLeft;
    QScopedPointer<QAction> m_actionTranslateModelRight;
    QScopedPointer<QAction> m_actionResetModelPosition;
    QScopedPointer<QAction> m_actionBoneXPosZero;
    QScopedPointer<QAction> m_actionBoneYPosZero;
    QScopedPointer<QAction> m_actionBoneZPosZero;
    QScopedPointer<QAction> m_actionBoneRotationZero;
    QScopedPointer<QAction> m_actionBoneResetAll;
    QScopedPointer<QAction> m_actionBoneDialog;
    QScopedPointer<QAction> m_actionRegisterKeyframe;
    QScopedPointer<QAction> m_actionSelectAllKeyframes;
    QScopedPointer<QAction> m_actionSelectKeyframeDialog;
    QScopedPointer<QAction> m_actionKeyframeWeightDialog;
    QScopedPointer<QAction> m_actionInterpolationDialog;
    QScopedPointer<QAction> m_actionInsertEmptyFrame;
    QScopedPointer<QAction> m_actionDeleteSelectedKeyframe;
    QScopedPointer<QAction> m_actionNextFrame;
    QScopedPointer<QAction> m_actionPreviousFrame;
    QScopedPointer<QAction> m_actionCut;
    QScopedPointer<QAction> m_actionCopy;
    QScopedPointer<QAction> m_actionPaste;
    QScopedPointer<QAction> m_actionReversedPaste;
    QScopedPointer<QAction> m_actionUndo;
    QScopedPointer<QAction> m_actionRedo;
    QScopedPointer<QAction> m_actionOpenUndoView;
    QScopedPointer<QAction> m_actionViewLogMessage;
    QScopedPointer<QAction> m_actionEnableGestures;
    QScopedPointer<QAction> m_actionEnableMoveGesture;
    QScopedPointer<QAction> m_actionEnableRotateGesture;
    QScopedPointer<QAction> m_actionEnableScaleGesture;
    QScopedPointer<QAction> m_actionEnableUndoGesture;
    QScopedPointer<QAction> m_actionShowTimelineDock;
    QScopedPointer<QAction> m_actionShowSceneDock;
    QScopedPointer<QAction> m_actionShowModelDock;
    QScopedPointer<QAction> m_actionShowModelDialog;
    QScopedPointer<QAction> m_actionRegisterKeyframeOnToolBar;
    QScopedPointer<QAction> m_actionDeleteSelectedKeyframeOnToolBar;
    QScopedPointer<QAction> m_actionSelectModelOnToolBar;
    QScopedPointer<QAction> m_actionCreateProjectOnToolBar;
    QScopedPointer<QAction> m_actionCreateMotionOnToolBar;
    QScopedPointer<QAction> m_actionAddObjectOnToolBar;
    QScopedPointer<QAction> m_actionDeleteModelOnToolBar;
    QScopedPointer<QAction> m_actionEnableEffectOnToolBar;
    QScopedPointer<QAction> m_actionPlayOnToolBar;
    QScopedPointer<QAction> m_actionExportImageOnToolBar;
    QScopedPointer<QAction> m_actionExportVideoOnToolBar;
    QScopedPointer<QAction> m_actionSetSoftwareSkinningFallback;
    QScopedPointer<QAction> m_actionSetParallelSkinning;
    QScopedPointer<QAction> m_actionSetOpenCLSkinningType1;
    QScopedPointer<QAction> m_actionSetOpenCLSkinningType2;
    QScopedPointer<QAction> m_actionSetVertexShaderSkinningType1;
    QScopedPointer<QAction> m_actionEnableEffect;
    QScopedPointer<QMenuBar> m_menuBar;
    QScopedPointer<QMenu> m_menuFile;
    QScopedPointer<QMenu> m_menuEdit;
    QScopedPointer<QMenu> m_menuProject;
    QScopedPointer<QMenu> m_menuScene;
    QScopedPointer<QMenu> m_menuModel;
    QScopedPointer<QMenu> m_menuKeyframe;
    QScopedPointer<QMenu> m_menuEffect;
    QScopedPointer<QMenu> m_menuView;
    QScopedPointer<QMenu> m_menuRetainModels;
    QScopedPointer<QMenu> m_menuRetainAssets;
    QScopedPointer<QMenu> m_menuRecentFiles;
    QScopedPointer<QMenu> m_menuHelp;
    QScopedPointer<QMenu> m_menuAcceleration;
    QScopedPointer<QProgressDialog> m_progress;
    QString m_currentProjectFilename;
    QString m_currentMotionFilename;
    LoggerWidget *m_loggerWidgetRef;
    IModelSharedPtr m_modelRef;
    IBone *m_boneRef;
    Vector3 m_position;
    Vector3 m_angle;
    Scalar m_fovy;
    Scalar m_distance;
    int m_currentFPS;
    int m_nestProgressCount;

    Q_DISABLE_COPY(MainWindow)
};

}

#endif // MAINWINDOW_H

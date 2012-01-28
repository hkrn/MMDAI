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
#include <QtGui/QMainWindow>

namespace vpvl {
class Asset;
class PMDModel;
}

class ExtendedSceneWidget;
class LicenseWidget;
class LoggerWidget;

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
    void openRecentFile();
    void addRecentFile(const QString &filename);
    void updateRecentFiles();
    void clearRecentFiles();
    void selectCurrentModel();
    void setCurrentModel(vpvl::PMDModel *value);
    void addModel(vpvl::PMDModel *model, const QUuid &uuid);
    void deleteModel(vpvl::PMDModel *model, const QUuid &uuid);
    void addAsset(vpvl::Asset *asset, const QUuid &uuid);
    void deleteAsset(vpvl::Asset *asset, const QUuid &uuid);
    void updateFPS(int fps);
    void executeCommand();
    void executeEvent();
    void selectNextModel();
    void selectPreviousModel();
    void connectSceneLoader();
    void disableAcceleration();

private:
    void startSceneUpdate();
    void stopSceneUpdate();
    const QString buildWindowTitle();
    const QString buildWindowTitle(int fps);
    void connectWidgets();
    void updateInformation();
    void buildMenuBar();
    void retranslate();

    QSettings m_settings;
    LicenseWidget *m_licenseWidget;
    LoggerWidget *m_loggerWidget;
    ExtendedSceneWidget *m_sceneWidget;
    vpvl::PMDModel *m_model;
    int m_currentFPS;

    QAction *m_actionRecentFiles[kMaxRecentFiles];
    QAction *m_actionClearRecentFiles;
    QAction *m_actionAddModel;
    QAction *m_actionAddAsset;
    QAction *m_actionInsertToAllModels;
    QAction *m_actionInsertToSelectedModel;
    QAction *m_actionSetCamera;
    QAction *m_actionExit;
    QAction *m_actionAbout;
    QAction *m_actionAboutQt;
    QAction *m_actionLoadScript;
    QAction *m_actionPlay;
    QAction *m_actionPause;
    QAction *m_actionStop;
    QAction *m_actionEnableAcceleration;
    QAction *m_actionShowModelDialog;
    QAction *m_actionExecuteCommand;
    QAction *m_actionExecuteEvent;
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
    QAction *m_actionShowBones;
    QAction *m_actionShowLogMessage;
    QAction *m_actionEnableMoveGesture;
    QAction *m_actionEnableRotateGesture;
    QAction *m_actionEnableScaleGesture;
    QMenuBar *m_menuBar;
    QMenu *m_menuFile;
    QMenu *m_menuScript;
    QMenu *m_menuScene;
    QMenu *m_menuModel;
    QMenu *m_menuRetainModels;
    QMenu *m_menuRetainAssets;
    QMenu *m_menuRecentFiles;
    QMenu *m_menuView;
    QMenu *m_menuHelp;

    Q_DISABLE_COPY(MainWindow)
};

#endif // MAINWINDOW_H

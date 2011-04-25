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

#ifndef QMAWINDOW_H
#define QMAWINDOW_H

#include <QMainWindow>
#include <QSettings>

class QMAPreference;
class QMAWidget;
class QMALogViewWidget;

class QMAWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QMAWindow(QWidget *parent = 0);
    ~QMAWindow();

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void insertMotionToAllModels();
    void insertMotionToSelectedModel();
    void addModel();
    void setStage();
    void setFloor();
    void setBackground();
    void rotateUp();
    void rotateDown();
    void rotateLeft();
    void rotateRight();
    void translateUp();
    void translateDown();
    void translateLeft();
    void translateRight();
    void toggleDisplayBone();
    void toggleDisplayRigidBody();
    void toggleFullScreen();
    void increaseEdgeThin();
    void decreaseEdgeThin();
    void togglePhysicSimulation();
    void toggleShadowMapping();
    void toggleShadowMappingLightFirst();
    void speak();
    void zoomIn();
    void zoomOut();
    void selectObject();
    void changeSelectedObject();
    void deleteSelectedObject();
    void showLogWindow();
    void saveScene();
    void resizeScene();
    void about();
    void receiveEvent(const QString &type, const QList<QVariant> &arguments);

private:
    void setEdgeThin(float value);
    void createMenu();
    void createActions();
    void readSetting();
    void writeSetting();
    void setDirectorySetting(const QString &key, const QString &fileName);

    QSettings m_settings;
    QMAPreference *m_preference;
    QMAWidget *m_widget;
    QMALogViewWidget *m_logView;
    QMenu *m_fileMenu;
    QMenu *m_sceneMenu;
    QMenu *m_modelMenu;
    QMenu *m_motionMenu;
    QMenu *m_selectModelMenu;
    QMenu *m_windowMenu;
    QMenu *m_helpMenu;
    QAction *m_insertMotionToAllAction;
    QAction *m_insertMotionToSelectedAction;
    QAction *m_addModelAction;
    QAction *m_setStageAction;
    QAction *m_setFloorAction;
    QAction *m_setBackgroundAction;
    QAction *m_saveSceneAction;
    QAction *m_showLogAction;
    QAction *m_increaseEdgeThinAction;
    QAction *m_decreaseEdgeThinAction;
    QAction *m_toggleDisplayBoneAction;
    QAction *m_toggleDisplayRigidBodyAction;
    QAction *m_togglePhysicSimulationAction;
    QAction *m_toggleShadowMappingAction;
    QAction *m_toggleShadowMappingFirstAction;
    QAction *m_toggleFullScreenAction;
    QAction *m_speakAction;
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_rotateUpAction;
    QAction *m_rotateDownAction;
    QAction *m_rotateLeftAction;
    QAction *m_rotateRightAction;
    QAction *m_translateUpAction;
    QAction *m_translateDownAction;
    QAction *m_translateLeftAction;
    QAction *m_translateRightAction;
    QAction *m_changeSelectedObjectAction;
    QAction *m_deleteSelectedObjectAction;
    QAction *m_resize512x288Action;
    QAction *m_resize512x384Action;
    QAction *m_resize640x480Action;
    QAction *m_resize800x480Action;
    QAction *m_resize1024x768Action;
    QAction *m_resize1280x800Action;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_aboutQtAction;

    bool m_isFullScreen;
    bool m_enablePhysicsSimulation;

    Q_DISABLE_COPY(QMAWindow);
};

#endif // QMAWINDOW_H

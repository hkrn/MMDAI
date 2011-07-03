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
#include <QtGui/QMainWindow>
#include <LinearMath/btVector3.h>

namespace vpvl {
class Bone;
class PMDModel;
}

class QLabel;
class FaceWidget;
class HandleWidget;
class PerspectionWidget;
class SceneWidget;
class TimelineWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void about();
    void selectModel();
    void revertSelectedModel();
    void addModel(vpvl::PMDModel *model);
    void deleteModel(vpvl::PMDModel *model);
    void setCurrentFPS(int value);
    void setModel(vpvl::PMDModel *value);
    void setBone(vpvl::Bone *value);
    void setCameraPerspection(const btVector3 &pos, const btVector3 &angle, float fovy, float distance);

private:
    void createActions();
    void createMenus(QMenuBar *menuBar);
    void updateInformation();

    QAction *m_addModelAction;
    QAction *m_addAssetAction;
    QAction *m_insertMotionToAllAction;
    QAction *m_insertMotionToSelectedAction;
    QAction *m_setCameraAction;
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
    QAction *m_resetCameraAction;
    QAction *m_revertSelectedModelAction;
    QAction *m_deleteSelectedModelAction;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_aboutQtAction;
    QMenu *m_fileMenu;
    QMenu *m_sceneMenu;
    QMenu *m_modelMenu;
    QMenu *m_helpMenu;
    QMenu *m_selectModelMenu;

    QLabel *m_info;
    QSettings m_settings;
    FaceWidget *m_face;
    HandleWidget *m_handle;
    PerspectionWidget *m_perspection;
    SceneWidget *m_scene;
    TimelineWidget *m_timeline;

    vpvl::PMDModel *m_model;
    vpvl::Bone *m_bone;
    btVector3 m_position;
    btVector3 m_angle;
    float m_fovy;
    float m_distance;
    int m_currentFPS;
};

#endif // MAINWINDOW_H

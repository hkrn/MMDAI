/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn (libMMDAI)                         */
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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

#include "QMAWidget.h"

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
  void increaseEdgeThin();
  void decreaseEdgeThin();
  void togglePhysicSimulation();
  void toggleShadowMapping();
  void toggleShadowMappingLightFirst();
  void zoomIn();
  void zoomOut();
  void changeSelectedObject();
  void deleteSelectedObject();
  void about();
  void receiveEvent(SceneController *controller,
                    const QString &type,
                    const QString &arguments);

private:
  void setEdgeThin(int n);
  void createMenu();
  void createActions();
  void readSetting();
  void writeSetting();

  QSettings *m_settings;
  QMAWidget *m_widget;
  QMenu *m_fileMenu;
  QMenu *m_sceneMenu;
  QMenu *m_modelMenu;
  QMenu *m_helpMenu;
  QMenu *m_motionMenu;
  QAction *m_insertMotionToAllAction;
  QAction *m_insertMotionToSelectedAction;
  QAction *m_addModelAction;
  QAction *m_setStageAction;
  QAction *m_setFloorAction;
  QAction *m_setBackgroundAction;
  QAction *m_increaseEdgeThinAction;
  QAction *m_decreaseEdgeThinAction;
  QAction *m_toggleDisplayBone;
  QAction *m_toggleDisplayRigidBody;
  QAction *m_togglePhysicSimulationAction;
  QAction *m_toggleShadowMapping;
  QAction *m_toggleShadowMappingFirst;
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
  QAction *m_exitAction;
  QAction *m_aboutAction;
  QAction *m_aboutQtAction;

  bool m_enablePhysicsSimulation;
};

#endif // QMAWINDOW_H

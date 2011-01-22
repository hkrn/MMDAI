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

#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QtGui>

#include "MMDAI/CommandParser.h"
#include "MMDAI/SceneController.h"
#include "MMDAI/TextRenderer.h"

#include "QMATimer.h"

#include <qgl.h>

#define MAX_MODEL 20

class QMAWidget : public QGLWidget, public SceneEventHandler
{
  Q_OBJECT

public:
  explicit QMAWidget(QWidget *parent = 0);
  ~QMAWidget();

  void handleEventMessage(const char *eventType, int argc, ...);
  SceneController *getSceneController();

  void toggleDisplayBone();
  void toggleDisplayRigidBody();
  void sendKeyEvent(const QString &text);
  void changeBaseMotion(PMDObject *object, const char *filename);

public slots:
  void delegateCommand(const QString &command, const QStringList &arguments);
  void delegateEvent(const QString &type, const QStringList &arguments);

signals:
  void pluginInitialized(const QString);
  void pluginStarted();
  void pluginStopped();
  void pluginWindowCreated();
  void pluginCommandPost(const QString&, const QStringList&);
  void pluginEventPost(const QString&, const QStringList&);
  void pluginUpdated(const QRect, const double);
  void pluginRendered();

protected:
  void initializeGL();
  void resizeGL(int width, int height);
  void paintGL();
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseDoubleClickEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);
  void timerEvent(QTimerEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dropEvent(QDropEvent *event);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void closeEvent(QCloseEvent *event);

private:
  void updateScene();
  void loadModel();
  void loadPlugins();

  void updateModelPositionAndRotation(double fps);
  void renderDebugModel();
  void renderLogger();

  QMATimer m_timer;
  SceneController *m_controller;
  CommandParser m_parser;

  int m_x;
  int m_y;

  bool m_doubleClicked;
  bool m_showLog;
  bool m_displayBone;
  bool m_displayRigidBody;

  double m_frameAdjust;
  double m_frameCue;
  double m_movings[MAX_MODEL];

  Q_DISABLE_COPY(QMAWidget);
};

#endif // WIDGET_H

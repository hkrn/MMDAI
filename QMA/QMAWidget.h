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

#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QtGui>

#include <MMDAI/CommandParser.h>
#include <MMDAI/ISceneEventHandler.h>

#include "QMAModelLoaderFactory.h"
#include "QMAPlugin.h"
#include "QMATimer.h"

/* to load glee correctly, should include QtOpenGL after MMDAI/MMDME */
#include <QtOpenGL>

namespace MMDAI {
class PMDObject;
class SceneController;
}

enum QMAWidgetZoomOption {
    Normal = 0x0,
    Faster = 0x1,
    Slower = 0x2
         };

class QMAPreference;

class QMAWidget : public QGLWidget, public MMDAI::ISceneEventHandler
{
    Q_OBJECT

public:
    explicit QMAWidget(QMAPreference *preference, QWidget *parent = 0);
    ~QMAWidget();

    bool addModel(const QString &filename);
    bool changeModel(const QString &filename, MMDAI::PMDObject *object);
    bool setStage(const QString &filename);
    bool setFloor(const QString &filename);
    bool setBackground(const QString &filename);
    bool insertMotionToAllModels(const QString &filename);
    bool insertMotionToModel(const QString &filename, MMDAI::PMDObject *object);
    void handleEventMessage(const char *eventType, int argc, ...);
    void setBaseMotion(MMDAI::PMDObject *object, MMDAI::IMotionLoader *loader);
    void zoom(bool up, enum QMAWidgetZoomOption option);

    inline void toggleDisplayBone() {
        m_displayBone = !m_displayBone;
    }
    inline void toggleDisplayRigidBody() {
        m_displayRigidBody = !m_displayRigidBody;
    }
    inline QMATimer *getSceneFrameTimer() {
        return &m_sceneFrameTimer;
    }
    inline MMDAI::SceneController *getSceneController() const {
        return m_controller;
    }

public slots:
    void delegateCommand(const QString &command, const QList<QVariant> &arguments);
    void delegateEvent(const QString &type, const QList<QVariant> &arguments);

signals:
    void pluginLoaded(MMDAI::SceneController *, const QString &);
    void pluginUnloaded();
    void pluginCommandPost(const QString&, const QList<QVariant>&);
    void pluginEventPost(const QString&, const QList<QVariant>&);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void showEvent(QShowEvent *event);
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void updateScene();

private:
    void loadModel();
    void loadPlugins(QFile &file);
    void addPlugin(QMAPlugin *plugin);

    void updateModelPositionAndRotation(double fps);
    void renderDebugModel();
    void renderLogger();

    QMAModelLoaderFactory m_factory;
    QMAPreference *m_preference;
    QMATimer m_sceneFrameTimer;
    QTimer m_sceneUpdateTimer;
    MMDAI::SceneController *m_controller;
    MMDAI::CommandParser m_parser;

    int m_x;
    int m_y;

    bool m_doubleClicked;
    bool m_showLog;
    bool m_displayBone;
    bool m_displayRigidBody;
    bool m_activeMotion;

    Q_DISABLE_COPY(QMAWidget);
};

#endif // WIDGET_H

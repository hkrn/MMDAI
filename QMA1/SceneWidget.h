/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef SCENEWIDGET_H
#define SCENEWIDGET_H

#include <QtCore/QElapsedTimer>
#include <GL/glew.h>
#include <QtOpenGL/QGLWidget>

#include <LinearMath/btVector3.h>
#include <vpvl/Bone.h>

namespace vpvl {
namespace gl {
class Renderer;
}
class Asset;
class PMDModel;
class Scene;
class VMDMotion;
}

class Delegate;
class PlayerWidget;
class QProgressDialog;
class QSettings;
class SceneLoader;
class VPDFile;
class World;

class SceneWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit SceneWidget(QWidget *parent = 0);
    ~SceneWidget();

    vpvl::PMDModel *findModel(const QString &name);
    vpvl::PMDModel *selectedModel() const;
    void setSelectedModel(vpvl::PMDModel *value);
    void setCurrentFPS(int value);

    void setSettings(QSettings *value) { m_settings = value; }
    bool isDisplayBones() const { return m_visibleBones; }
    void setDisplayBones(bool value) { m_visibleBones = value; }

public slots:
    void play();
    void pause();
    void stop();

    void addModel();
    vpvl::PMDModel *addModel(const QString &path);
    void insertMotionToAllModels();
    vpvl::VMDMotion *insertMotionToAllModels(const QString &path);
    void insertMotionToSelectedModel();
    vpvl::VMDMotion *insertMotionToSelectedModel(const QString &path);
    vpvl::VMDMotion *insertMotionToModel(const QString &path, vpvl::PMDModel *model);
    void addAsset();
    vpvl::Asset *addAsset(const QString &path);
    void setCamera();
    vpvl::VMDMotion *setCamera(const QString &path);
    void deleteSelectedModel();
    void deleteModel(vpvl::PMDModel *model);
    void deleteMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void resetCamera();
    void setLightColor(const btVector4 &color);
    void setLightPosition(const btVector3 &position);

    void rotate(float x, float y);
    void translate(float x, float y);
    void seekMotion(float frameIndex);
    void setCameraPerspective(btVector3 *pos, btVector3 *angle, float *fovy, float *distance);
    void zoom(bool up, const Qt::KeyboardModifiers &modifiers);
    void zoomIn() { zoom(true, Qt::NoModifier); }
    void zoomOut() { zoom(false, Qt::NoModifier); }
    void rotateUp() { rotate(10.0f, 0.0f); }
    void rotateDown() { rotate(-10.0f, 0.0f); }
    void rotateLeft() { rotate(0.0f, 10.0f); }
    void rotateRight() { rotate(0.0f, -10.0f); }
    void translateUp() { translate(0.0f, 1.0f); }
    void translateDown() { translate(0.0f, -1.0f); }
    void translateLeft() { translate(-1.0f, 0.0f); }
    void translateRight() { translate(1.0f, 0.0f); }
    void revertSelectedModel() { setSelectedModel(0); }
    void setBones(const QList<vpvl::Bone *> &bones);

signals:
    void modelDidAdd(vpvl::PMDModel *model);
    void modelWillDelete(vpvl::PMDModel *model);
    void modelDidMakePose(VPDFile *pose, vpvl::PMDModel *model);
    void motionDidAdd(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void motionDidFinished(const QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> &motions);
    void assetDidAdd(vpvl::Asset *asset);
    void assetWillDelete(vpvl::Asset *asset);
    void cameraMotionDidSet(vpvl::VMDMotion *motion);
    void lightColorDidSet(const btVector4 &color);
    void lightPositionDidSet(const btVector3 &position);
    void modelDidSelect(vpvl::PMDModel *model);
    void cameraPerspectiveDidSet(const btVector3 &pos, const btVector3 &angle, float fovy, float distance);
    void fpsDidUpdate(int fps);
    void sceneDidPlay();
    void sceneDidPause();
    void sceneDidStop();

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void initializeGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintGL();
    void resizeGL(int w, int h);
    void timerEvent(QTimerEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    void drawBones();
    void updateFPS();
    QProgressDialog *getProgressDialog(const QString &label, int max);
    const QString openFileDialog(const QString &name, const QString &desc, const QString &exts);

    vpvl::gl::Renderer *m_renderer;
    vpvl::Bone *m_bone;
    Delegate *m_delegate;
    PlayerWidget *m_player;
    SceneLoader *m_loader;
    World *m_world;
    QSettings *m_settings;
    QElapsedTimer m_timer;
    QPoint m_prevPos;
    int m_frameCount;
    int m_currentFPS;
    int m_defaultFPS;
    int m_interval;
    int m_internalTimerID;
    bool m_visibleBones;
    bool m_playing;
};

#endif // SCENEWIDGET_H

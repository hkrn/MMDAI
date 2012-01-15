/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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
#include <QtCore/QUuid>
#include <QtOpenGL/QtOpenGL>

#include <vpvl/Bone.h>
#include <vpvl/Common.h>

namespace vpvl {
#ifdef VPVL_ENABLE_GLSL
namespace gl2 {
#else
namespace gl {
#endif
class Renderer;
}
class Asset;
class PMDModel;
class Scene;
class VMDMotion;
}

namespace internal {
class DebugDrawer;
class Delegate;
class Grid;
class InfoPanel;
class World;
}

class Handles;
class QGestureEvent;
class QPanGesture;
class QPinchGesture;
class QProgressDialog;
class QSettings;
class QSwipeGesture;
class SceneLoader;
class Script;
class VPDFile;

class SceneWidget : public QGLWidget, protected QGLFunctions
{
    Q_OBJECT

public:
    explicit SceneWidget(QSettings *settings, QWidget *parent = 0);
    ~SceneWidget();

    SceneLoader *sceneLoader() const;
    const vpvl::Scene *scene() const;
    vpvl::Scene *mutableScene();
    vpvl::PMDModel *selectedModel() const;
    void setSelectedModel(vpvl::PMDModel *value);
    void setPreferredFPS(int value);
    void setHandlesVisible(bool value);
    void setInfoPanelVisible(bool value);
    void showSelectedModelEdge();
    void hideSelectedModelEdge();

    vpvl::PMDModel *addModel(const QString &path, bool skipDialog = false);
    vpvl::VMDMotion *insertMotionToAllModels(const QString &path);
    vpvl::VMDMotion *insertMotionToSelectedModel(const QString &path);
    vpvl::VMDMotion *insertMotionToModel(const QString &path, vpvl::PMDModel *model);
    vpvl::Asset *addAsset(const QString &path);
    vpvl::Asset *addAssetFromMetadata(const QString &path);
    VPDFile *insertPoseToSelectedModel(const QString &filename, vpvl::PMDModel *model);
    vpvl::VMDMotion *setCamera(const QString &path);
    void makeRay(const QPointF &input, vpvl::Vector3 &rayFrom, vpvl::Vector3 &rayTo) const;
    Handles *handles() const { return m_handles; }
    vpvl::Bone *selectedBone() const { return m_bone; }
    float modelEdgeOffset() const { return m_selectedEdgeOffset; }
    int preferredFPS() const { return m_defaultFPS; }
    bool isGridVisible() const;
    bool isBoneWireframeVisible() const { return m_visibleBones; }
    bool isPhysicsEnabled() const { return m_enablePhysics; }
    bool isPlaying() const { return m_playing; }
    bool isMoveGestureEnabled() const { return m_enableMoveGesture; }
    bool isRotateGestureEnabled() const { return m_enableRotateGesture; }
    bool isScaleGestureEnabled() const { return m_enableScaleGesture; }
    bool isUndoGestureEnabled() const { return m_enableUndoGesture; }
    bool showModelDialog() const { return m_showModelDialog; }
    const QString openFileDialog(const QString &name, const QString &desc, const QString &exts);

public slots:
    void play();
    void pause();
    void stop();
    void clear();
    void addModel();
    void insertMotionToAllModels();
    void insertMotionToSelectedModel();
    void setEmptyMotion(vpvl::PMDModel *model);
    void addAsset();
    void addAssetFromMetadata();
    void saveMetadataFromAsset(vpvl::Asset *asset);
    void insertPoseToSelectedModel();
    void setCamera();
    void deleteSelectedModel();
    void resetCamera();
    void setLightColor(const vpvl::Color &color);
    void setLightPosition(const vpvl::Vector3 &position);
    void rotateScene(const vpvl::Vector3 &delta);
    void rotateModel(vpvl::PMDModel *model, const vpvl::Quaternion &delta);
    void translateScene(const vpvl::Vector3 &delta);
    void translateModel(vpvl::PMDModel *model, const vpvl::Vector3 &delta);
    void resetModelPosition();
    void advanceMotion(float frameIndex);
    void seekMotion(float frameIndex);
    void setCameraPerspective(vpvl::Vector3 *pos, vpvl::Vector3 *angle, float *fovy, float *distance);
    void setGridVisible(bool value);
    void setPhysicsEnable(bool value);
    void zoom(bool up, const Qt::KeyboardModifiers &modifiers);
    void selectBones(const QList<vpvl::Bone *> &bones);
    void loadFile(const QString &file);

    void zoomIn() { zoom(true, Qt::NoModifier); }
    void zoomOut() { zoom(false, Qt::NoModifier); }
    void rotateUp() { rotateScene(vpvl::Vector3(10.0f, 0.0f, 0.0f)); }
    void rotateDown() { rotateScene(vpvl::Vector3(-10.0f, 0.0f, 0.0f)); }
    void rotateLeft() { rotateScene(vpvl::Vector3(0.0f, 10.0f, 0.0f)); }
    void rotateRight() { rotateScene(vpvl::Vector3(0.0f, -10.0f, 0.0f)); }
    void translateUp() { translateScene(vpvl::Vector3(0.0f, 1.0f, 0.0f)); }
    void translateDown() { translateScene(vpvl::Vector3(0.0f, -1.0f, 0.0f)); }
    void translateLeft() { translateScene(vpvl::Vector3(-1.0f, 0.0f, 0.0f)); }
    void translateRight() { translateScene(vpvl::Vector3(1.0f, 0.0f, 0.0f)); }
    void translateModelUp() { translateModel(vpvl::Vector3(0.0f, 0.5f, 0.0f)); }
    void translateModelDown() { translateModel(vpvl::Vector3(0.0f, -0.5f, 0.0f)); }
    void translateModelLeft() { translateModel(vpvl::Vector3(-0.5f, 0.0f, 0.0f)); }
    void translateModelRight() { translateModel(vpvl::Vector3(0.5f, 0.0f, 0.0f)); }
    void revertSelectedModel() { setSelectedModel(0); }
    void rotateModel(const vpvl::Quaternion &delta) { rotateModel(selectedModel(), delta); }
    void translateModel(const vpvl::Vector3 &delta) { translateModel(selectedModel(), delta); }
    void updateMotion() { seekMotion(m_frameIndex); }
    void setModelEdgeOffset(float value) { m_selectedEdgeOffset = value; }
    void setBoneWireframeVisible(bool value) { m_visibleBones = value; }
    void setShowModelDialog(bool value) { m_showModelDialog = value; }
    void setMoveGestureEnable(bool value) { m_enableMoveGesture = value; }
    void setRotateGestureEnable(bool value) { m_enableRotateGesture = value; }
    void setScaleGestureEnable(bool value) { m_enableScaleGesture = value; }
    void setUndoGestureEnable(bool value) { m_enableUndoGesture = value; }

signals:
    void initailizeGLContextDidDone();
    void fileDidLoad(const QString &filename);
    void newMotionDidSet(vpvl::PMDModel *model);
    void lightColorDidSet(const vpvl::Color &color);
    void lightPositionDidSet(const vpvl::Vector3 &position);
    void modelDidSelect(vpvl::PMDModel *model);
    void modelDidMove(const vpvl::Vector3 &lastPosition);
    void modelDidRotate(const vpvl::Quaternion &lastRotation);
    void cameraPerspectiveDidSet(const vpvl::Vector3 &pos, const vpvl::Vector3 &angle, float fovy, float distance);
    void fpsDidUpdate(int fps);
    void sceneDidPlay();
    void sceneDidPause();
    void sceneDidStop();
    void handleDidGrab();
    void handleDidRelease();
    void handleDidMove(const vpvl::Vector3 &delta, vpvl::Bone *bone, int mode);
    void handleDidRotate(const vpvl::Quaternion &delta, vpvl::Bone *bone, int mode, bool minus);
    void boneDidSelect(const QList<vpvl::Bone *> &bones);
    void motionDidSeek(float frameIndex);
    void undoDidRequest();
    void redoDidRequest();

protected:
    bool event(QEvent *event);
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void initializeGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintGL();
    void resizeGL(int w, int h);
    void timerEvent(QTimerEvent *event);
    void wheelEvent(QWheelEvent *event);
    bool gestureEvent(QGestureEvent *event);
    void panTriggered(QPanGesture *event);
    void pinchTriggered(QPinchGesture *event);
    void swipeTriggered(QSwipeGesture *event);

#ifdef VPVL_ENABLE_GLSL
    vpvl::gl2::Renderer *m_renderer;
#else
    vpvl::gl::Renderer *m_renderer;
#endif
    internal::Delegate *m_delegate;
    internal::World *m_world;
    SceneLoader *m_loader;

private:
    bool acceptAddingModel(vpvl::PMDModel *model);
    void updateFPS();
    void changeCursorIfHandlesHit(const QPointF &pos);
    void grabImageHandle(const QPointF &diff);
    void grabModelHandleByRaycast(const QPointF &pos);

    internal::DebugDrawer *m_debugDrawer;
    internal::Grid *m_grid;
    internal::InfoPanel *m_info;
    vpvl::Bone *m_bone;
    Handles *m_handles;
    QSettings *m_settings;
    QElapsedTimer m_timer;
    float m_lastDistance;
    float m_prevElapsed;
    float m_selectedEdgeOffset;
    float m_frameIndex;
    int m_frameCount;
    int m_currentFPS;
    int m_defaultFPS;
    int m_interval;
    int m_internalTimerID;
    int m_handleFlags;
    bool m_visibleBones;
    bool m_playing;
    bool m_enableBoneMove;
    bool m_enableBoneRotate;
    bool m_enablePhysics;
    bool m_showModelDialog;
    bool m_lockTouchEvent;
    bool m_enableMoveGesture;
    bool m_enableRotateGesture;
    bool m_enableScaleGesture;
    bool m_enableUndoGesture;

    Q_DISABLE_COPY(SceneWidget)
};

#endif // SCENEWIDGET_H

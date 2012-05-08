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
#include "VPDFile.h"

#include <vpvl2/Common.h>
#include <vpvl2/Scene.h>

namespace vpvl2 {
class Factory;
class IBone;
class IEncoding;
class IModel;
class IMotion;
class Scene;
}

namespace internal {
class DebugDrawer;
class Delegate;
class Grid;
class InfoPanel;
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
    enum EditMode {
        kSelect,
        kRotate,
        kMove
    };

    explicit SceneWidget(vpvl2::IEncoding *encoding, vpvl2::Factory *factory, QSettings *settings, QWidget *parent = 0);
    ~SceneWidget();

    SceneLoader *sceneLoader() const;
    void setSelectedModel(vpvl2::IModel *value);
    void setWorldGravity(const vpvl2::Vector3 &value);
    void setPreferredFPS(int value);
    void setHandlesVisible(bool value);
    void setInfoPanelVisible(bool value);
    void setBoneWireFramesVisible(bool value);
    void startAutomaticRendering();
    void stopAutomaticRendering();
    void loadProject(const QString &filename);
    void saveProject(const QString &filename);

    vpvl2::IModel *addModel(const QString &path, bool skipDialog = false);
    vpvl2::IMotion *insertMotionToAllModels(const QString &path);
    vpvl2::IMotion *insertMotionToSelectedModel(const QString &path);
    vpvl2::IMotion *insertMotionToModel(const QString &path, vpvl2::IModel *model);
    vpvl2::IModel *addAsset(const QString &path);
    vpvl2::IModel *addAssetFromMetadata(const QString &path);
    VPDFilePtr insertPoseToSelectedModel(const QString &filename, vpvl2::IModel *model);
    vpvl2::IMotion *setCamera(const QString &path);
    void makeRay(const QPointF &input, vpvl2::Vector3 &rayFrom, vpvl2::Vector3 &rayTo) const;
    Handles *handles() const { return m_handles; }
    EditMode editMode() const { return m_editMode; }
    const QList<vpvl2::IBone *> &selectedBones() const { return m_bones; }
    float currentFrameIndex() const { return m_frameIndex; }
    bool isPlaying() const { return m_playing; }
    bool isMoveGestureEnabled() const { return m_enableMoveGesture; }
    bool isRotateGestureEnabled() const { return m_enableRotateGesture; }
    bool isScaleGestureEnabled() const { return m_enableScaleGesture; }
    bool isUndoGestureEnabled() const { return m_enableUndoGesture; }
    bool showModelDialog() const { return m_showModelDialog; }

public slots:
    void play();
    void pause();
    void stop();
    void clear();
    void setEmptyMotion();
    void insertMotionToAllModels();
    void insertMotionToSelectedModel();
    void deleteSelectedModel();
    void loadFile(const QString &file);
    void setEmptyMotion(vpvl2::IModel *model);
    void saveMetadataFromAsset(vpvl2::IModel *asset);
    void rotateScene(const vpvl2::Vector3 &delta);
    void rotateModel(const vpvl2::Quaternion &delta);
    void rotateModel(vpvl2::IModel *model, const vpvl2::Quaternion &delta);
    void translateScene(const vpvl2::Vector3 &delta);
    void translateModel(const vpvl2::Vector3 &delta);
    void translateModel(vpvl2::IModel *model, const vpvl2::Vector3 &delta);
    void advanceMotion(float delta);
    void seekMotion(float frameIndex, bool forceCameraUpdate);
    void resetMotion();
    void setCameraPerspective(const QSharedPointer<vpvl2::Scene::ICamera> &camera);
    void setModelEdgeOffset(double value);
    void setModelEdgeColor(const QColor &color);
    void setModelPositionOffset(const vpvl2::Vector3 &value);
    void setModelRotationOffset(const vpvl2::Vector3 &value);
    void setModelProjectiveShadowEnable(bool value);
    void setModelSelfShadowEnable(bool value);
    void selectBones(const QList<vpvl2::IBone *> &bones);
    void setEditMode(SceneWidget::EditMode value);

signals:
    void initailizeGLContextDidDone();
    void fileDidLoad(const QString &filename);
    void newMotionDidSet(vpvl2::IModel *model);
    void modelDidMove(const vpvl2::Vector3 &lastPosition);
    void modelDidRotate(const vpvl2::Quaternion &lastRotation);
    void cameraPerspectiveDidSet(const vpvl2::Scene::ICamera *camera);
    void fpsDidUpdate(int fps);
    void sceneDidPlay();
    void sceneDidPause();
    void sceneDidStop();
    void handleDidGrab();
    void handleDidRelease();
    void handleDidMoveAbsolute(const vpvl2::Vector3 &position, vpvl2::IBone *bone, int mode);
    void handleDidMoveRelative(const vpvl2::Vector3 &position, vpvl2::IBone *bone, int mode);
    void handleDidRotate(const vpvl2::Scalar &angle, vpvl2::IBone *bone, int mode);
    void bonesDidSelect(const QList<vpvl2::IBone *> &bones);
    void motionDidSeek(float frameIndex);
    void undoDidRequest();
    void redoDidRequest();

protected slots:
    void setShowModelDialog(bool value) { m_showModelDialog = value; }

protected:
    bool event(QEvent *event);
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void initializeGL();
    void mousePressEvent(QMouseEvent *event);
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

    SceneLoader *m_loader;
    QSettings *m_settings;

private slots:
    void addModel();
    void addAsset();
    void addAssetFromMetadata();
    void insertPoseToSelectedModel();
    void setCamera();
    void resetCamera();
    void resetModelPosition();
    void zoom(bool up, const Qt::KeyboardModifiers &modifiers);
    void openErrorDialogIfFailed(bool loadingProjectFailed);
    void zoomIn() { zoom(true, Qt::NoModifier); }
    void zoomOut() { zoom(false, Qt::NoModifier); }
    void rotateUp() { rotateScene(vpvl2::Vector3(10.0f, 0.0f, 0.0f)); }
    void rotateDown() { rotateScene(vpvl2::Vector3(-10.0f, 0.0f, 0.0f)); }
    void rotateLeft() { rotateScene(vpvl2::Vector3(0.0f, 10.0f, 0.0f)); }
    void rotateRight() { rotateScene(vpvl2::Vector3(0.0f, -10.0f, 0.0f)); }
    void translateUp() { translateScene(vpvl2::Vector3(0.0f, 1.0f, 0.0f)); }
    void translateDown() { translateScene(vpvl2::Vector3(0.0f, -1.0f, 0.0f)); }
    void translateLeft() { translateScene(vpvl2::Vector3(-1.0f, 0.0f, 0.0f)); }
    void translateRight() { translateScene(vpvl2::Vector3(1.0f, 0.0f, 0.0f)); }
    void translateModelUp() { translateModel(vpvl2::Vector3(0.0f, 0.5f, 0.0f)); }
    void translateModelDown() { translateModel(vpvl2::Vector3(0.0f, -0.5f, 0.0f)); }
    void translateModelLeft() { translateModel(vpvl2::Vector3(-0.5f, 0.0f, 0.0f)); }
    void translateModelRight() { translateModel(vpvl2::Vector3(0.5f, 0.0f, 0.0f)); }
    void revertSelectedModel() { setSelectedModel(0); }
    void refreshScene() { seekMotion(m_frameIndex, true); }
    void refreshMotions() { seekMotion(m_frameIndex, false); }
    void setMoveGestureEnable(bool value) { m_enableMoveGesture = value; }
    void setRotateGestureEnable(bool value) { m_enableRotateGesture = value; }
    void setScaleGestureEnable(bool value) { m_enableScaleGesture = value; }
    void setUndoGestureEnable(bool value) { m_enableUndoGesture = value; }

private:
    void clearSelectedBones();
    void updateScene();
    bool acceptAddingModel(vpvl2::IModel *model);
    bool testHitModelHandle(const QPointF &pos);
    void updateFPS();
    void grabImageHandle(const vpvl2::Scalar &deltaValue);
    void grabModelHandleByRaycast(const QPointF &pos, const QPointF &diff, int flags);

    vpvl2::IEncoding *m_encoding;
    vpvl2::Factory *m_factory;
    internal::DebugDrawer *m_debugDrawer;
    internal::Grid *m_grid;
    internal::InfoPanel *m_info;
    Handles *m_handles;
    QList<vpvl2::IBone *> m_bones;
    QElapsedTimer m_timer;
    QPointF m_clickOrigin;
    EditMode m_editMode;
    vpvl2::Scalar m_totalDelta;
    float m_lastDistance;
    float m_prevElapsed;
    float m_frameIndex;
    int m_frameCount;
    int m_currentFPS;
    int m_interval;
    int m_internalTimerID;
    int m_handleFlags;
    bool m_playing;
    bool m_enableBoneMove;
    bool m_enableBoneRotate;
    bool m_showModelDialog;
    bool m_lockTouchEvent;
    bool m_enableMoveGesture;
    bool m_enableRotateGesture;
    bool m_enableScaleGesture;
    bool m_enableUndoGesture;
    bool m_isImageHandleRectIntersect;

    Q_DISABLE_COPY(SceneWidget)
};

#endif // SCENEWIDGET_H

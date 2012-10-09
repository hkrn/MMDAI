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

#ifndef VPVM_SCENEWIDGET_H
#define VPVM_SCENEWIDGET_H

#include <QtCore/QElapsedTimer>
#include <QtCore/QUuid>
#include <QtOpenGL/QGLFunctions>
#include <QtOpenGL/QGLWidget>
#include "VPDFile.h"

#include <vpvl2/Common.h>
#include <vpvl2/IKeyframe.h>
#include <vpvl2/Scene.h>

namespace vpvl2 {
class Factory;
class IBone;
class IEncoding;
class IModel;
class IMotion;
}

class QGestureEvent;
class QPanGesture;
class QPinchGesture;
class QProgressDialog;
class QSettings;
class QSwipeGesture;

namespace vpvm
{

using namespace vpvl2;

class BackgroundImage;
class DebugDrawer;
class Grid;
class InfoPanel;
class Handles;
class SceneLoader;
class VPDFile;

class SceneWidget : public QGLWidget, protected QGLFunctions
{
    Q_OBJECT

public:
    enum EditMode {
        kNone,
        kSelect,
        kRotate,
        kMove
    };

    explicit SceneWidget(IEncoding *encoding, Factory *factory, QSettings *settings, QWidget *parent = 0);
    ~SceneWidget();

    SceneLoader *sceneLoader() const;
    void setWorldGravity(const Vector3 &value);
    void setPreferredFPS(int value);
    void setHandlesVisible(bool value);
    void setInfoPanelVisible(bool value);
    void setBoneWireFramesVisible(bool value);
    void startAutomaticRendering();
    void stopAutomaticRendering();
    void loadProject(const QString &filename);
    void saveProject(const QString &filename);

    IModel *addModel(const QString &path, bool skipDialog = false);
    IMotion *insertMotionToAllModels(const QString &path);
    IMotion *insertMotionToSelectedModel(const QString &path);
    IMotion *insertMotionToModel(const QString &path, IModel *model);
    IModel *addAsset(const QString &path);
    IModel *addAssetFromMetadata(const QString &path);
    VPDFilePtr insertPoseToSelectedModel(const QString &filename, IModel *model);
    IMotion *setCamera(const QString &path);
    void makeRay(const QPointF &input, Vector3 &rayFrom, Vector3 &rayTo) const;
    Handles *handles() const { return m_handles; }
    EditMode editMode() const { return m_editMode; }
    const QList<IBone *> &selectedBones() const { return m_selectedBones; }
    const IKeyframe::TimeIndex &currentTimeIndex() const { return m_timeIndex; }
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
    void setEmptyMotion(IModel *model);
    void saveMetadataFromAsset(IModel *asset);
    void rotateScene(const Vector3 &delta);
    void rotateModel(const Quaternion &delta);
    void rotateModel(IModel *model, const Quaternion &delta);
    void translateScene(const Vector3 &delta);
    void translateModel(const Vector3 &delta);
    void translateModel(IModel *model, const Vector3 &delta);
    void advanceMotion(const IKeyframe::TimeIndex &delta);
    void seekMotion(const IKeyframe::TimeIndex &timeIndex, bool forceCameraUpdate, bool forceEvenSame);
    void resetMotion();
    void setCameraPerspective(const QSharedPointer<ICamera> &camera);
    void setModelEdgeOffset(double value);
    void setModelOpacity(const Scalar &value);
    void setModelEdgeColor(const QColor &color);
    void setModelPositionOffset(const Vector3 &value);
    void setModelRotationOffset(const Vector3 &value);
    void setModelProjectiveShadowEnable(bool value);
    void setModelSelfShadowEnable(bool value);
    void setModelOpenSkinningEnable(bool value);
    void setModelVertexShaderSkinningType1Enable(bool value);
    void selectBones(const QList<IBone *> &bones);
    void setEditMode(SceneWidget::EditMode value);
    void setSelectedModel(IModel *value);
    void setBackgroundImage(const QString &filename);

signals:
    void initailizeGLContextDidDone();
    void fileDidLoad(const QString &filename);
    void newMotionDidSet(IModel *model);
    void modelDidMove(const Vector3 &lastPosition);
    void modelDidRotate(const Quaternion &lastRotation);
    void cameraPerspectiveDidSet(const ICamera *camera);
    void fpsDidUpdate(int fps);
    void sceneDidPlay();
    void sceneDidPause();
    void sceneDidStop();
    void handleDidGrab();
    void handleDidRelease();
    void handleDidMoveAbsolute(const Vector3 &position, IBone *bone, int mode);
    void handleDidMoveRelative(const Vector3 &position, IBone *bone, int mode);
    void handleDidRotate(const Scalar &angle, IBone *bone, int mode);
    void bonesDidSelect(const QList<IBone *> &bones);
    void motionDidSeek(const IKeyframe::TimeIndex &timeIndex);
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
    BackgroundImage *m_background;

private slots:
    void addModel();
    void addAsset();
    void addAssetFromMetadata();
    void insertPoseToSelectedModel();
    void setBackgroundImage();
    void setBackgroundPosition(const QPoint &value);
    void setBackgroundImageUniformEnable(bool value);
    void clearBackgroundImage();
    void setCamera();
    void resetCamera();
    void resetModelPosition();
    void updatePlaneWorld(const ICamera *camera);
    void renderBackgroundObjects();
    void zoom(bool up, const Qt::KeyboardModifiers &modifiers);
    void openErrorDialogIfFailed(bool loadingProjectFailed);
    void zoomIn() { zoom(true, Qt::NoModifier); }
    void zoomOut() { zoom(false, Qt::NoModifier); }
    void rotateUp() { rotateScene(Vector3(10.0f, 0.0f, 0.0f)); }
    void rotateDown() { rotateScene(Vector3(-10.0f, 0.0f, 0.0f)); }
    void rotateLeft() { rotateScene(Vector3(0.0f, 10.0f, 0.0f)); }
    void rotateRight() { rotateScene(Vector3(0.0f, -10.0f, 0.0f)); }
    void translateUp() { translateScene(Vector3(0.0f, 1.0f, 0.0f)); }
    void translateDown() { translateScene(Vector3(0.0f, -1.0f, 0.0f)); }
    void translateLeft() { translateScene(Vector3(-1.0f, 0.0f, 0.0f)); }
    void translateRight() { translateScene(Vector3(1.0f, 0.0f, 0.0f)); }
    void translateModelUp() { translateModel(Vector3(0.0f, 0.5f, 0.0f)); }
    void translateModelDown() { translateModel(Vector3(0.0f, -0.5f, 0.0f)); }
    void translateModelLeft() { translateModel(Vector3(-0.5f, 0.0f, 0.0f)); }
    void translateModelRight() { translateModel(Vector3(0.5f, 0.0f, 0.0f)); }
    void revertSelectedModel() { setSelectedModel(0); }
    void refreshScene() { seekMotion(m_timeIndex, true, false); }
    void refreshMotions() { seekMotion(m_timeIndex, false, false); }
    void setMoveGestureEnable(bool value) { m_enableMoveGesture = value; }
    void setRotateGestureEnable(bool value) { m_enableRotateGesture = value; }
    void setScaleGestureEnable(bool value) { m_enableScaleGesture = value; }
    void setUndoGestureEnable(bool value) { m_enableUndoGesture = value; }

private:
    class PlaneWorld;
    void clearSelectedBones();
    void updateScene();
    bool acceptAddingModel(IModel *model);
    bool testHitModelHandle(const QPointF &pos);
    void updateFPS();
    void grabImageHandle(const Scalar &deltaValue);
    void grabModelHandleByRaycast(const QPointF &pos,
                                  const QPointF &diff,
                                  int flags);
    IBone *findNearestBone(const IModel *model,
                           const Vector3 &znear,
                           const Vector3 &zfar,
                           const Scalar &threshold) const;
    bool intersectsBone(const IBone *bone,
                        const Vector3 &znear,
                        const Vector3 &zfar,
                        const Scalar &threshold) const;

    IEncoding *m_encoding;
    Factory *m_factory;
    IBone *m_currentSelectedBone;
    Vector3 m_lastBonePosition;
    Scalar m_totalDelta;
    DebugDrawer *m_debugDrawer;
    Grid *m_grid;
    InfoPanel *m_info;
    PlaneWorld *m_plane;
    Handles *m_handles;
    QList<IBone *> m_selectedBones;
    QElapsedTimer m_timer;
    QPointF m_clickOrigin;
    QPointF m_delta;
    EditMode m_editMode;
    float m_lastDistance;
    float m_prevElapsed;
    IKeyframe::TimeIndex m_timeIndex;
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
    bool m_enableUpdateGL;
    bool m_isImageHandleRectIntersect;

    Q_DISABLE_COPY(SceneWidget)
};

} /* namespace vpvm */

#endif // SCENEWIDGET_H

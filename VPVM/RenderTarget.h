/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef SCENEITEM_H
#define SCENEITEM_H

#include <QAudioDecoder>
#include <QElapsedTimer>
#include <QImage>
#include <QMatrix4x4>
#include <QMediaPlayer>
#include <QProcess>
#include <QQuickItem>
#include <QQmlPropertyMap>
#include <vpvl2/extensions/icu4c/StringMap.h>
#include <vpvl2/extensions/FPSCounter.h>
#include <glm/glm.hpp>

#include "ModelProxy.h"

namespace vpvl2 {
class IEncoding;
class IRenderContext;
class Factory;
class Scene;
}

class GraphicsDevice;
class Grid;
class QOpenGLFramebufferObject;
class QOpenGLShaderProgram;
class QOpenGLVertexArrayObject;
class QTemporaryDir;
class ProjectProxy;
class IGizmo;

class RenderTarget : public QQuickItem
{
    Q_OBJECT

    Q_ENUMS(EditModeType)
    Q_FLAGS(VisibleGizmoMask VisibleGizmoMasks)
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged FINAL)
    Q_PROPERTY(bool playing READ isPlaying WRITE setPlaying NOTIFY playingChanged FINAL)
    Q_PROPERTY(bool dirty READ isDirty WRITE setDirty NOTIFY dirtyChanged FINAL)
    Q_PROPERTY(bool grabbingGizmo READ grabbingGizmo NOTIFY grabbingGizmoChanged FINAL)
    Q_PROPERTY(bool enableSnapGizmo READ isSnapGizmoEnabled WRITE setSnapGizmoEnabled NOTIFY enableSnapGizmoChanged FINAL)
    Q_PROPERTY(qreal currentTimeIndex READ currentTimeIndex WRITE setCurrentTimeIndex NOTIFY currentTimeIndexChanged FINAL)
    Q_PROPERTY(qreal lastTimeIndex READ lastTimeIndex WRITE setLastTimeIndex NOTIFY lastTimeIndexChanged FINAL)
    Q_PROPERTY(qreal currentFPS READ currentFPS NOTIFY currentFPSChanged FINAL)
    Q_PROPERTY(QRect viewport READ viewport WRITE setViewport NOTIFY viewportChanged FINAL)
    Q_PROPERTY(QUrl audioUrl READ audioUrl WRITE setAudioUrl NOTIFY audioUrlChanged FINAL)
    Q_PROPERTY(QUrl videoUrl READ videoUrl WRITE setVideoUrl NOTIFY videoUrlChanged FINAL)
    Q_PROPERTY(QVector3D shadowMapSize READ shadowMapSize WRITE setShadowMapSize NOTIFY shadowMapSizeChanged)
    Q_PROPERTY(QVector3D snapGizmoStepSize READ snapGizmoStepSize WRITE setSnapGizmoStepSize NOTIFY snapGizmoStepSizeChanged FINAL)
    Q_PROPERTY(QMatrix4x4 viewMatrix READ viewMatrix NOTIFY viewMatrixChanged FINAL)
    Q_PROPERTY(QMatrix4x4 projectionMatrix READ projectionMatrix NOTIFY projectionMatrixChanged FINAL)
    Q_PROPERTY(GraphicsDevice *graphicsDevice READ graphicsDevice NOTIFY graphicsDeviceChanged FINAL)
    Q_PROPERTY(ProjectProxy *project READ projectProxy WRITE setProjectProxy FINAL)
    Q_PROPERTY(Grid *grid READ grid CONSTANT FINAL)
    Q_PROPERTY(EditModeType editMode READ editMode WRITE setEditMode NOTIFY editModeChanged FINAL)
    Q_PROPERTY(VisibleGizmoMasks visibleGizmoMasks READ visibleGizmoMasks WRITE setVisibleGizmoMasks NOTIFY visibleGizmoMasksChanged FINAL)

public:
    static const QVector3D kDefaultShadowMapSize;

    enum EditModeType {
        SelectMode,
        MoveMode,
        RotateMode
    };
    enum VisibleGizmoMask {
        AxisX = 0x1,
        AxisY = 0x2,
        AxisZ = 0x4,
        AxisTrackBall = 0x8,
        AxisScreen = 0x16
    };
    Q_DECLARE_FLAGS(VisibleGizmoMasks, VisibleGizmoMask)

    explicit RenderTarget(QQuickItem *parent = 0);
    ~RenderTarget();

    Q_INVOKABLE bool handleMousePress(int x, int y);
    Q_INVOKABLE void handleMouseMove(int x, int y);
    Q_INVOKABLE void handleMouseRelease(int x, int y);
    Q_INVOKABLE void toggleRunning(bool value);

    bool isInitialized() const;
    qreal currentTimeIndex() const;
    void setCurrentTimeIndex(qreal value);
    qreal lastTimeIndex() const;
    void setLastTimeIndex(qreal value);
    qreal currentFPS() const;
    ProjectProxy *projectProxy() const;
    Grid *grid() const;
    void setProjectProxy(ProjectProxy *value);
    bool isPlaying() const;
    void setPlaying(bool value);
    bool isDirty() const;
    void setDirty(bool value);
    bool isSnapGizmoEnabled() const;
    void setSnapGizmoEnabled(bool value);
    bool grabbingGizmo() const;
    QRect viewport() const;
    void setViewport(const QRect &value);
    QVector3D shadowMapSize() const;
    void setShadowMapSize(const QVector3D &value);
    QUrl audioUrl() const;
    void setAudioUrl(const QUrl &value);
    QUrl videoUrl() const;
    void setVideoUrl(const QUrl &value);
    EditModeType editMode() const;
    void setEditMode(EditModeType value);
    VisibleGizmoMasks visibleGizmoMasks() const;
    void setVisibleGizmoMasks(VisibleGizmoMasks value);
    QVector3D snapGizmoStepSize() const;
    void setSnapGizmoStepSize(const QVector3D &value);
    QVector3D snapOrientationGizmoStepSize() const;
    void setSnapOrientationGizmoStepSize(const QVector3D &value);
    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;
    GraphicsDevice *graphicsDevice() const;

public slots:
    Q_INVOKABLE void update();
    Q_INVOKABLE void render();
    Q_INVOKABLE void exportImage(const QUrl &fileUrl, const QSize &size);
    Q_INVOKABLE void exportVideo(const QUrl &fileUrl, const QSize &size, const QString &videoType, const QString &frameImageType);
    Q_INVOKABLE void cancelExportingVideo();
    void resetCurrentTimeIndex();
    void resetLastTimeIndex();

signals:
    void initializedChanged();
    void currentTimeIndexChanged();
    void currentFPSChanged();
    void lastTimeIndexChanged();
    void playingChanged();
    void dirtyChanged();
    void grabbingGizmoChanged();
    void enableSnapGizmoChanged();
    void snapGizmoStepSizeChanged();
    void snapOrientationGizmoStepSizeChanged();
    void viewportChanged();
    void shadowMapSizeChanged();
    void audioUrlChanged();
    void videoUrlChanged();
    void viewMatrixChanged();
    void projectionMatrixChanged();
    void editModeChanged();
    void visibleGizmoMasksChanged();
    void graphicsDeviceChanged();
    void errorDidHappen(const QString &message);
    void modelDidUpload(ModelProxy *model);
    void allModelsDidUpload();
    void allModelsDidDelete();
    void renderWillPerform();
    void renderDidPerform();
    void videoFrameDidSave(const qreal &current, const qreal &duration);
    void encodeDidBegin();
    void encodeDidProceed(quint64 proceed, quint64 estimated);
    void encodeDidFinish(bool isNormalExit);
    void encodeDidCancel();

protected slots:
    void handleWindowChange(QQuickWindow *window);

private slots:
    void draw();
    void drawOffscreenForImage();
    void drawOffscreenForVideo();
    void writeExportedImage();
    void launchEncodingTask();
    void prepareSyncMotionState();
    void prepareUpdatingLight();
    void syncExplicit();
    void syncMotionState();
    void syncImplicit();
    void initialize();
    void release();
    void uploadModelAsync(ModelProxy *model);
    void deleteModelAsync(ModelProxy *model);
    void performUploadingEnqueuedModels();
    void performDeletingEnqueuedModels();
    void performUpdatingLight();
    void prepareProject();
    void activateProject();
    void markDirty();
    void updateGizmo();
    void seekMediaFromProject();
    void handleAudioDecoderError(QAudioDecoder::Error error);
    void handleMediaPlayerError(QMediaPlayer::Error error);

private:
    class ApplicationContext;
    class DebugDrawer;
    class EncodingTask;
    class ModelDrawer;
    class VideoSurface;
    QMediaPlayer *mediaPlayer() const;
    EncodingTask *encodingTask() const;
    IGizmo *translationGizmo() const;
    IGizmo *orientationGizmo() const;
    void clearScene();
    void drawShadowMap();
    void drawScene();
    void drawDebug();
    void drawModelBones();
    void drawCurrentGizmo();
    void drawOffscreen(QOpenGLFramebufferObject *fbo);
    void updateViewport();
    void seekVideo(const qreal &value);

    QScopedPointer<ApplicationContext> m_applicationContext;
    mutable QScopedPointer<DebugDrawer> m_debugDrawer;
    mutable QScopedPointer<EncodingTask> m_encodingTask;
    mutable QScopedPointer<ModelDrawer> m_modelDrawer;
    mutable QScopedPointer<VideoSurface> m_videoSurface;
    mutable QScopedPointer<IGizmo> m_translationGizmo;
    mutable QScopedPointer<IGizmo> m_orientationGizmo;
    mutable QScopedPointer<QMediaPlayer> m_mediaPlayer;
    QScopedPointer<Grid> m_grid;
    QScopedPointer<GraphicsDevice> m_graphicsDevice;
    QElapsedTimer m_renderTimer;
    QSize m_exportSize;
    QUrl m_exportLocation;
    QImage m_exportImage;
    QVector3D m_shadowMapSize;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewProjectionMatrix;
    vpvl2::extensions::icu4c::StringMap m_config;
    vpvl2::extensions::FPSCounter m_counter;
    EditModeType m_editMode;
    ProjectProxy *m_projectProxyRef;
    IGizmo *m_currentGizmoRef;
    QRect m_viewport;
    QMatrix4x4 m_editMatrix;
    qreal m_lastTimeIndex;
    qreal m_currentTimeIndex;
    QVector3D m_snapStepSize;
    VisibleGizmoMasks m_visibleGizmoMasks;
    bool m_grabbingGizmo;
    bool m_playing;
    bool m_dirty;
};

#endif // SCENEITEM_H

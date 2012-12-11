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

#ifndef VPVM_SCENELOADER_H
#define VPVM_SCENELOADER_H

#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QUuid>
#include <QtGui/QColor>
#include <QtGui/QMatrix4x4>
#include <QtOpenGL/QtOpenGL>

#include "VPDFile.h"

#include <vpvl2/IEffect.h>
#include <vpvl2/IModel.h>
#include <vpvl2/Common.h>
#include <vpvl2/Project.h>

namespace vpvl2 {
class IMotion;
class IRenderContext;
class Project;
namespace qt {
class Archive;
class RenderContext;
class World;
}
}

class QGLFramebufferObject;

namespace vpvm
{

using namespace vpvl2;
typedef QScopedPointer<IModel> IModelPtr;
typedef QScopedPointer<IMotion> IMotionPtr;
typedef QScopedPointer<IRenderEngine> IRenderEnginePtr;
typedef QScopedPointer<Project> ProjectPtr;

class SceneLoader : public QObject
{
    Q_OBJECT

public:
    static const QRegExp kAllLoadable;
    static const QRegExp kAllExtensions;
    static const QRegExp kAssetLoadable;
    static const QRegExp kAssetExtensions;
    static const QRegExp kModelLoadable;
    static const QRegExp kModelExtensions;

    explicit SceneLoader(IEncoding *encodingRef, Factory *factoryRef, qt::RenderContext *renderContextRef);
    ~SceneLoader();

    QList<IModel *> allModels() const;
    void addModel(IModel *model,  const QFileInfo &finfo, const QFileInfo &entry, QUuid &uuid);
    void bindDepthTexture();
    void deleteModel(IModel *&model);
    IModel *findAsset(const QUuid &uuid) const;
    IModel *findModel(const QUuid &uuid) const;
    IMotion *findMotion(const QUuid &uuid) const;
    const QUuid findUUID(const IModel *model) const;
    void getBoundingSphere(Vector3 &center, Scalar &radius) const;
    void getCameraMatrices(QMatrix4x4 &worldRef, QMatrix4x4 &view, QMatrix4x4 &projection) const;
    bool isProjectModified() const;
    bool loadAsset(const QString &filename, QUuid &uuid, IModelPtr &assetPtr);
    bool loadAsset(const QByteArray &bytes, const QFileInfo &finfo, const QFileInfo &entry, QUuid &uuid, IModelPtr &assetPtr);
    bool loadAssetFromMetadata(const QString &baseName, const QDir &dir, QUuid &uuid, IModelPtr &modelPtr);
    bool loadCameraMotion(const QString &path, IMotionPtr &motionPtr);
    bool loadModel(const QString &filename, IModelPtr &modelPtr);
    bool loadModel(const QByteArray &bytes, IModel::Type type, IModelPtr &modelPtr);
    bool loadModelMotion(const QString &path, IMotionPtr &motionPtr);
    bool loadModelMotion(const QString &path, QList<IModel *> &models, IMotionPtr &motionPtr);
    bool loadModelMotion(const QString &path, IModel *model, IMotionPtr &motionPtr);
    VPDFilePtr loadModelPose(const QString &path, IModel *model);
    void newCameraMotion(IMotionPtr &motionPtr) const;
    void newModelMotion(const IModel *model, IMotionPtr &motionPtr) const;
    void newProject(ProjectPtr &projectPtr);
    void releaseProject();
    void releaseDepthTexture();
    void renderWindow();
    void renderOffscreen(const QSize &size);
    void renderZPlot();
    void renderZPlotToTexture();
    void setLightViewProjectionMatrix();
    void setMousePosition(const QMouseEvent *event, const QRect &geometry);
    void updateMatrices(const QSizeF &size);
    void updateDepthBuffer(const QSize &value);
    const QList<QUuid> renderOrderList() const;
    const QMatrix4x4 &projectionMatrix() const { return m_projection; }

    bool isGridVisible() const;
    bool isPhysicsEnabled() const;
    const Vector3 worldGravity() const;
    unsigned long worldRandSeed() const;
    const QColor screenColor() const;
    int timeIndexPlayFrom() const;
    int timeIndexPlayTo() const;
    int sceneFPSForPlay() const;
    int timeIndexEncodeVideoFrom() const;
    int timeIndexEncodeVideoTo() const;
    int sceneFPSForEncodeVideo() const;
    int sceneWidth() const;
    int sceneHeight() const;
    bool isLoop() const;
    bool isGridIncluded() const;
    const QString backgroundAudio() const;
    const QSize shadowMapSize() const;
    const Vector4 shadowBoundingSphere() const;
    bool isSoftShadowEnabled() const;
    const QString backgroundImage() const;
    const QPoint backgroundImagePosition() const;
    bool isBackgroundImageUniformEnabled() const;
    bool isOpenCLSkinningType1Enabled() const;
    bool isOpenCLSkinningType2Enabled() const;
    bool isVertexShaderSkinningType1Enabled() const;
    bool isEffectEnabled() const;

    bool isProjectiveShadowEnabled(const IModel *model) const;
    void setProjectiveShadowEnable(const IModel *model, bool value);
    bool isSelfShadowEnabled(const IModel *model) const;
    void setSelfShadowEnable(const IModel *model, bool value);
    bool isOpenCLSkinningType1Enabled(const IModel *model) const;
    void setOpenCLSkinningEnableType1(const IModel *model, bool value);
    bool isVertexShaderSkinningType1Enabled(const IModel *model) const;
    void setVertexShaderSkinningType1Enable(const IModel *model, bool value);
    IModel *selectedModelRef() const;
    bool isModelSelected(const IModel *value) const;
    void setModelEdgeColor(IModel *model, const QColor &value);
    void setModelEdgeOffset(IModel *model, float value);
    void setModelOpacity(IModel *model, const Scalar &value);
    void setModelPosition(IModel *model, const Vector3 &value);
    const Vector3 modelRotation(IModel *value) const;
    void setModelRotation(IModel *model, const Vector3 &value);

    const Vector3 assetPosition(const IModel *asset);
    void setAssetPosition(const IModel *asset, const Vector3 &value);
    const Quaternion assetRotation(const IModel *asset);
    void setAssetRotation(const IModel *asset, const Quaternion &value);
    float assetOpacity(const IModel *asset);
    void setAssetOpacity(const IModel *asset, float value);
    float assetScaleFactor(const IModel *asset);
    void setAssetScaleFactor(const IModel *asset, float value);
    IModel *selectedAsset() const;
    bool isAssetSelected(const IModel *value) const;
    IModel *assetParentModel(const IModel *asset) const;
    void setAssetParentModel(const IModel *asset, IModel *model);
    IBone *assetParentBone(const IModel *asset) const;
    void setAssetParentBone(const IModel *asset, IBone *bone);

    Scene *sceneRef() const;
    qt::RenderContext *renderContextRef() const;
    qt::World *worldRef() const;
    int maxTimeIndex() const;

public slots:
    void newProject();
    void deleteCameraMotion();
    void deleteMotion(IMotion *&motion);
    void loadProject(const QString &path);
    void saveMetadataFromAsset(const QString &path, IModel *asset);
    void saveProject(const QString &path);
    void setCameraMotion(IMotion *motion);
    void setLightColor(const Vector3 &color);
    void setLightDirection(const Vector3 &value);
    void setModelMotion(IMotion *motion, IModel *model);
    void setRenderOrderList(const QList<QUuid> &value);
    void setWorldGravity(const Vector3 &value);
    void setWorldRandSeed(unsigned long value);
    void sort(bool useOrderAttr = false);
    void startPhysicsSimulation();
    void stopPhysicsSimulation();

    void setGridVisible(bool value);
    void setPhysicsEnabled(bool value);
    void setTimeIndexPlayFrom(int value);
    void setTimeIndexPlayTo(int value);
    void setSceneFPSForPlay(int value);
    void setTimeIndexEncodeVideoFrom(int value);
    void setTimeIndexEncodeVideoTo(int value);
    void setSceneFPSForEncodeVideo(int value);
    void setSceneWidth(int value);
    void setSceneHeight(int value);
    void setLoop(bool value);
    void setGridIncluded(bool value);
    void setSelectedModel(IModel *value);
    void setSelectedAsset(IModel *value);
    void setBackgroundAudioPath(const QString &value);
    void setPreferredFPS(int value);
    void setScreenColor(const QColor &value);
    void setShadowMapSize(const QSize &value);
    void setShadowBoundingSphere(const Vector4 &value);
    void setSoftShadowEnable(bool value);
    void setBackgroundImagePath(const QString &value);
    void setBackgroundImagePosition(const QPoint &value);
    void setBackgroundImageUniformEnable(bool value);
    void setOpenCLSkinningEnableType1(bool value);
    void setOpenCLSkinningEnableType2(bool value);
    void setVertexShaderSkinningType1Enable(bool value);
    void setSoftwareSkinningEnable(bool value);
    void setEffectEnable(bool value);

signals:
    void projectDidInitialized();
    void projectDidOpenProgress(const QString &title, bool cancellable);
    void projectDidUpdateProgress(int value, int max, const QString &text);
    void projectDidLoad(bool loaded);
    void projectDidSave(bool saved);
    void modelDidSelect(IModel *model, SceneLoader *loader);
    void modelDidAdd(IModel *model, const QUuid &uuid);
    void modelWillDelete(IModel *model, const QUuid &uuid);
    void modelDidMakePose(VPDFilePtr pose, IModel *model);
    void motionDidAdd(IMotion *motion, const IModel *model, const QUuid &uuid);
    void motionWillDelete(IMotion *motion, const QUuid &uuid);
    void cameraMotionDidSet(IMotion *motion, const QUuid &uuid);
    void lightColorDidSet(const Vector3 &color);
    void lightDirectionDidSet(const Vector3 &position);
    void preprocessDidPerform();
    void effectDidEnable(bool value);

private slots:
    void setProjectDirtyFalse();
    void deleteModelSlot(IModel *model);

private:
    typedef QPair<QString, QString> FilePathPair;
    bool createModelEngine(IModel *model, const QDir &dir, IRenderEnginePtr &enginePtr);
    bool loadModelFromProject(const QString &path, IModel *model);
    bool handleFuture(const QFuture<IModel *> &future, IModelPtr &modelPtr) const;
    void addAsset(const IModelPtr &assetPtr, const QFileInfo &finfo, IRenderEnginePtr &enginePtr, QUuid &uuid);
    void emitMotionDidAdd(const Array<IMotion *> &motions, IModel *model);
    void insertModel(IModel *model, const QString &name);
    void insertMotion(IMotion *motion, IModel *model);
    void commitAssetProperties();
    bool globalSetting(const char *key, bool def) const;
    int globalSetting(const char *key, int def) const;
    Scene::AccelerationType globalAccelerationType() const;
    Scene::AccelerationType modelAccelerationType(const IModel *model) const;
    IModel *loadBytesAsync(const QByteArray &bytes, IModel::Type type);
    QByteArray loadFile(const FilePathPair &path, const QRegExp &loadable, const QRegExp &extensions, IModel::Type &type);
    IModel *loadFileAsync(const FilePathPair &path, const QRegExp &loadable, const QRegExp &extensions);
    bool loadFileDirectAsync(const FilePathPair &path, const QRegExp &loadable, const QRegExp &extensions, IModel *model);

    QScopedPointer<QGLFramebufferObject> m_depthBuffer;
    QScopedPointer<qt::World> m_world;
    QScopedPointer<Project::IDelegate> m_projectDelegate;
    QScopedPointer<Project> m_project;
    QScopedPointer<IMotion> m_camera;
    qt::RenderContext *m_renderContextRef;
    IEncoding *m_encodingRef;
    Factory *m_factoryRef;
    QMap<QString, IModel*> m_name2assets;
    QMatrix4x4 m_projection;
    IModel *m_selectedModelRef;
    IModel *m_selectedAssetRef;
    Array<QUuid> m_renderOrderList;
    GLuint m_depthBufferID;

    Q_DISABLE_COPY(SceneLoader)
};

} /* namespace vpvm */

#endif // SCENELOADER_H

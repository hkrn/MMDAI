/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include <vpvl2/IEffect.h>
#include <vpvl2/IModel.h>
#include <vpvl2/IMotion.h>
#include <vpvl2/Common.h>
#include <vpvl2/extensions/Archive.h>
#include <vpvl2/extensions/Pose.h>
#include <vpvl2/extensions/Project.h>
#include <vpvl2/qt/RenderContext.h>
#include <glm/glm.hpp>

#include <QObject>
#include <QDir>
#include <QFuture>
#include <QHash>
#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QUuid>
#include <QColor>
#include <QMatrix4x4>

namespace vpvl2 {
class IRenderContext;
namespace extensions {
class Project;
}
}

class btRigidBody;
class QMouseEvent;

namespace vpvm
{

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::qt;

typedef QScopedPointer<Project> ProjectPtr;
typedef QSharedPointer<Pose> PosePtr;

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
    static const QRegExp kDocumentLoadable;

#ifdef VPVL2_ENABLE_EXTENSIONS_ARCHIVE
    static QStringList toStringList(const Archive::EntryNames &value);
    static void getEntrySet(const QStringList &value, Archive::EntrySet &setRef);
#endif

    SceneLoader(IEncoding *encodingRef, Factory *factoryRef, qt::RenderContext *renderContextRef);
    ~SceneLoader();

    QList<IModelSharedPtr> allModels() const;
    void addModel(IModelSharedPtr model, const QFileInfo &finfo, const QFileInfo &entry, QUuid &uuid);
    IRenderEnginePtr createModelEngine(IModelSharedPtr model, const QDir &dir);
    IModelSharedPtr findAsset(const QUuid &uuid) const;
    IModelSharedPtr findModel(const QUuid &uuid) const;
    IMotionSharedPtr findMotion(const QUuid &uuid) const;
    const QUuid findUUID(const IModel *model) const;
    void getBoundingSphere(Vector3 &center, Scalar &radius) const;
    void getCameraMatrices(glm::mat4 &worldRef, glm::mat4 &view, glm::mat4 &projection) const;
    bool isProjectModified() const;
    bool loadAsset(const QString &filename, QUuid &uuid, IModelSharedPtr &assetPtr);
    bool loadAsset(const QByteArray &bytes, const QFileInfo &finfo, const QFileInfo &entry, QUuid &uuid, IModelSharedPtr &assetPtr);
    bool loadAssetFromMetadata(const QString &baseName, const QDir &dir, QUuid &uuid, IModelSharedPtr &assetPtr);
    bool loadCameraMotion(const QString &path, IMotionSharedPtr &motionPtr);
    bool loadEffectRef(const QString &filename, IEffect *&effectRef);
    bool loadModel(const QString &filename, IModelSharedPtr &modelPtr);
    bool loadModel(const QByteArray &bytes, IModel::Type type, IModelSharedPtr &modelPtr);
    bool loadModelMotion(const QString &path, IMotionSharedPtr &motionPtr);
    bool loadModelMotion(const QString &path, QList<IModelSharedPtr> &models, IMotionSharedPtr &motionPtr);
    bool loadModelMotion(const QString &path, IModelSharedPtr model, IMotionSharedPtr &motionPtr);
    PosePtr loadModelPose(const QString &path, IModelSharedPtr model);
    void createCameraMotion();
    void newCameraMotion(IMotionSharedPtr &motionPtr) const;
    void newModelMotion(IModelSharedPtr model, IMotionSharedPtr &motionPtr) const;
    void newProject(ProjectPtr &projectPtr);
    void releaseProject();
    void renderWindow();
    void renderOffscreen();
    void setLightViewProjectionMatrix();
    void setMousePosition(const QMouseEvent *event, const QRect &geometry);
    void updateDepthBuffer(const QSize &value);
    const QList<QUuid> renderOrderList() const;
    const QMatrix4x4 &projectionMatrix() const { return m_projection; }

    bool isGridVisible() const;
    bool isPhysicsEnabled() const;
    const Vector3 worldGravity() const;
    int worldMaxSubSteps() const;
    bool worldFloorEnabled() const;
    unsigned long worldRandSeed() const;
    const QColor screenColor() const;
    int timeIndexPlayFrom() const;
    int timeIndexPlayTo() const;
    int timeIndexEncodeVideoFrom() const;
    int timeIndexEncodeVideoTo() const;
    Scalar sceneFPSForPlay() const;
    Scalar sceneFPSForEncodeVideo() const;
    int sceneWidth() const;
    int sceneHeight() const;
    bool isLoop() const;
    bool isGridIncluded() const;
    const QString backgroundAudio() const;
    const QSize shadowMapSize() const;
    const Vector3 shadowCenter() const;
    const QString backgroundImage() const;
    const QPoint backgroundImagePosition() const;
    bool isBackgroundImageUniformEnabled() const;
    bool isParallelSkinningEnabled() const;
    bool isOpenCLSkinningType1Enabled() const;
    bool isOpenCLSkinningType2Enabled() const;
    bool isVertexShaderSkinningType1Enabled() const;
    bool isEffectEnabled() const;
    const Scalar shadowDistance() const;

    bool isProjectiveShadowEnabled(const IModel *model) const;
    void setProjectiveShadowEnable(const IModel *model, bool value);
    bool isSelfShadowEnabled(const IModel *model) const;
    void setSelfShadowEnable(const IModel *model, bool value);
    IModelSharedPtr selectedModelRef() const;
    bool isModelSelected(const IModel *value) const;
    void setModelEdgeColor(IModelSharedPtr model, const QColor &value);
    void setModelEdgeOffset(IModelSharedPtr model, float value);
    void setModelOpacity(IModelSharedPtr model, const Scalar &value);
    void setModelPosition(IModelSharedPtr model, const Vector3 &value);
    const Vector3 modelRotation(IModelSharedPtr value) const;
    void setModelRotation(IModelSharedPtr model, const Vector3 &value);

    const Vector3 assetPosition(const IModel *asset);
    const Quaternion assetRotation(const IModel *asset);
    float assetOpacity(const IModel *asset);
    float assetScaleFactor(const IModel *asset);
    IModelSharedPtr selectedAsset() const;
    bool isAssetSelected(const IModel *value) const;
    IModelSharedPtr assetParentModel(const IModel *asset) const;
    IBone *assetParentBone(const IModel *asset) const;

    Scene *sceneRef() const;
    qt::RenderContext *renderContextRef() const;
    qt::World *worldRef() const;

public slots:
    void newProject();
    void deleteCameraMotion();
    void deleteModel(IModelSharedPtr model);
    void deleteMotion(IMotionSharedPtr motion);
    void loadProject(const QString &path);
    void saveMetadataFromAsset(const QString &path, IModelSharedPtr asset);
    void saveProject(const QString &path);
    void setCameraMotion(IMotionSharedPtr motion, const QUuid &uuid, bool addToScene);
    void setLightColor(const Vector3 &color);
    void setLightDirection(const Vector3 &value);
    void setModelMotion(IMotionSharedPtr motion, IModelSharedPtr model);
    void setRenderOrderList(const QList<QUuid> &value);
    void setWorldGravity(const Vector3 &value);
    void setWorldMaxSubSteps(int value);
    void setWorldFloorEnable(bool value);
    void setWorldRandSeed(unsigned long value);
    void sort();
    void startPhysicsSimulation();
    void stopPhysicsSimulation();

    void setGridVisible(bool value);
    void setPhysicsEnabled(bool value);
    void setTimeIndexPlayFrom(int value);
    void setTimeIndexPlayTo(int value);
    void setSceneFPSForPlay(const Scalar &value);
    void setSceneFPSForEncodeVideo(const Scalar &value);
    void setTimeIndexEncodeVideoFrom(int value);
    void setTimeIndexEncodeVideoTo(int value);
    void setSceneWidth(int value);
    void setSceneHeight(int value);
    void setLoop(bool value);
    void setGridIncluded(bool value);
    void setSelectedModel(IModelSharedPtr value);
    void setSelectedAsset(IModelSharedPtr value);
    void setBackgroundAudioPath(const QString &value);
    void setPreferredFPS(const Scalar &value);
    void setScreenColor(const QColor &value);
    void setShadowMapSize(const QSize &value);
    void setShadowCenter(const Vector3 &value);
    void setBackgroundImagePath(const QString &value);
    void setBackgroundImagePosition(const QPoint &value);
    void setBackgroundImageUniformEnable(bool value);
    void setOpenCLSkinningEnableType1(bool value);
    void setOpenCLSkinningEnableType2(bool value);
    void setVertexShaderSkinningType1Enable(bool value);
    void setParallelSkinningEnable(bool value);
    void setSoftwareSkinningEnable(bool value);
    void setEffectEnable(bool value);
    void setShadowDistance(const Scalar &value);

signals:
    void projectDidInitialized();
    void projectDidLoad(bool loaded);
    void projectDidSave(bool saved);
    void modelDidSelect(IModelSharedPtr model);
    void modelDidAdd(IModelSharedPtr model, const QUuid &uuid);
    void modelWillDelete(IModelSharedPtr model, const QUuid &uuid);
    void modelDidMakePose(PosePtr pose, IModelSharedPtr model);
    void motionDidAdd(IMotionSharedPtr motion, IModelSharedPtr model, const QUuid &uuid);
    void motionWillDelete(IMotionSharedPtr motion, const QUuid &uuid);
    void cameraMotionDidSet(IMotionSharedPtr motion, const QUuid &uuid);
    void lightColorDidSet(const Vector3 &color);
    void lightDirectionDidSet(const Vector3 &position);
    void preprocessDidPerform();
    void effectDidEnable(bool value);

    void backgroundAudioPathDidChange(const QString &value);
    void sceneWidthDidChange(int value);
    void sceneHeightDidChange(int value);
    void timeIndexEncodeVideoFromDidChange(int value);
    void timeIndexEncodeVideoToDidChange(int value);
    void sceneFPSForEncodeVideoDidChange(int value);
    void gridIncludedDidChange(bool value);

private slots:
    void updatePhysicsSimulation(const Scalar &timeStep);
    void markProjectDirtyToClean();

private:
    typedef QPair<QString, QString> FilePathPair;
    IEffect *createEffectRef(IModelSharedPtr model, const QString &dirOrPath, int &flags);
    void handleFuture(QFuture<IModelSharedPtr> future, IModelSharedPtr &modelPtr) const;
    void addAsset(IModelSharedPtr assetPtr, const QFileInfo &finfo, IRenderEnginePtr &enginePtr, QUuid &uuid);
    void emitMotionDidAdd(const Array<IMotion *> &motions, IModelSharedPtr model);
    void insertModel(IModelSharedPtr model, const QString &name);
    void insertMotion(IMotionSharedPtr motion, IModelSharedPtr model);
    bool globalSetting(const char *key, bool def) const;
    int globalSetting(const char *key, int def) const;
    float globalSetting(const char *key, float def) const;
    Scene::AccelerationType globalAccelerationType() const;
    Scene::AccelerationType modelAccelerationType(const IModel *model) const;
    QByteArray loadFile(const FilePathPair &path, const QRegExp &loadable, const QRegExp &extensions, IModel::Type &type);
    IModelSharedPtr loadModelFromBytesAsync(const QByteArray &bytes, IModel::Type type);
    IModelSharedPtr loadModelFromFileAsync(const FilePathPair &path, const QRegExp &loadable, const QRegExp &extensions);
    bool loadModelFromFileDirectAsync(const FilePathPair &path, const QRegExp &loadable, const QRegExp &extensions, IModelSharedPtr model);
    void restoreSceneStatesFromProject(Project *project);
    void applyParallelSkinning(bool value);

    QScopedPointer<qt::World> m_world;
    QScopedPointer<Project::IDelegate> m_projectDelegate;
    QScopedPointer<Project> m_project;
    QScopedPointer<btRigidBody> m_floor;
    IMotionSharedPtr m_camera;
    RenderContext *m_renderContextRef;
    IEncoding *m_encodingRef;
    Factory *m_factoryRef;
    QMap<QString, IModel*> m_name2assets;
    QMatrix4x4 m_projection;
    IModelSharedPtr m_selectedModelRef;
    IModelSharedPtr m_selectedAssetRef;

    Q_DISABLE_COPY(SceneLoader)
};

} /* namespace vpvm */

#endif // SCENELOADER_H

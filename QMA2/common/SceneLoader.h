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

#ifndef SCENELOADER_H
#define SCENELOADER_H

#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QUuid>
#include <QtGui/QColor>
#include <QtGui/QMatrix4x4>

#include "VPDFile.h"

#include <vpvl2/Common.h>
#include <vpvl2/Project.h>

namespace internal {
class Encoding;
class World;
}

namespace vpvl2 {
class IModel;
class IMotion;
class IRenderDelegate;
class Project;
}

class QGLFramebufferObject;

class SceneLoader : public QObject
{
    Q_OBJECT

public:
    explicit SceneLoader(vpvl2::IEncoding *encoding, vpvl2::Factory *factory);
    ~SceneLoader();

    QList<vpvl2::IModel *> allModels() const;
    vpvl2::IModel *findAsset(const QUuid &uuid) const;
    vpvl2::IModel *findModel(const QUuid &uuid) const;
    vpvl2::IMotion *findMotion(const QUuid &uuid) const;
    const QUuid findUUID(vpvl2::IModel *model) const;
    void getBoundingSphere(vpvl2::Vector3 &center, vpvl2::Scalar &radius) const;
    bool isProjectModified() const;
    bool loadAsset(const QString &filename, QUuid &uuid, vpvl2::IModel *&asset);
    vpvl2::IModel *loadAssetFromMetadata(const QString &baseName, const QDir &dir, QUuid &uuid);
    vpvl2::IMotion *loadCameraMotion(const QString &path);
    bool loadModel(const QString &filename, vpvl2::IModel *&model);
    vpvl2::IMotion *loadModelMotion(const QString &path);
    vpvl2::IMotion *loadModelMotion(const QString &path, QList<vpvl2::IModel *> &models);
    vpvl2::IMotion *loadModelMotion(const QString &path, vpvl2::IModel *model);
    VPDFilePtr loadModelPose(const QString &path, vpvl2::IModel *model);
    vpvl2::IMotion *newCameraMotion() const;
    vpvl2::IMotion *newModelMotion(vpvl2::IModel *model) const;
    void release();
    void renderModels();
    void renderZPlot(QGLFramebufferObject *renderTarget);
    void updateMatrices(const QSizeF &size);
    const QList<QUuid> renderOrderList() const;
    const QMatrix4x4 &projectionMatrix() const { return m_projection; }

    bool isGridVisible() const;
    bool isPhysicsEnabled() const;
    bool isAccelerationEnabled() const;
    bool isBlackBackgroundEnabled() const;
    const vpvl2::Vector3 worldGravity() const;
    const QColor screenColor() const;
    int frameIndexPlayFrom() const;
    int frameIndexPlayTo() const;
    int sceneFPSForPlay() const;
    int frameIndexEncodeVideoFrom() const;
    int frameIndexEncodeVideoTo() const;
    int sceneFPSForEncodeVideo() const;
    int sceneWidth() const;
    int sceneHeight() const;
    bool isLoop() const;
    bool isGridIncluded() const;
    const QString backgroundAudio() const;

    bool isProjectiveShadowEnabled(const vpvl2::IModel *model) const;
    void setProjectiveShadowEnable(const vpvl2::IModel *model, bool value);
    vpvl2::IModel *selectedModel() const;
    bool isModelSelected(const vpvl2::IModel *value) const;
    void setModelEdgeColor(vpvl2::IModel *model, const QColor &value);
    void setModelEdgeOffset(vpvl2::IModel *model, float value);
    void setModelPosition(vpvl2::IModel *model, const vpvl2::Vector3 &value);
    const vpvl2::Vector3 modelRotation(vpvl2::IModel *value) const;
    void setModelRotation(vpvl2::IModel *model, const vpvl2::Vector3 &value);

    const vpvl2::Vector3 assetPosition(const vpvl2::IModel *asset);
    void setAssetPosition(const vpvl2::IModel *asset, const vpvl2::Vector3 &value);
    const vpvl2::Quaternion assetRotation(const vpvl2::IModel *asset);
    void setAssetRotation(const vpvl2::IModel *asset, const vpvl2::Quaternion &value);
    float assetOpacity(const vpvl2::IModel *asset);
    void setAssetOpacity(const vpvl2::IModel *asset, float value);
    float assetScaleFactor(const vpvl2::IModel *asset);
    void setAssetScaleFactor(const vpvl2::IModel *asset, float value);
    vpvl2::IModel *selectedAsset() const;
    bool isAssetSelected(const vpvl2::IModel *value) const;
    vpvl2::IModel *assetParentModel(vpvl2::IModel *asset) const;
    void setAssetParentModel(const vpvl2::IModel *asset, vpvl2::IModel *model);
    vpvl2::IBone *assetParentBone(vpvl2::IModel *asset) const;
    void setAssetParentBone(const vpvl2::IModel *asset, vpvl2::IBone *bone);

    vpvl2::Scene *scene() const;
    internal::World *world() const;
    int maxFrameIndex() const;

public slots:
    void addModel(vpvl2::IModel *model, const QString &baseName, const QDir &dir, QUuid &uuid);
    void createProject();
    void deleteAsset(vpvl2::IModel *asset);
    void deleteCameraMotion();
    void deleteModel(vpvl2::IModel *&model);
    void deleteMotion(vpvl2::IMotion *&motion);
    void loadProject(const QString &path);
    void saveMetadataFromAsset(const QString &path, vpvl2::IModel *asset);
    void saveProject(const QString &path);
    void setCameraMotion(vpvl2::IMotion *motion);
    void setLightColor(const vpvl2::Vector3 &color);
    void setLightPosition(const vpvl2::Vector3 &position);
    void setModelMotion(vpvl2::IMotion *motion, vpvl2::IModel *model);
    void setRenderOrderList(const QList<QUuid> &value);
    void setWorldGravity(const vpvl2::Vector3 &value);
    void sort(bool useOrderAttr = false);
    void startPhysicsSimulation();
    void stopPhysicsSimulation();

    void setGridVisible(bool value);
    void setPhysicsEnabled(bool value);
    void setAccelerationEnabled(bool value);
    void setFrameIndexPlayFrom(int value);
    void setFrameIndexPlayTo(int value);
    void setSceneFPSForPlay(int value);
    void setFrameIndexEncodeVideoFrom(int value);
    void setFrameIndexEncodeVideoTo(int value);
    void setSceneFPSForEncodeVideo(int value);
    void setSceneWidth(int value);
    void setSceneHeight(int value);
    void setLoop(bool value);
    void setGridIncluded(bool value);
    void setSelectedModel(vpvl2::IModel *value);
    void setSelectedAsset(vpvl2::IModel *value);
    void setBackgroundAudioPath(const QString &value);
    void setPreferredFPS(int value);
    void setScreenColor(const QColor &value);

signals:
    void projectDidCount(int value);
    void projectDidProceed(int value);
    void projectDidLoad(bool loaded);
    void projectDidSave();
    void modelDidSelect(vpvl2::IModel *model, SceneLoader *loader);
    void modelDidAdd(vpvl2::IModel *model, const QUuid &uuid);
    void modelWillDelete(vpvl2::IModel *model, const QUuid &uuid);
    void modelDidMakePose(VPDFilePtr pose, vpvl2::IModel *model);
    void motionDidAdd(vpvl2::IMotion *motion, vpvl2::IModel *model, const QUuid &uuid);
    void motionWillDelete(vpvl2::IMotion *motion, const QUuid &uuid);
    void assetDidSelect(vpvl2::IModel *asset, SceneLoader *loader);
    void assetDidAdd(vpvl2::IModel *asset, const QUuid &uuid);
    void assetWillDelete(vpvl2::IModel *asset, const QUuid &uuid);
    void cameraMotionDidSet(vpvl2::IMotion *motion, const QUuid &uuid);
    void lightColorDidSet(const vpvl2::Vector3 &color);
    void lightDirectionDidSet(const vpvl2::Vector3 &position);

private:
    void insertModel(vpvl2::IModel *model, const QString &name);
    void insertMotion(vpvl2::IMotion *motion, vpvl2::IModel *model);
    void commitAssetProperties();
    bool globalSetting(const char *key, bool def) const;
    int globalSetting(const char *key, int def) const;

    internal::World *m_world;
    QMap<QString, vpvl2::IModel*> m_name2assets;
    QMatrix4x4 m_projection;
    vpvl2::IEncoding *m_encoding;
    vpvl2::IRenderDelegate *m_renderDelegate;
    vpvl2::Factory *m_factory;
    vpvl2::Project *m_project;
    vpvl2::Project::IDelegate *m_projectDelegate;
    vpvl2::IModel *m_model;
    vpvl2::IModel *m_asset;
    vpvl2::IMotion *m_camera;
    vpvl2::Array<QUuid> m_renderOrderList;

    Q_DISABLE_COPY(SceneLoader)
};

#endif // SCENELOADER_H

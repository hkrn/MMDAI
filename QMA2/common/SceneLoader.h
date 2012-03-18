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

#include <vpvl/Common.h>
#include <vpvl/Project.h>
#include <vpvl/gl2/Renderer.h>

namespace internal {
class World;
}

namespace vpvl
{
namespace gl2
{
class Renderer;
}
class Asset;
class Bone;
class PMDModel;
class VMDMotion;
}

class VPDFile;

class SceneLoader : public QObject
{
    Q_OBJECT

public:
    static bool isAccelerationSupported();

    explicit SceneLoader(int width, int height, int fps);
    ~SceneLoader();

    QList<vpvl::PMDModel *> allModels() const;
    vpvl::Asset *findAsset(const QUuid &uuid) const;
    vpvl::PMDModel *findModel(const QUuid &uuid) const;
    vpvl::VMDMotion *findMotion(const QUuid &uuid) const;
    const QUuid findUUID(vpvl::Asset *asset) const;
    const QUuid findUUID(vpvl::PMDModel *model) const;
    bool isProjectModified() const;
    bool loadAsset(const QString &filename, QUuid &uuid, vpvl::Asset *&asset);
    vpvl::Asset *loadAssetFromMetadata(const QString &baseName, const QDir &dir, QUuid &uuid);
    vpvl::VMDMotion *loadCameraMotion(const QString &path);
    bool loadModel(const QString &filename, vpvl::PMDModel *&model);
    vpvl::VMDMotion *loadModelMotion(const QString &path);
    vpvl::VMDMotion *loadModelMotion(const QString &path, QList<vpvl::PMDModel *> &models);
    vpvl::VMDMotion *loadModelMotion(const QString &path, vpvl::PMDModel *model);
    VPDFile *loadModelPose(const QString &path, vpvl::PMDModel *model);
    vpvl::VMDMotion *newCameraMotion() const;
    vpvl::VMDMotion *newModelMotion(vpvl::PMDModel *model) const;
    void release();
    void render();
    const QList<QUuid> renderOrderList() const;

    bool isGridVisible() const;
    bool isPhysicsEnabled() const;
    bool isAccelerationEnabled() const;
    bool isBlackBackgroundEnabled() const;
    const vpvl::Vector3 worldGravity() const;
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

    bool isProjectiveShadowEnabled(const vpvl::PMDModel *model) const;
    void setProjectiveShadowEnable(const vpvl::PMDModel *model, bool value);
    vpvl::PMDModel *selectedModel() const;
    bool isModelSelected(const vpvl::PMDModel *value) const;
    void setModelEdgeColor(vpvl::PMDModel *model, const QColor &value);
    void setModelEdgeOffset(vpvl::PMDModel *model, float value);

    const vpvl::Vector3 assetPosition(const vpvl::Asset *asset);
    void setAssetPosition(const vpvl::Asset *asset, const vpvl::Vector3 &value);
    const vpvl::Quaternion assetRotation(const vpvl::Asset *asset);
    void setAssetRotation(const vpvl::Asset *asset, const vpvl::Quaternion &value);
    float assetOpacity(const vpvl::Asset *asset);
    void setAssetOpacity(const vpvl::Asset *asset, float value);
    float assetScaleFactor(const vpvl::Asset *asset);
    void setAssetScaleFactor(const vpvl::Asset *asset, float value);
    vpvl::Asset *selectedAsset() const;
    bool isAssetSelected(const vpvl::Asset *value) const;
    vpvl::PMDModel *assetParentModel(vpvl::Asset *asset) const;
    void setAssetParentModel(const vpvl::Asset *asset, vpvl::PMDModel *model);
    vpvl::Bone *assetParentBone(vpvl::Asset *asset) const;
    void setAssetParentBone(const vpvl::Asset *asset, vpvl::Bone *bone);

    vpvl::gl2::Renderer *renderEngine() const {
        return m_renderer;
    }

public slots:
    void addModel(vpvl::PMDModel *model, const QString &baseName, const QDir &dir, QUuid &uuid);
    void createProject();
    void deleteAsset(vpvl::Asset *asset);
    void deleteCameraMotion();
    void deleteModel(vpvl::PMDModel *&model);
    void deleteMotion(vpvl::VMDMotion *&motion);
    void loadProject(const QString &path);
    void saveMetadataFromAsset(const QString &path, vpvl::Asset *asset);
    void saveProject(const QString &path);
    void setCameraMotion(vpvl::VMDMotion *motion);
    void setLightColor(const vpvl::Color &color);
    void setLightPosition(const vpvl::Vector3 &position);
    void setModelMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void setRenderOrderList(const QList<QUuid> &value);
    void setWorldGravity(const vpvl::Vector3 &value);
    void sort(bool useOrderAttr = false);
    void startPhysicsSimulation();
    void stopPhysicsSimulation();

    void setGridVisible(bool value);
    void setPhysicsEnabled(bool value);
    void setAccelerationEnabled(bool value);
    void setBlackBackgroundEnabled(bool value);
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
    void setSelectedModel(vpvl::PMDModel *value);
    void setSelectedAsset(vpvl::Asset *value);
    void setBackgroundAudio(const QString &path);
    void setPreferredFPS(int value);

signals:
    void projectDidCount(int value);
    void projectDidProceed(int value);
    void projectDidLoad(bool loaded);
    void projectDidSave();
    void modelDidSelect(vpvl::PMDModel *model, SceneLoader *loader);
    void modelDidAdd(vpvl::PMDModel *model, const QUuid &uuid);
    void modelWillDelete(vpvl::PMDModel *model, const QUuid &uuid);
    void modelDidMakePose(VPDFile *pose, vpvl::PMDModel *model);
    void motionDidAdd(vpvl::VMDMotion *motion, vpvl::PMDModel *model, const QUuid &uuid);
    void motionWillDelete(vpvl::VMDMotion *motion, const QUuid &uuid);
    void assetDidSelect(vpvl::Asset *asset, SceneLoader *loader);
    void assetDidAdd(vpvl::Asset *asset, const QUuid &uuid);
    void assetWillDelete(vpvl::Asset *asset, const QUuid &uuid);
    void cameraMotionDidSet(vpvl::VMDMotion *motion, const QUuid &uuid);
    void lightColorDidSet(const vpvl::Color &color);
    void lightPositionDidSet(const vpvl::Vector3 &position);

private:
    void insertModel(vpvl::PMDModel *model, const QString &name);
    void insertMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void commitAssetProperties();
    bool globalSetting(const char *key, bool def) const;
    int globalSetting(const char *key, int def) const;

    internal::World *m_world;
    vpvl::gl2::Renderer *m_renderer;
    vpvl::gl2::Renderer::IDelegate *m_renderDelegate;
    QMap<QString, vpvl::Asset*> m_name2assets;
    vpvl::Project *m_project;
    vpvl::Project::IDelegate *m_projectDelegate;
    vpvl::PMDModel *m_model;
    vpvl::Asset *m_asset;
    vpvl::VMDMotion *m_camera;
    vpvl::Array<QUuid> m_renderOrderList;

    Q_DISABLE_COPY(SceneLoader)
};

#endif // SCENELOADER_H

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

#ifndef SCENELOADER_H
#define SCENELOADER_H

#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QUuid>

#include <vpvl/Common.h>
#include <vpvl/Project.h>

namespace vpvl
{
#ifdef VPVL_USE_GLSL
namespace gl2
#else
namespace gl
#endif
{
class Renderer;
}
class Asset;
class PMDModel;
class VMDMotion;
}

class VPDFile;

class SceneLoader : public QObject
{
    Q_OBJECT

public:
#ifdef VPVL_USE_GLSL
    explicit SceneLoader(vpvl::gl2::Renderer *renderer);
#else
    explicit SceneLoader(vpvl::gl::Renderer *renderer);
#endif
    ~SceneLoader();

    vpvl::Asset *findAsset(const QUuid &uuid) const;
    vpvl::PMDModel *findModel(const QUuid &uuid) const;
    const QUuid findUUID(vpvl::Asset *asset) const;
    const QUuid findUUID(vpvl::PMDModel *model) const;
    QList<vpvl::VMDMotion *> findModelMotions(vpvl::PMDModel *model) const;
    bool isProjectModified() const;
    vpvl::Asset *loadAsset(const QString &baseName, const QDir &dir, QUuid &uuid);
    vpvl::Asset *loadAssetFromMetadata(const QString &baseName, const QDir &dir, QUuid &uuid);
    vpvl::VMDMotion *loadCameraMotion(const QString &path);
    vpvl::PMDModel *loadModel(const QString &baseName, const QDir &dir);
    vpvl::VMDMotion *loadModelMotion(const QString &path);
    vpvl::VMDMotion *loadModelMotion(const QString &path, QList<vpvl::PMDModel *> &models);
    vpvl::VMDMotion *loadModelMotion(const QString &path, vpvl::PMDModel *model);
    VPDFile *loadModelPose(const QString &path, vpvl::PMDModel *model);
    vpvl::VMDMotion *newCameraMotion() const;
    vpvl::VMDMotion *newModelMotion(vpvl::PMDModel *model) const;
    void release();
    const QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> stoppedMotions();

public slots:
    void addModel(vpvl::PMDModel *model, const QString &baseName, const QDir &dir, QUuid &uuid);
    void createProject();
    void deleteAsset(vpvl::Asset *asset);
    void deleteCameraMotion();
    void deleteModel(vpvl::PMDModel *model);
    void loadProject(const QString &path);
    void saveMetadataFromAsset(const QString &path, vpvl::Asset *asset);
    void saveProject(const QString &path);
    void setCameraMotion(vpvl::VMDMotion *motion);
    void setModelMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);

signals:
    void projectDidLoad();
    void projectDidSave();
    void modelDidAdd(vpvl::PMDModel *model, const QUuid &uuid);
    void modelWillDelete(vpvl::PMDModel *model, const QUuid &uuid);
    void modelDidMakePose(VPDFile *pose, vpvl::PMDModel *model);
    void motionDidAdd(vpvl::VMDMotion *motion, vpvl::PMDModel *model, const QUuid &uuid);
    void assetDidAdd(vpvl::Asset *asset, const QUuid &uuid);
    void assetWillDelete(vpvl::Asset *asset, const QUuid &uuid);
    void cameraMotionDidSet(vpvl::VMDMotion *motion, const QUuid &uuid);

private:
    void insertModel(vpvl::PMDModel *model, const QString &name);
    void insertMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);

#ifdef VPVL_USE_GLSL
    vpvl::gl2::Renderer *m_renderer;
#else
    vpvl::gl::Renderer *m_renderer;
#endif
    QMap<QString, vpvl::Asset*> m_name2assets;
    vpvl::Project *m_project;
    vpvl::Project::IDelegate *m_delegate;
    vpvl::VMDMotion *m_camera;

    Q_DISABLE_COPY(SceneLoader)
};

#endif // SCENELOADER_H

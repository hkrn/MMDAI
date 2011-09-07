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
#include <QtCore/QString>

namespace vpvl
{
namespace gl
{
class Renderer;
}
class Asset;
class PMDModel;
class VMDMotion;
}

class VPDFile;

class SceneLoader
{
public:
    SceneLoader(vpvl::gl::Renderer *renderer);
    ~SceneLoader();

    bool deleteModel(vpvl::PMDModel *model);
    vpvl::PMDModel *findModel(const QString &name) const;
    vpvl::VMDMotion *findModelMotion(vpvl::PMDModel *model) const;
    vpvl::Asset *loadAsset(const QString &baseName, const QDir &dir);
    vpvl::VMDMotion *loadCameraMotion(const QString &path);
    vpvl::PMDModel *loadModel(const QString &baseName, const QDir &dir, vpvl::VMDMotion *&nullMotion);
    vpvl::VMDMotion *loadModelMotion(const QString &path);
    vpvl::VMDMotion *loadModelMotion(const QString &path, QList<vpvl::PMDModel *> &models);
    vpvl::VMDMotion *loadModelMotion(const QString &path, vpvl::PMDModel *model);
    VPDFile *loadPose(const QString &path, vpvl::PMDModel *model);
    void setModelMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model, const QString &location);

private:
    void insertModel(vpvl::PMDModel *model, const QString &name, const QString &location);
    void insertMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model, const QString &location);

    vpvl::gl::Renderer *m_renderer;
    vpvl::VMDMotion *m_camera;
    QHash<QString, vpvl::PMDModel *> m_models;
    QHash<QString, vpvl::Asset *> m_assets;
    QHash<vpvl::PMDModel *, vpvl::VMDMotion *> m_motions;
    QHash<vpvl::PMDModel *, QString> m_modelLocation;
    QHash<vpvl::VMDMotion *, QString> m_motionLocation;

    Q_DISABLE_COPY(SceneLoader)
};

#endif // SCENELOADER_H

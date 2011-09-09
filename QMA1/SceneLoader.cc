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

#include "SceneLoader.h"
#include "util.h"

#include <QtCore/QtCore>
#include <vpvl/vpvl.h>
#include <vpvl/gl/Renderer.h>

SceneLoader::SceneLoader(vpvl::gl::Renderer *renderer)
    : m_renderer(renderer),
      m_camera(0)
{
}

SceneLoader::~SceneLoader()
{
    delete m_camera;
    m_camera = 0;
    QHashIterator<vpvl::PMDModel *, vpvl::VMDMotion *> i(m_motions);
    while (i.hasNext()) {
        i.next();
        vpvl::PMDModel *model = i.key();
        vpvl::VMDMotion *motion = i.value();
        model->removeMotion(motion);
        delete motion;
    }
    foreach (vpvl::PMDModel *model, m_models) {
        m_renderer->unloadModel(model);
        delete model;
    }
    m_models.clear();
    m_motions.clear();
    m_assets.clear();
}

bool SceneLoader::deleteModel(vpvl::PMDModel *model)
{
    if (!model)
        return false;
    const QString &key = m_models.key(model);
    if (!key.isNull()) {
        deleteModelMotion(model);
        m_renderer->scene()->removeModel(model);
        m_renderer->unloadModel(model);
        m_renderer->setSelectedModel(0);
        m_models.remove(key);
        delete model;
        return true;
    }
    return false;
}

bool SceneLoader::deleteModelMotion(vpvl::PMDModel *model)
{
    if (!model)
        return false;
    if (m_motions.contains(model)) {
        QHashIterator<vpvl::PMDModel *, vpvl::VMDMotion *> i(m_motions);
        while (i.hasNext()) {
            i.next();
            if (i.key() == model) {
                vpvl::VMDMotion *motion = i.value();
                model->removeMotion(motion);
                delete motion;
            }
        }
        m_motions.remove(model);
        return true;
    }
    return false;
}

bool SceneLoader::deleteModelMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (m_motions.contains(model)) {
        QMutableHashIterator<vpvl::PMDModel *, vpvl::VMDMotion *> i(m_motions);
        while (i.hasNext()) {
            i.next();
            if (i.key() == model && i.value() == motion) {
                model->removeMotion(motion);
                m_motions.remove(model, motion);
                delete motion;
                break;
            }
        }
        return true;
    }
    return false;
}

vpvl::PMDModel *SceneLoader::findModel(const QString &name) const
{
    return m_models.value(name);
}

QList<vpvl::VMDMotion *> SceneLoader::findModelMotions(vpvl::PMDModel *model) const
{
    return m_motions.values(model);
}

vpvl::Asset *SceneLoader::loadAsset(const QString &baseName, const QDir &dir)
{
    QFile file(dir.absoluteFilePath(baseName));
    vpvl::Asset *asset = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        asset = new vpvl::Asset();
        if (asset->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            QString key = baseName;
            if (m_assets.contains(key)) {
                int i = 0;
                while (true) {
                    QString tmpKey = QString("%1%2").arg(key).arg(i);
                    if (!m_assets.contains(tmpKey))
                        key = tmpKey;
                    i++;
                }
            }
            const QByteArray &assetName = baseName.toUtf8();
            char *name = new char[assetName.size() + 1];
            memcpy(name, assetName.constData(), assetName.size());
            name[assetName.size()] = 0;
            asset->setName(name);
            m_renderer->loadAsset(asset, std::string(dir.absolutePath().toLocal8Bit()));
            m_assets[key] = asset;
        }
        else {
            delete asset;
            asset = 0;
        }
    }
    return asset;
}

vpvl::VMDMotion *SceneLoader::loadCameraMotion(const QString &path)
{
    QFile file(path);
    vpvl::VMDMotion *motion = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        motion = new vpvl::VMDMotion();
        if (motion->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            delete m_camera;
            m_camera = motion;
            m_renderer->scene()->setCameraMotion(motion);
        }
        else {
            delete motion;
            motion = 0;
        }
    }
    return motion;
}

vpvl::PMDModel *SceneLoader::loadModel(const QString &baseName, const QDir &dir)
{
    const QString &path = dir.absoluteFilePath(baseName);
    QFile file(path);
    vpvl::PMDModel *model = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        model = new vpvl::PMDModel();
        if (model->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            m_renderer->loadModel(model, std::string(dir.absolutePath().toLocal8Bit()));
            m_renderer->scene()->addModel(model);
            QString key = internal::toQString(model);
            qDebug() << key << baseName;
            if (m_models.contains(key)) {
                int i = 0;
                while (true) {
                    QString tmpKey = QString("%1%2").arg(key).arg(i);
                    if (!m_models.contains(tmpKey))
                        key = tmpKey;
                    i++;
                }
            }
            m_models.insert(key, model);
        }
        else {
            delete model;
            model = 0;
        }
    }
    return model;
}

vpvl::VMDMotion *SceneLoader::loadModelMotion(const QString &path)
{
    QFile file(path);
    vpvl::VMDMotion *motion = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        motion = new vpvl::VMDMotion();
        if (!motion->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            delete motion;
            motion = 0;
        }
    }
    return motion;
}

vpvl::VMDMotion *SceneLoader::loadModelMotion(const QString &path, QList<vpvl::PMDModel *> &models)
{
    vpvl::VMDMotion *motion = loadModelMotion(path);
    if (motion) {
        foreach (vpvl::PMDModel *model, m_models) {
            setModelMotion(motion, model);
            models.append(model);
        }
    }
    return motion;
}

vpvl::VMDMotion *SceneLoader::loadModelMotion(const QString &path, vpvl::PMDModel *model)
{
    vpvl::VMDMotion *motion = loadModelMotion(path);
    if (motion)
        setModelMotion(motion, model);
    return motion;
}

void SceneLoader::setModelMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    model->addMotion(motion);
    m_motions.insert(model, motion);
}

const QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> SceneLoader::stoppedMotions()
{
    QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> ret;
    QHashIterator<vpvl::PMDModel *, vpvl::VMDMotion *> i(m_motions);
    while (i.hasNext()) {
        i.next();
        vpvl::VMDMotion *motion = i.value();
        if (motion->status() == vpvl::VMDMotion::kStopped && motion->isReached()) {
            vpvl::PMDModel *model = i.key();
            ret.insert(model, motion);
        }
    }
    return ret;
}


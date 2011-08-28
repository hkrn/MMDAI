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
class XModel;
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
    vpvl::XModel *loadAsset(const QString &baseName, const QDir &dir);
    vpvl::VMDMotion *loadCameraMotion(const QString &path);
    vpvl::PMDModel *loadModel(const QString &baseName, const QDir &dir, vpvl::VMDMotion *&motion);
    vpvl::VMDMotion *loadModelMotion(const QString &path);
    vpvl::VMDMotion *loadModelMotion(const QString &path, QList<vpvl::PMDModel *> &models);
    vpvl::VMDMotion *loadModelMotion(const QString &path, vpvl::PMDModel *model);
    VPDFile *loadPose(const QString &path, vpvl::PMDModel *model);
    void setModelMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);

private:
    vpvl::gl::Renderer *m_renderer;
    vpvl::VMDMotion *m_camera;
    QHash<QString, vpvl::PMDModel *> m_models;
    QHash<QString, vpvl::XModel *> m_assets;
    QHash<vpvl::PMDModel *, vpvl::VMDMotion *> m_motions;

    Q_DISABLE_COPY(SceneLoader)
};

#endif // SCENELOADER_H

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

#include <qglobal.h>

#include "SceneLoader.h"
#include "VPDFile.h"
#include "util.h"

#include <QtCore/QtCore>
#include <vpvl/vpvl.h>

#ifdef VPVL_ENABLE_GLSL
#include <vpvl/gl2/Renderer.h>
using namespace vpvl::gl2;
#else
#include <vpvl/gl/Renderer.h>
using namespace vpvl::gl;
#endif

namespace
{

class Delegate : public vpvl::Project::IDelegate
{
public:
    Delegate()
        : m_codec(0)
    {
        m_codec = QTextCodec::codecForName("Shift-JIS");
    }
    ~Delegate()
    {
    }

    const std::string toUnicode(const std::string &value) const {
        return m_codec->toUnicode(value.c_str()).toStdString();
    }
    const std::string fromUnicode(const std::string &value) const {
        const QByteArray &bytes = m_codec->fromUnicode(value.c_str());
        return std::string(bytes.constData(), bytes.length());
    }
    void error(const char *format, va_list ap) {
        qWarning("[ERROR: %s]", QString("").vsprintf(format, ap).toUtf8().constData());
    }
    void warning(const char *format, va_list ap) {
        qWarning("[ERROR: %s]", QString("").vsprintf(format, ap).toUtf8().constData());
    }

private:
    QTextCodec *m_codec;
};

static const vpvl::Vector3 UIGetVector3(const std::string &value, const vpvl::Vector3 &def)
{
    if (!value.empty()) {
        QStringList gravity = QString::fromStdString(value).split(",");
        if (gravity.length() == 3) {
            float x = gravity.at(0).toFloat();
            float y = gravity.at(1).toFloat();
            float z = gravity.at(2).toFloat();
            return vpvl::Vector3(x, y, z);
        }
    }
    return def;
}

}

bool SceneLoader::isAccelerationSupported()
{
    return Renderer::isAcceleratorSupported();
}

SceneLoader::SceneLoader(Renderer *renderer)
    : QObject(),
      m_renderer(renderer),
      m_project(0),
      m_delegate(0),
      m_asset(0),
      m_camera(0)
{
    m_delegate = new Delegate();
    m_project = new vpvl::Project(m_delegate);
    /*
     * デフォルトではグリッド表示と物理演算を有効にするため、設定後強制的に dirty フラグを無効にする
     * これによってアプリケーションを起動して何もしないまま終了する際の保存ダイアログを抑制する
     */
    m_project->setGlobalSetting("grid.visible", "true");
    m_project->setGlobalSetting("physics.enabled", "true");
    m_project->setDirty(false);
}

SceneLoader::~SceneLoader()
{
    release();
}

void SceneLoader::addModel(vpvl::PMDModel *model, const QString &baseName, const QDir &dir, QUuid &uuid)
{
    /* モデル名が空っぽの場合はファイル名から補完しておく */
    const QString &key = internal::toQString(model).trimmed();
    if (key.isEmpty()) {
        const QByteArray &bytes = internal::fromQString(baseName);
        model->setName(reinterpret_cast<const uint8_t *>(bytes.constData()));
    }
    /*
     * モデルをレンダリングエンジンに渡してレンダリング可能な状態にする
     * upload としているのは GPU (サーバ) にテクスチャや頂点を渡すという意味合いのため
     */
    m_renderer->uploadModel(model, std::string(dir.absolutePath().toLocal8Bit()));
    /* モデルを SceneLoader にヒモ付けする */
    const QString &path = dir.absoluteFilePath(baseName);
    uuid = QUuid::createUuid();
    m_project->addModel(model, uuid.toString().toStdString());
    m_project->setModelSetting(model, vpvl::Project::kSettingNameKey, key.toStdString());
    m_project->setModelSetting(model, vpvl::Project::kSettingURIKey, path.toStdString());
    m_project->setModelSetting(model, "selected", "false");
    emit modelDidAdd(model, uuid);
}

QList<vpvl::PMDModel *> SceneLoader::allModels() const
{
    const vpvl::Project::UUIDList &uuids = m_project->modelUUIDs();
    QList<vpvl::PMDModel *> models;
    vpvl::Project::UUIDList::const_iterator it = uuids.begin(), end = uuids.end();
    while (it != end) {
        models.append(m_project->model(*it));
        it++;
    }
    return models;
}

void SceneLoader::createProject()
{
    if (!m_project)
        m_project = new vpvl::Project(m_delegate);
}

void SceneLoader::deleteAsset(vpvl::Asset *asset)
{
    /* アクセサリをレンダリングエンジンから削除し、SceneLoader のヒモ付けも解除する */
    if (asset && m_project->containsAsset(asset)) {
        const QUuid uuid(m_project->assetUUID(asset).c_str());
        emit assetWillDelete(asset, uuid);
        m_renderer->deleteAsset(asset);
        m_project->deleteAsset(asset);
    }
}

void SceneLoader::deleteCameraMotion()
{
    /* カメラモーションをシーンから解除及び削除し、最初の視点に戻しておく */
    vpvl::Scene *scene = m_renderer->scene();
    scene->setCameraMotion(0);
    scene->resetCamera();
    delete m_camera;
    m_camera = 0;
}

void SceneLoader::deleteModel(vpvl::PMDModel *&model)
{
    /*
     * まずモデルに紐づいたモーションを全て削除し、その後にモデルをレンダリングエンジンから削除し、
     * Project クラスから論理削除する。順番が重要でこの順番で行う必要があり、変更してはいけない。
     * deleteModel の引数は delete した上で 0 にされるので、delete される前のポインタを保持しておき、
     * Project で論理削除する(二重解放になるので Project クラスで物理削除してはいけない)。
     *
     * vpvl::Project にひもづけられるモーションの削除の観点を忘れていたので、
     * モデルに属するモーションを Project から解除するように変更
     */
    if (m_project->containsModel(model)) {
        const QUuid uuid(m_project->modelUUID(model).c_str());
        emit modelWillDelete(model, uuid);
        vpvl::PMDModel *ptr = model;
        const vpvl::Array<vpvl::VMDMotion *> &motions = model->motions();
        int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            vpvl::VMDMotion *motion = motions[i];
            m_project->deleteMotion(motion, model);
        }
        m_renderer->deleteModel(model);
        m_renderer->setSelectedModel(0);
        m_project->removeModel(ptr);
    }
}

void SceneLoader::deleteMotion(vpvl::VMDMotion *&motion)
{
    const QUuid uuid(m_project->motionUUID(motion).c_str());
    emit motionWillDelete(motion, uuid);
    m_project->deleteMotion(motion, motion->parentModel());
}

vpvl::Asset *SceneLoader::findAsset(const QUuid &uuid) const
{
    return m_project->asset(uuid.toString().toStdString());
}

vpvl::PMDModel *SceneLoader::findModel(const QUuid &uuid) const
{
    return m_project->model(uuid.toString().toStdString());
}

vpvl::VMDMotion *SceneLoader::findMotion(const QUuid &uuid) const
{
    return m_project->motion(uuid.toString().toStdString());
}

const QUuid SceneLoader::findUUID(vpvl::Asset *asset) const
{
    return QUuid(m_project->assetUUID(asset).c_str());
}

const QUuid SceneLoader::findUUID(vpvl::PMDModel *model) const
{
    return QUuid(m_project->modelUUID(model).c_str());
}

bool SceneLoader::isProjectModified() const
{
    return m_project->isDirty();
}

vpvl::Asset *SceneLoader::loadAsset(const QString &baseName, const QDir &dir, QUuid &uuid)
{
    QFile file(dir.absoluteFilePath(baseName));
    vpvl::Asset *asset = 0;
    /*
     * アクセサリをファイルから読み込み、レンダリングエンジンに渡してレンダリング可能な状態にする
     */
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        asset = new vpvl::Asset();
        if (asset->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            /* PMD と違って名前を格納している箇所が無いので、アクセサリのファイル名をアクセサリ名とする */
            const QByteArray &assetName = baseName.toUtf8();
            int len = assetName.size();
            char *rawName = new char[len + 1];
            strncpy(rawName, assetName.constData(), len);
            asset->setName(rawName);
            const std::string &name = std::string(dir.absolutePath().toLocal8Bit());
            m_renderer->uploadAsset(asset, name);
            const QString &filename = dir.absoluteFilePath(baseName);
            uuid = QUuid::createUuid();
            m_project->addAsset(asset, uuid.toString().toStdString());
            m_project->setAssetSetting(asset, vpvl::Project::kSettingNameKey, baseName.toStdString());
            m_project->setAssetSetting(asset, vpvl::Project::kSettingURIKey, filename.toStdString());
            m_project->setAssetSetting(asset, "selected", "false");
            emit assetDidAdd(asset, uuid);
        }
        else {
            delete asset;
            asset = 0;
        }
    }
    return asset;
}

vpvl::Asset *SceneLoader::loadAssetFromMetadata(const QString &baseName, const QDir &dir, QUuid &uuid)
{
    QFile file(dir.absoluteFilePath(baseName));
    /* VAC 形式からアクセサリを読み込む。VAC は Shift_JIS で読み込む必要がある */
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("Shift-JIS");
        /* 1行目: アクセサリ名 */
        const QString &name = stream.readLine();
        /* 2行目: ファイル名 */
        const QString &filename = stream.readLine();
        /* 3行目: アクセサリの拡大率 */
        float scaleFactor = stream.readLine().toFloat();
        /* 4行目: アクセサリの位置パラメータ */
        const QStringList &position = stream.readLine().split(',');
        /* 5行目: アクセサリの回転パラメータ */
        const QStringList &rotation = stream.readLine().split(',');
        /* 6行目: アクセサリに紐付ける親ボーン(未実装) */
        const QString &bone = stream.readLine();
        /* 7行目: 影をつけるかどうか(未実装) */
        bool enableShadow = stream.readLine().toInt() == 1;
        vpvl::Asset *asset = loadAsset(filename, dir, uuid);
        if (asset) {
            if (!name.isEmpty()) {
                const QByteArray &bytes = internal::fromQString(name);
                asset->setName(bytes.constData());
            }
            if (!filename.isEmpty()) {
                m_name2assets.insert(filename, asset);
            }
            if (scaleFactor > 0)
                asset->setScaleFactor(scaleFactor);
            if (position.count() == 3) {
                float x = position.at(0).toFloat();
                float y = position.at(1).toFloat();
                float z = position.at(2).toFloat();
                asset->setPosition(vpvl::Vector3(x, y, z));
            }
            if (rotation.count() == 3) {
                float x = rotation.at(0).toFloat();
                float y = rotation.at(1).toFloat();
                float z = rotation.at(2).toFloat();
                asset->setRotation(vpvl::Quaternion(x, y, z));
            }
            vpvl::PMDModel *model = m_renderer->selectedModel();
            if (!bone.isEmpty() && model) {
                const QByteArray &bytes = internal::fromQString(name);
                vpvl::Bone *bone = model->findBone(reinterpret_cast<const uint8_t *>(bytes.constData()));
                asset->setParentBone(bone);
            }
            Q_UNUSED(enableShadow);
        }
        return asset;
    }
    else {
        qWarning("Cannot load %s: %s", qPrintable(baseName), qPrintable(file.errorString()));
        return 0;
    }
}

vpvl::VMDMotion *SceneLoader::loadCameraMotion(const QString &path)
{
    /* カメラモーションをファイルから読み込み、場面オブジェクトに設定する */
    QFile file(path);
    vpvl::VMDMotion *motion = 0;
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        motion = new vpvl::VMDMotion();
        if (motion->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())
                && motion->cameraAnimation().countKeyframes() > 0) {
            setCameraMotion(motion);
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
    /*
     * モデルをファイルから読み込む。レンダリングエンジンに送るには addModel を呼び出す必要がある
     * (確認ダイアログを出す必要があるので、読み込みとレンダリングエンジンへの追加は別処理)
     */
    const QString &path = dir.absoluteFilePath(baseName);
    QFile file(path);
    vpvl::PMDModel *model = 0;
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
        model = new vpvl::PMDModel();
        if (!model->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            delete model;
            model = 0;
        }
    }
    return model;
}

vpvl::VMDMotion *SceneLoader::loadModelMotion(const QString &path)
{
    /* モーションをファイルから読み込む。モデルへの追加は setModelMotion を使う必要がある */
    QFile file(path);
    vpvl::VMDMotion *motion = 0;
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &data = file.readAll();
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
    /* モーションをファイルから読み込み、対象の全てのモデルに対してモーションを適用する */
    vpvl::VMDMotion *motion = loadModelMotion(path);
    if (motion) {
        const vpvl::Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        int nmodels = modelUUIDs.size();
        for (int i = 0; i < nmodels; i++) {
            vpvl::PMDModel *model = m_project->model(modelUUIDs[i]);
            setModelMotion(motion, model);
            models.append(model);
        }
    }
    return motion;
}

vpvl::VMDMotion *SceneLoader::loadModelMotion(const QString &path, vpvl::PMDModel *model)
{
    /* loadModelMotion に setModelMotion の追加が入ったショートカット的なメソッド */
    vpvl::VMDMotion *motion = loadModelMotion(path);
    if (motion)
        setModelMotion(motion, model);
    return motion;
}


VPDFile *SceneLoader::loadModelPose(const QString &path, vpvl::PMDModel * /* model */)
{
    /* ポーズをファイルから読み込む。処理の関係上 makePose は呼ばない */
    QFile file(path);
    VPDFile *pose = 0;
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        pose = new VPDFile();
        if (pose->load(stream)) {
            // pose->makePose(model);
        }
        else {
            delete pose;
            pose = 0;
        }
    }
    return pose;
}

void SceneLoader::loadProject(const QString &path)
{
    delete m_project;
    m_project = new vpvl::Project(m_delegate);
    bool ret = m_project->load(path.toLocal8Bit().constData());
    if (ret) {
        QList<vpvl::PMDModel *> flm;
        /* vpvl::Project はモデルのインスタンスを作成しか行わないので、ここでモデルとそのリソースの読み込みを行う */
        const vpvl::Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        int nmodels = modelUUIDs.size();
        for (int i = 0; i < nmodels; i++) {
            vpvl::PMDModel *model = m_project->model(modelUUIDs[i]);
            const std::string &name = m_project->modelSetting(model, vpvl::Project::kSettingNameKey);
            const std::string &uri = m_project->modelSetting(model, vpvl::Project::kSettingURIKey);
            QFile file(QString::fromStdString(uri));
            if (file.open(QFile::ReadOnly)) {
                const QByteArray &bytes = file.readAll();
                if (model->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
                    m_renderer->uploadModel(model, QFileInfo(file).dir().absolutePath().toStdString());
                    /* ModelInfoWidget でエッジ幅の値を設定するので modelDidSelect を呼ぶ前に設定する */
                    const vpvl::Vector3 &color = UIGetVector3(m_project->modelSetting(model, "edge.color"), vpvl::kZeroV);
                    model->setEdgeColor(vpvl::Color(color.x(), color.y(), color.z(), 1.0));
                    model->setEdgeOffset(QString::fromStdString(m_project->modelSetting(model, "edge.offset")).toFloat());
                    if (isModelSelected(model))
                        setSelectedModel(model);
                    emit modelDidAdd(model, QUuid(m_project->modelUUID(model).c_str()));
                    const vpvl::Array<vpvl::VMDMotion *> &motions = model->motions();
                    const int nmotions = motions.count();
                    for (int i = 0; i < nmotions; i++) {
                        vpvl::VMDMotion *motion = motions[i];
                        motion->reload();
                        emit motionDidAdd(motion, model, QUuid(m_project->motionUUID(motion).c_str()));
                    }
                    continue;
                }
            }
            /* 読み込みに失敗したモデルは後で vpvl::Project から削除するため失敗したリストに追加する */
            qWarning("Model \"%s\" at \"%s\" cannot be loaded: %s",
                     name.c_str(),
                     qPrintable(file.fileName()),
                     qPrintable(file.errorString()));
            flm.append(model);
        }
        /* vpvl::Project はアクセサリのインスタンスを作成しか行わないので、ここでアクセサリとそのリソースの読み込みを行う */
        QList<vpvl::Asset *> fla;
        const vpvl::Project::UUIDList &assetUUIDs = m_project->assetUUIDs();
        int nassets = assetUUIDs.size();
        for (int i = 0; i < nassets; i++) {
            vpvl::Asset *asset = m_project->asset(assetUUIDs[i]);
            const std::string &uri = m_project->assetSetting(asset, vpvl::Project::kSettingURIKey);
            QFile file(QString::fromStdString(uri));
            if (file.open(QFile::ReadOnly)) {
                const QByteArray &bytes = file.readAll();
                if (asset->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
                    m_renderer->uploadAsset(asset, QFileInfo(file).dir().absolutePath().toStdString());
                    emit assetDidAdd(asset, QUuid(m_project->assetUUID(asset).c_str()));
                    if (isAssetSelected(asset))
                        setSelectedAsset(asset);
                    continue;
                }
            }
            /* 読み込みに失敗したアクセサリは後で vpvl::Project から削除するため失敗したリストに追加する */
            qWarning("Asset %s at %s cannot be loaded: %s",
                     qPrintable(internal::toQString(asset)),
                     qPrintable(file.fileName()),
                     qPrintable(file.errorString()));
            fla.append(asset);
        }
        /* 読み込みに失敗したモデルとアクセサリを vpvl::Project から削除する */
        foreach (vpvl::PMDModel *model, flm)
            m_project->deleteModel(model);
        foreach (vpvl::Asset *asset, fla)
            m_project->deleteAsset(asset);
        m_project->setDirty(false);
        /* FIXME: モデルとアクセサリ、モーションの追加の通知 */
        emit projectDidLoad();
    }
}

vpvl::VMDMotion *SceneLoader::newCameraMotion() const
{
    /* 0番目に空のキーフレームが入ったカメラのモーションを作成する */
    vpvl::VMDMotion *newCameraMotion = new vpvl::VMDMotion();
    vpvl::CameraAnimation *cameraAnimation = newCameraMotion->mutableCameraAnimation();
    vpvl::CameraKeyframe *frame = new vpvl::CameraKeyframe();
    vpvl::Scene *scene = m_renderer->scene();
    frame->setDefaultInterpolationParameter();
    frame->setPosition(scene->cameraPosition());
    frame->setAngle(scene->cameraAngle());
    frame->setFovy(scene->fovy());
    frame->setDistance(scene->cameraDistance());
    cameraAnimation->addKeyframe(frame);
    return newCameraMotion;
}

vpvl::VMDMotion *SceneLoader::newModelMotion(vpvl::PMDModel *model) const
{
    /* 全ての可視ボーンと頂点モーフに対して0番目に空のキーフレームが入ったモデルのモーションを作成する */
    vpvl::VMDMotion *newModelMotion = 0;
    if (model) {
        newModelMotion = new vpvl::VMDMotion();
        const vpvl::BoneList &bones = model->bones();
        const int nbones = bones.count();
        vpvl::BoneAnimation *boneAnimation = newModelMotion->mutableBoneAnimation();
        for (int i = 0; i < nbones; i++) {
            vpvl::Bone *bone = bones[i];
            if (bone->isMovable() || bone->isRotateable()) {
                vpvl::BoneKeyframe *frame = new vpvl::BoneKeyframe();
                frame->setDefaultInterpolationParameter();
                frame->setName(bone->name());
                boneAnimation->addKeyframe(frame);
            }
        }
        const vpvl::FaceList &faces = model->faces();
        const int nfaces = faces.count();
        vpvl::FaceAnimation *faceAnimation = newModelMotion->mutableFaceAnimation();
        for (int i = 0; i < nfaces; i++) {
            vpvl::Face *face = faces[i];
            vpvl::FaceKeyframe *frame = new vpvl::FaceKeyframe();
            frame->setName(face->name());
            faceAnimation->addKeyframe(frame);
        }
    }
    return newModelMotion;
}

void SceneLoader::release()
{
    /*
      releaseProject は Project 内にある全ての Asset と PMDModel のインスタンスを Renderer クラスから物理削除し、
      Project クラスから論理削除 (remove*) を行う。Project が物理削除 (delete*) を行なってしまうと Renderer クラスで
      物理削除した時二重削除となってしまい不正なアクセスが発生するため、Project 側は論理削除だけにとどめておく必要がある。
     */
    m_renderer->releaseProject(m_project);
    delete m_project;
    m_project = 0;
    delete m_camera;
    m_camera = 0;
}

void SceneLoader::saveMetadataFromAsset(const QString &path, vpvl::Asset *asset)
{
    /* 現在のアセットの位置情報からファイルに書き出す。行毎の意味は loadMetadataFromAsset を参照 */
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream.setCodec("Shift-JIS");
        const char lineSeparator[] = "\r\n";
        stream << internal::toQString(asset) << lineSeparator;
        stream << m_name2assets.key(asset) << lineSeparator;
        stream << asset->scaleFactor() << lineSeparator;
        const vpvl::Vector3 &position = asset->position();
        stream << QString("%1,%2,%3").arg(position.x(), 0, 'f', 1)
                  .arg(position.y(), 0, 'f', 1).arg(position.z(), 0, 'f', 1) << lineSeparator;
        const vpvl::Quaternion &rotation = asset->rotation();
        stream << QString("%1,%2,%3").arg(rotation.x(), 0, 'f', 1)
                  .arg(rotation.y(), 0, 'f', 1).arg(rotation.z(), 0, 'f', 1) << lineSeparator;
        const vpvl::Bone *bone = asset->parentBone();
        stream << (bone ? internal::toQString(bone) : "地面") << lineSeparator;
        stream << 1 << lineSeparator;
    }
    else {
        qWarning("Cannot load %s: %s", qPrintable(path), qPrintable(file.errorString()));
    }
}

void SceneLoader::saveProject(const QString &path)
{
    m_project->save(path.toLocal8Bit().constData());
    emit projectDidSave();
}

void SceneLoader::setCameraMotion(vpvl::VMDMotion *motion)
{
    const QUuid &uuid = QUuid::createUuid();
    m_project->deleteMotion(m_camera, 0);
    m_camera = motion;
    m_project->addMotion(motion, 0, uuid.toString().toStdString());
    m_renderer->scene()->setCameraMotion(motion);
    emit cameraMotionDidSet(motion, uuid);
}

void SceneLoader::setModelMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    const QUuid &uuid = QUuid::createUuid();
#ifndef QMA_ENABLE_MULTIPLE_MOTION
    const vpvl::Array<vpvl::VMDMotion *> &motions = model->motions();
    const int nmotions = motions.count();
    for (int i = 0; i < nmotions; i++) {
        /* 先に PMDModel#deleteMotion を呼んでから Project#removeMotion を呼ばないとメモリリークになる */
        vpvl::VMDMotion *motion = motions[i], *ptr = motion;
        model->deleteMotion(motion);
        m_project->removeMotion(ptr, model);
    }
#endif
    model->addMotion(motion);
    m_project->addMotion(motion, model, uuid.toString().toStdString());
    emit motionDidAdd(motion, model, uuid);
}

const vpvl::Vector3 SceneLoader::worldGravity() const
{
    static const vpvl::Vector3 defaultGravity(0.0, -9.8, 0.0);
    return m_project ? UIGetVector3(m_project->globalSetting("physics.gravity"), defaultGravity) : defaultGravity;
}

void SceneLoader::setWorldGravity(const vpvl::Vector3 &value)
{
    if (m_project) {
        QString str;
        str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
        m_project->setGlobalSetting("physics.gravity", str.toStdString());
    }
}

bool SceneLoader::isProjectiveShadowEnabled(const vpvl::PMDModel *model) const
{
    const std::string &value = m_project->modelSetting(model, "projective_shadow");
    return value == "true";
}

void SceneLoader::setProjectiveShadowEnable(const vpvl::PMDModel *model, bool value)
{
    m_project->setModelSetting(model, "projective_shadow", value ? "true" : "false");
}

vpvl::PMDModel *SceneLoader::selectedModel() const
{
    return m_renderer->selectedModel();
}

bool SceneLoader::isModelSelected(const vpvl::PMDModel *value) const
{
    return m_project->modelSetting(value, "selected") == "true";
}

void SceneLoader::setSelectedModel(vpvl::PMDModel *value)
{
    m_renderer->setSelectedModel(value);
    m_project->setModelSetting(value, "selected", "true");
    emit modelDidSelect(value, this);
}

void SceneLoader::setModelEdgeOffset(vpvl::PMDModel *model, float value)
{
    QString str;
    str.sprintf("%.5f", value);
    model->setEdgeOffset(value);
    m_project->setModelSetting(model, "edge.offset", str.toStdString());
}

void SceneLoader::setModelEdgeColor(vpvl::PMDModel *model, const QColor &value)
{
    QString str;
    float red = value.redF(), green = value.greenF(), blue = value.blueF();
    str.sprintf("%.5f,%.5f,%.5f", red, green, blue);
    model->setEdgeColor(vpvl::Color(red, green, blue, 1.0));
    m_project->setModelSetting(model, "edge.color", str.toStdString());
}

bool SceneLoader::isGridVisible() const
{
    return m_project ? m_project->globalSetting("grid.visible") == "true" : true;
}

void SceneLoader::setGridVisible(bool value)
{
    /* 値が同じの場合は更新しない (Qt のスロット/シグナル経由での setDirty を抑制する) */
    if (m_project && isGridVisible() != value)
        m_project->setGlobalSetting("grid.visible", value ? "true" : "false");
}

bool SceneLoader::isPhysicsEnabled() const
{
    return m_project ? m_project->globalSetting("physics.enabled") == "true" : false;
}

void SceneLoader::setPhysicsEnabled(bool value)
{
    /* 値が同じの場合は更新しない (Qt のスロット/シグナル経由での setDirty を抑制する) */
    if (m_project && isPhysicsEnabled() != value)
        m_project->setGlobalSetting("physics.enabled", value ? "true" : "false");
}

bool SceneLoader::isAccelerationEnabled() const
{
    return m_project ? m_project->globalSetting("acceleration.enabled") == "true" : false;
}

void SceneLoader::setAccelerationEnabled(bool value)
{
    /* アクセレーションをサポートする場合のみ有効にする。しない場合は常に無効に設定 */
    if (isAccelerationSupported()) {
        if (value) {
            if (m_renderer->initializeAccelerator()) {
                m_renderer->scene()->setSoftwareSkinningEnable(false);
                if (m_project && !isAccelerationEnabled())
                    m_project->setGlobalSetting("acceleration.enabled", "true");
                return;
            }
            else {
                qWarning("%s", qPrintable(tr("Failed enabling acceleration and set fallback.")));
            }
        }
    }
    else {
        qWarning("%s", qPrintable(tr("Acceleration is not supported on this platform and set fallback.")));
    }
    m_renderer->scene()->setSoftwareSkinningEnable(true);
    if (m_project && isAccelerationEnabled())
        m_project->setGlobalSetting("acceleration.enabled", "false");
}

bool SceneLoader::isBlackBackgroundEnabled() const
{
    return m_project ? m_project->globalSetting("background.black") == "true" : false;
}

void SceneLoader::setBlackBackgroundEnabled(bool value)
{
    /* 値が同じの場合は更新しない (Qt のスロット/シグナル経由での setDirty を抑制する) */
    if (m_project && isBlackBackgroundEnabled() != value)
        m_project->setGlobalSetting("background.black", value ? "true" : "false");
}

const vpvl::Vector3 SceneLoader::assetPosition(const vpvl::Asset *asset)
{
    return UIGetVector3(m_project->assetSetting(asset, "position"), vpvl::kZeroV);
}

void SceneLoader::setAssetPosition(const vpvl::Asset *asset, const vpvl::Vector3 &value)
{
    QString str;
    str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
    m_project->setAssetSetting(asset, "position", str.toStdString());
}

const vpvl::Vector3 SceneLoader::assetRotation(const vpvl::Asset *asset)
{
    return UIGetVector3(m_project->assetSetting(asset, "rotation"), vpvl::kZeroV);
}

void SceneLoader::setAssetRotation(const vpvl::Asset *asset, const vpvl::Vector3 &value)
{
    QString str;
    str.sprintf("%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
    m_project->setAssetSetting(asset, "rotation", str.toStdString());
}

float SceneLoader::assetOpacity(const vpvl::Asset *asset)
{
    float value = QString::fromStdString(m_project->assetSetting(asset, "opacity")).toFloat();
    return qBound(0.0f, value, 1.0f);
}

void SceneLoader::setAssetOpacity(const vpvl::Asset *asset, float value)
{
    QString str;
    str.sprintf("%.5f", value);
    m_project->setAssetSetting(asset, "opacity", str.toStdString());
}

float SceneLoader::assetScale(const vpvl::Asset *asset)
{
    float value = QString::fromStdString(m_project->assetSetting(asset, "scale")).toFloat();
    return qBound(0.0001f, value, 10000.0f);
}

void SceneLoader::setAssetScale(const vpvl::Asset *asset, float value)
{
    QString str;
    str.sprintf("%.5f", value);
    m_project->setAssetSetting(asset, "scale", str.toStdString());
}

vpvl::Asset *SceneLoader::selectedAsset() const
{
    return m_asset;
}

bool SceneLoader::isAssetSelected(const vpvl::Asset *value) const
{
    return m_project->assetSetting(value, "selected") == "true";
}

void SceneLoader::setSelectedAsset(vpvl::Asset *value)
{
    m_asset = value;
    m_project->setAssetSetting(value, "selected", "true");
    emit assetDidSelect(value, this);
}

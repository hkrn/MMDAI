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

/*
 * Renderer と Project の二重管理を行うため、メモリ解放にまつわる実装がややこしいことになっている。
 * 解放手順として以下を必ず行うようにする。
 *
 * 1. Project#remove*() を呼び出す
 * 2. Renderer#delete*() を呼び出す
 *^
 * このようにするのは Renderer が独自のデータを保持しているため。Project は独自のデータを持たないので、
 * remove*() を呼び出して論理削除しても問題ないが、Renderer は上記の理由により delete*() しか呼び出せない。
 * また、delete*() は引数のポインタを解放後 0 にするという特性を持つため、先に remove*() を呼び出す理由になっている。
 */

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

static const vpvl::Quaternion UIGetQuaternion(const std::string &value, const vpvl::Quaternion &def)
{
    if (!value.empty()) {
        QStringList gravity = QString::fromStdString(value).split(",");
        if (gravity.length() == 4) {
            float x = gravity.at(0).toFloat();
            float y = gravity.at(1).toFloat();
            float z = gravity.at(2).toFloat();
            float w = gravity.at(3).toFloat();
            return vpvl::Quaternion(x, y, z, w);
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
      m_model(0),
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

void SceneLoader::commitAssetProperties()
{
    if (m_asset) {
        setAssetPosition(m_asset, m_asset->position());
        setAssetRotation(m_asset, m_asset->rotation());
        setAssetOpacity(m_asset, m_asset->opacity());
        setAssetScaleFactor(m_asset, m_asset->scaleFactor());
        setAssetParentModel(m_asset, m_asset->parentModel());
        setAssetParentBone(m_asset, m_asset->parentBone());
    }
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
        m_project->removeAsset(asset);
        m_renderer->deleteAsset(asset);
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
     * メモリ解放に関しては最初の部分を参照
     *
     * vpvl::Project にひもづけられるモーションの削除の観点を忘れていたので、
     * モデルに属するモーションを Project から解除するように変更
     */
    if (m_project->containsModel(model)) {
        const QUuid uuid(m_project->modelUUID(model).c_str());
        emit modelWillDelete(model, uuid);
        const vpvl::Array<vpvl::VMDMotion *> &motions = model->motions();
        int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            vpvl::VMDMotion *motion = motions[i];
            m_project->deleteMotion(motion, model);
        }
        m_project->removeModel(model);
        m_renderer->deleteModel(model);
        m_model = 0;
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
            setAssetPosition(asset, asset->position());
            setAssetRotation(asset, asset->rotation());
            setAssetOpacity(asset, asset->opacity());
            setAssetScaleFactor(asset, asset->scaleFactor());
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
            if (!bone.isEmpty() && m_model) {
                const QByteArray &bytes = internal::fromQString(name);
                vpvl::Bone *bone = m_model->findBone(reinterpret_cast<const uint8_t *>(bytes.constData()));
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
    release();
    createProject();
    bool ret = m_project->load(path.toLocal8Bit().constData());
    if (ret) {
        if (isAccelerationEnabled()) {
            if (m_renderer->initializeAccelerator())
                m_renderer->scene()->setSoftwareSkinningEnable(false);
        }
        int progress = 0;
        QList<vpvl::PMDModel *> lostModels;
        QList<vpvl::Asset *> lostAssets;
        const vpvl::Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
        const vpvl::Project::UUIDList &assetUUIDs = m_project->assetUUIDs();
        emit projectDidCount(modelUUIDs.size() + assetUUIDs.size());
        /* vpvl::Project はモデルのインスタンスを作成しか行わないので、ここでモデルとそのリソースの読み込みを行う */
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
                    emit modelDidAdd(model, QUuid(m_project->modelUUID(model).c_str()));
                    if (isModelSelected(model))
                        setSelectedModel(model);
                    const vpvl::Array<vpvl::VMDMotion *> &motions = model->motions();
                    const int nmotions = motions.count();
                    for (int i = 0; i < nmotions; i++) {
                        vpvl::VMDMotion *motion = motions[i];
                        motion->reload();
                        emit motionDidAdd(motion, model, QUuid(m_project->motionUUID(motion).c_str()));
                    }
                    emit projectDidProceed(++progress);
                    continue;
                }
            }
            /* 読み込みに失敗したモデルは後で vpvl::Project から削除するため失敗したリストに追加する */
            qWarning("Model \"%s\" at \"%s\" cannot be loaded: %s",
                     name.c_str(),
                     qPrintable(file.fileName()),
                     qPrintable(file.errorString()));
            lostModels.append(model);
            emit projectDidProceed(++progress);
        }
        /* vpvl::Project はアクセサリのインスタンスを作成しか行わないので、ここでアクセサリとそのリソースの読み込みを行う */
        int nassets = assetUUIDs.size();
        for (int i = 0; i < nassets; i++) {
            vpvl::Asset *asset = m_project->asset(assetUUIDs[i]);
            const std::string &uri = m_project->assetSetting(asset, vpvl::Project::kSettingURIKey);
            QFile file(QString::fromStdString(uri));
            if (file.open(QFile::ReadOnly)) {
                const QByteArray &bytes = file.readAll();
                if (asset->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
                    asset->setName(QFileInfo(file).fileName().toUtf8().constData());
                    m_renderer->uploadAsset(asset, QFileInfo(file).dir().absolutePath().toStdString());
                    emit assetDidAdd(asset, QUuid(m_project->assetUUID(asset).c_str()));
                    if (isAssetSelected(asset))
                        setSelectedAsset(asset);
                    emit projectDidProceed(++progress);
                    continue;
                }
            }
            /* 読み込みに失敗したアクセサリは後で vpvl::Project から削除するため失敗したリストに追加する */
            qWarning("Asset %s at %s cannot be loaded: %s",
                     qPrintable(internal::toQString(asset)),
                     qPrintable(file.fileName()),
                     qPrintable(file.errorString()));
            lostAssets.append(asset);
            emit projectDidProceed(++progress);
        }
        /* 読み込みに失敗したモデルとアクセサリを vpvl::Project から削除する */
        foreach (vpvl::PMDModel *model, lostModels)
            m_project->deleteModel(model);
        foreach (vpvl::Asset *asset, lostAssets)
            m_project->deleteAsset(asset);
        m_project->setDirty(false);
        emit projectDidLoad(true);
    }
    else {
        qDebug("Failed loading project %s", qPrintable(path));
        emit projectDidLoad(false);
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
    /* やっていることは削除ではなくシグナル発行すること以外 Renderer::releaseProject と同じ */
    const vpvl::Project::UUIDList &assetUUIDs = m_project->assetUUIDs();
    for (vpvl::Project::UUIDList::const_iterator it = assetUUIDs.begin(); it != assetUUIDs.end(); it++) {
        const vpvl::Project::UUID &assetUUID = *it;
        emit assetWillDelete(m_project->asset(assetUUID), QUuid(assetUUID.c_str()));
    }
    const vpvl::Project::UUIDList &modelUUIDs = m_project->modelUUIDs();
    for (vpvl::Project::UUIDList::const_iterator it = modelUUIDs.begin(); it != modelUUIDs.end(); it++) {
        const vpvl::Project::UUID &modelUUID = *it;
        vpvl::PMDModel *model = m_project->model(modelUUID);
        const vpvl::Array<vpvl::VMDMotion *> &motions = model->motions();
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            vpvl::VMDMotion *motion = motions[i];
            const vpvl::Project::UUID &motionUUID = m_project->motionUUID(motion);
            emit motionWillDelete(motion, QUuid(motionUUID.c_str()));
        }
        emit modelWillDelete(model, QUuid(modelUUID.c_str()));
    }
    /*
      releaseProject は Project 内にある全ての Asset と PMDModel のインスタンスを Renderer クラスから物理削除し、
      Project クラスから論理削除 (remove*) を行う。Project が物理削除 (delete*) を行なってしまうと Renderer クラスで
      物理削除した時二重削除となってしまい不正なアクセスが発生するため、Project 側は論理削除だけにとどめておく必要がある。
     */
    m_renderer->releaseProject(m_project);
    m_project->deleteMotion(m_camera, 0);
    delete m_project;
    m_project = 0;
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
    commitAssetProperties();
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
        /* 先にプロジェクトからモーションを論理削除した後にモデルから物理削除する */
        vpvl::VMDMotion *motion = motions[i];
        m_project->removeMotion(motion, model);
        model->deleteMotion(motion);
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
    const std::string &value = m_project->modelSetting(model, "shadow.projective");
    return value == "true";
}

void SceneLoader::setProjectiveShadowEnable(const vpvl::PMDModel *model, bool value)
{
    m_project->setModelSetting(model, "shadow.projective", value ? "true" : "false");
}

vpvl::PMDModel *SceneLoader::selectedModel() const
{
    return m_model;
}

bool SceneLoader::isModelSelected(const vpvl::PMDModel *value) const
{
    return m_project->modelSetting(value, "selected") == "true";
}

void SceneLoader::setSelectedModel(vpvl::PMDModel *value)
{
    m_model = value;
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
    return globalSetting("grid.visible", true);
}

void SceneLoader::setGridVisible(bool value)
{
    /* 値が同じの場合は更新しない (Qt のスロット/シグナル経由での setDirty を抑制する) */
    if (m_project && isGridVisible() != value)
        m_project->setGlobalSetting("grid.visible", value ? "true" : "false");
}

bool SceneLoader::isPhysicsEnabled() const
{
    return globalSetting("physics.enabled", false);
}

void SceneLoader::setPhysicsEnabled(bool value)
{
    /* 値が同じの場合は更新しない (Qt のスロット/シグナル経由での setDirty を抑制する) */
    if (m_project && isPhysicsEnabled() != value)
        m_project->setGlobalSetting("physics.enabled", value ? "true" : "false");
}

bool SceneLoader::isAccelerationEnabled() const
{
    return globalSetting("acceleration.enabled", false);
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
    return  globalSetting("background.black", false);
}

void SceneLoader::setBlackBackgroundEnabled(bool value)
{
    /* 値が同じの場合は更新しない (Qt のスロット/シグナル経由での setDirty を抑制する) */
    if (m_project && isBlackBackgroundEnabled() != value)
        m_project->setGlobalSetting("background.black", value ? "true" : "false");
}

/* 再生設定及びエンコード設定の場合は同値チェックを行わない。こちらは値を確実に保存させる必要があるため */
int SceneLoader::frameIndexPlayFrom() const
{
    return globalSetting("play.frame_index.from", 0);
}

void SceneLoader::setFrameIndexPlayFrom(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.frame_index.from", QVariant(value).toString().toStdString());
}

int SceneLoader::frameIndexPlayTo() const
{
    return globalSetting("play.frame_index.to", 0);
}

void SceneLoader::setFrameIndexPlayTo(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.frame_index.to", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneFPSForPlay() const
{
    return globalSetting("play.fps", 60);
}

void SceneLoader::setSceneFPSForPlay(int value)
{
    if (m_project)
        m_project->setGlobalSetting("play.fps", QVariant(value).toString().toStdString());
}

int SceneLoader::frameIndexEncodeVideoFrom() const
{
    return globalSetting("video.frame_index.from", 0);
}

void SceneLoader::setFrameIndexEncodeVideoFrom(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.frame_index.from", QVariant(value).toString().toStdString());
}

int SceneLoader::frameIndexEncodeVideoTo() const
{
    return globalSetting("video.frame_index.to", 0);
}

void SceneLoader::setFrameIndexEncodeVideoTo(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.frame_index.to", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneFPSForEncodeVideo() const
{
    return globalSetting("video.fps", 60);
}

void SceneLoader::setSceneFPSForEncodeVideo(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.fps", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneWidth() const
{
    return globalSetting("video.width", 0);
}

void SceneLoader::setSceneWidth(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.width", QVariant(value).toString().toStdString());
}

int SceneLoader::sceneHeight() const
{
    return globalSetting("video.height", 0);
}

void SceneLoader::setSceneHeight(int value)
{
    if (m_project)
        m_project->setGlobalSetting("video.height", QVariant(value).toString().toStdString());
}

bool SceneLoader::isLoop() const
{
    return globalSetting("play.loop", false);
}

void SceneLoader::setLoop(bool value)
{
    if (m_project)
        m_project->setGlobalSetting("play.loop", value ? "true" : "false");
}

bool SceneLoader::isGridIncluded() const
{
    return globalSetting("grid.video", false);
}

void SceneLoader::setGridIncluded(bool value)
{
    if (m_project)
        m_project->setGlobalSetting("grid.video", value ? "true" : "false");
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

const vpvl::Quaternion SceneLoader::assetRotation(const vpvl::Asset *asset)
{
    return UIGetQuaternion(m_project->assetSetting(asset, "rotation"), vpvl::kZeroQ);
}

void SceneLoader::setAssetRotation(const vpvl::Asset *asset, const vpvl::Quaternion &value)
{
    QString str;
    str.sprintf("%.5f,%.5f,%.5f,%.5f", value.x(), value.y(), value.z(), value.w());
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

float SceneLoader::assetScaleFactor(const vpvl::Asset *asset)
{
    float value = QString::fromStdString(m_project->assetSetting(asset, "scale")).toFloat();
    return qBound(0.0001f, value, 10000.0f);
}

void SceneLoader::setAssetScaleFactor(const vpvl::Asset *asset, float value)
{
    QString str;
    str.sprintf("%.5f", value);
    m_project->setAssetSetting(asset, "scale", str.toStdString());
}

vpvl::PMDModel *SceneLoader::assetParentModel(vpvl::Asset *asset) const
{
    return m_project->model(m_project->assetSetting(asset, "parent.model"));
}

void SceneLoader::setAssetParentModel(const vpvl::Asset *asset, vpvl::PMDModel *model)
{
    m_project->setAssetSetting(asset, "parent.model", m_project->modelUUID(model));
}

vpvl::Bone *SceneLoader::assetParentBone(vpvl::Asset *asset) const
{
    if (vpvl::PMDModel *model = assetParentModel(asset)) {
        const QString &name = QString::fromStdString(m_project->assetSetting(asset, "parent.bone"));
        const QByteArray &bytes = internal::fromQString(name);
        return model->findBone(reinterpret_cast<const uint8_t *>(bytes.constData()));
    }
    return 0;
}

void SceneLoader::setAssetParentBone(const vpvl::Asset *asset, vpvl::Bone *bone)
{
    m_project->setAssetSetting(asset, "parent.bone", internal::toQString(bone).toStdString());
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
    commitAssetProperties();
    m_asset = value;
    m_project->setAssetSetting(value, "selected", "true");
    emit assetDidSelect(value, this);
}

bool SceneLoader::globalSetting(const char *key, bool def) const
{
    return m_project ? m_project->globalSetting(key) == "true" : def;
}

int SceneLoader::globalSetting(const char *key, int def) const
{
    return m_project ? QString::fromStdString(m_project->globalSetting(key)).toInt() : def;
}

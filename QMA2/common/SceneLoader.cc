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

#include <qglobal.h>
#ifdef Q_OS_WIN
#include <GL/glew.h>
#endif /* Q_OS_WIN */

#include "SceneLoader.h"
#include "VPDFile.h"
#include "util.h"

#include <QtCore/QtCore>
#include <vpvl/vpvl.h>

#ifdef VPVL_USE_GLSL
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

    const std::string toUnicode(const std::string &value) {
        return m_codec->toUnicode(value.c_str()).toStdString();
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

}

SceneLoader::SceneLoader(Renderer *renderer)
    : m_renderer(renderer),
      m_project(0),
      m_delegate(0),
      m_camera(0)
{
    m_delegate = new Delegate();
    m_project = new vpvl::Project(m_delegate);
}

SceneLoader::~SceneLoader()
{
    release();
}

void SceneLoader::addModel(vpvl::PMDModel *model, const QString &baseName, const QDir &dir)
{
    /* モデル名が空っぽの場合はファイル名から補完しておく */
    QString key = internal::toQString(model).trimmed();
    if (key.isEmpty()) {
        const QByteArray bytes = internal::fromQString(baseName);
        model->setName(reinterpret_cast<const uint8_t *>(bytes.constData()));
        key = baseName;
    }
    /*
     * モデルをレンダリングエンジンに渡してレンダリング可能な状態にする
     * upload としているのは GPU (サーバ) にテクスチャや頂点を渡すという意味合いのため
     */
    m_renderer->uploadModel(model, std::string(dir.absolutePath().toLocal8Bit()));
    /* モデル名が重複していれば連番で名前を作成する */
    if (m_project->modelFromName(key.toStdString())) {
        int i = 0;
        while (true) {
            QString tmpKey = QString("%1%2").arg(key).arg(i);
            if (!m_project->modelFromName(tmpKey.toStdString())) {
                key = tmpKey;
                break;
            }
            i++;
        }
    }
    /* モデルを SceneLoader にヒモ付けする */
    const QString &path = dir.absoluteFilePath(baseName);
    m_project->addModel(model);
    m_project->setModelSetting(model, vpvl::Project::kSettingNameKey, key.toStdString());
    m_project->setModelSetting(model, vpvl::Project::kSettingURIKey, path.toStdString());
}

void SceneLoader::createProject()
{
    if (!m_project)
        m_project = new vpvl::Project(m_delegate);
}

bool SceneLoader::deleteAsset(vpvl::Asset *asset)
{
    if (!asset)
        return false;
    /* アクセサリをレンダリングエンジンから削除し、SceneLoader のヒモ付けも解除する */
    if (m_project->containsAsset(asset)) {
        m_renderer->deleteAsset(asset);
        m_project->deleteAsset(asset);
        return true;
    }
    return false;
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

bool SceneLoader::deleteModel(vpvl::PMDModel *model)
{
    if (!model)
        return false;
    /*
     * まずモデルに紐づいたモーションを削除し、その後にモデルをレンダリングエンジンから削除し、
     * SceneLoader のヒモ付けも解除する
     */
    if (m_project->containsModel(model)) {
        model->deleteAllMotions();
        m_renderer->deleteModel(model);
        m_renderer->setSelectedModel(0);
        m_project->deleteModel(model);
        return true;
    }
    return false;
}

vpvl::PMDModel *SceneLoader::findModel(const QString &name) const
{
    return m_project->modelFromName(name.toStdString());
}

bool SceneLoader::isProjectModified() const
{
    return m_project->isDirty();
}

vpvl::Asset *SceneLoader::loadAsset(const QString &baseName, const QDir &dir)
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
            QString key = baseName;
            /* アクセサリ名が重複していれば連番で名前を作成する */
            if (m_project->assetFromName(key.toStdString())) {
                int i = 0;
                while (true) {
                    QString tmpKey = QString("%1%2").arg(key).arg(i);
                    if (!m_project->assetFromName(tmpKey.toStdString())) {
                        key = tmpKey;
                        break;
                    }
                    i++;
                }
            }
            /* PMD と違って名前を格納している箇所が無いので、アクセサリのファイル名をアクセサリ名とする */
            const QByteArray &assetName = baseName.toUtf8();
            int len = assetName.size();
            char *rawName = new char[len + 1];
            strncpy(rawName, assetName.constData(), len);
            asset->setName(rawName);
            const std::string &name = std::string(dir.absolutePath().toLocal8Bit());
            m_renderer->uploadAsset(asset, name);
            const QString &filename = dir.absoluteFilePath(baseName);
            m_project->addAsset(asset);
            m_project->setAssetSetting(asset, vpvl::Project::kSettingNameKey, baseName.toStdString());
            m_project->setAssetSetting(asset, vpvl::Project::kSettingURIKey, filename.toStdString());
        }
        else {
            delete asset;
            asset = 0;
        }
    }
    return asset;
}

vpvl::Asset *SceneLoader::loadAssetFromMetadata(const QString &baseName, const QDir &dir)
{
    QFile file(dir.absoluteFilePath(baseName));
    /* VAC 形式からアクセサリを読み込む。VAC は Shift_JIS で読み込む必要がある */
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("Shift-JIS");
        /* 1行目: アクセサリ名 */
        QString name = stream.readLine();
        /* 2行目: ファイル名 */
        QString filename = stream.readLine();
        /* 3行目: アクセサリの拡大率 */
        float scaleFactor = stream.readLine().toFloat();
        /* 4行目: アクセサリの位置パラメータ */
        QStringList position = stream.readLine().split(',');
        /* 5行目: アクセサリの回転パラメータ */
        QStringList rotation = stream.readLine().split(',');
        /* 6行目: アクセサリに紐付ける親ボーン(未実装) */
        QString bone = stream.readLine();
        /* 7行目: 影をつけるかどうか(未実装) */
        bool enableShadow = stream.readLine().toInt() == 1;
        vpvl::Asset *asset = loadAsset(filename, dir);
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
                QByteArray bytes = internal::fromQString(name);
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
                && motion->cameraAnimation().countKeyFrames() > 0) {
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
        const vpvl::Array<vpvl::PMDModel *> &m = m_project->models();
        int nmodels = m.count();
        for (int i = 0; i < nmodels; i++) {
            vpvl::PMDModel *model = m.at(i);
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

bool SceneLoader::loadProject(const QString &path)
{
    delete m_project;
    m_project = new vpvl::Project(m_delegate);
    bool ret = m_project->load(path.toLocal8Bit().constData());
    if (ret) {
        QList<vpvl::PMDModel *> flm;
        /* vpvl::Project はモデルのインスタンスを作成しか行わないので、ここでモデルとそのリソースの読み込みを行う */
        const vpvl::Array<vpvl::PMDModel *> &models = m_project->models();
        int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            vpvl::PMDModel *model = models[i];
            const std::string &uri = m_project->modelSetting(model, vpvl::Project::kSettingURIKey);
            QFile file(QString::fromStdString(uri));
            if (file.open(QFile::ReadOnly)) {
                const QByteArray &bytes = file.readAll();
                if (model->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
                    m_renderer->uploadModel(model, QFileInfo(file).dir().absolutePath().toStdString());
                    continue;
                }
            }
            /* 読み込みに失敗したモデルは後で vpvl::Project から削除するため失敗したリストに追加する */
            flm.append(model);
        }
        /* vpvl::Project はアクセサリのインスタンスを作成しか行わないので、ここでアクセサリとそのリソースの読み込みを行う */
        QList<vpvl::Asset *> fla;
        const vpvl::Array<vpvl::Asset *> &assets = m_project->assets();
        int nassets = assets.count();
        for (int i = 0; i < nassets; i++) {
            vpvl::Asset *asset = assets[i];
            const std::string &uri = m_project->assetSetting(asset, vpvl::Project::kSettingURIKey);
            QFile file(QString::fromStdString(uri));
            if (file.open(QFile::ReadOnly)) {
                const QByteArray &bytes = file.readAll();
                if (asset->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
                    m_renderer->uploadAsset(asset, QFileInfo(file).dir().absolutePath().toStdString());
                    continue;
                }
            }
            /* 読み込みに失敗したアクセサリは後で vpvl::Project から削除するため失敗したリストに追加する */
            fla.append(asset);
        }
        /* 読み込みに失敗したモデルとアクセサリを vpvl::Project から削除する */
        foreach (vpvl::PMDModel *model, flm)
            m_project->deleteModel(model);
        foreach (vpvl::Asset *asset, fla)
            m_project->deleteAsset(asset);
        /* ここで PMDModel と VMDMotion のヒモ付をする必要があるのだが vpvl::Project が該当部分についてまだ未実装 */
    }
    return ret;
}

void SceneLoader::release()
{
    /*
      releaseProject は Project 内にある全ての Asset と PMDModel のインスタンスを Renderer クラスから物理削除し、
      Project クラスから論理削除 (remove*) を行う。Project が物理削除 (delete*) を行なってしまうと Renderer クラスで
      物理削除した時二重削除となってしまい不正なアクセスが発生するため、Project 側は論理削除だけにとどめておく必要がある。
     */
    m_renderer->releaseProject(m_project);
    delete m_delegate;
    m_delegate = 0;
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
}

void SceneLoader::setCameraMotion(vpvl::VMDMotion *motion)
{
    delete m_camera;
    m_camera = motion;
    m_renderer->scene()->setCameraMotion(motion);
}

void SceneLoader::setModelMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    model->addMotion(motion);
    m_project->addMotion(motion);
}

const QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> SceneLoader::stoppedMotions()
{
    /* 停止されたモーションを取得する (MMDAI2 上では使っていない) */
    QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> ret;
    /*
    QHashIterator<vpvl::PMDModel *, vpvl::VMDMotion *> i(m_motions);
    while (i.hasNext()) {
        i.next();
        vpvl::VMDMotion *motion = i.value();
        if (!motion->isActive() && motion->isReachedTo(motion->maxFrameIndex())) {
            vpvl::PMDModel *model = i.key();
            ret.insert(model, motion);
        }
    }
    */
    return ret;
}

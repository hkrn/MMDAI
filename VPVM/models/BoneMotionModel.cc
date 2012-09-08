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

#include "common/SceneLoader.h"
#include "common/SceneWidget.h"
#include "common/VPDFile.h"
#include "common/util.h"
#include "models/BoneMotionModel.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/qt/CString.h>

using namespace vpvl2;
using namespace vpvl2::qt;

namespace
{

class TreeItem : public MotionBaseModel::ITreeItem
{
public:
    TreeItem(const QString &name, IBone *bone, bool isRoot, bool isCategory, TreeItem *parent)
        : m_name(name),
          m_parent(parent),
          m_bone(bone),
          m_isRoot(isRoot),
          m_isCategory(isCategory)
    {
    }
    ~TreeItem() {
        qDeleteAll(m_children);
        m_parent = 0;
        m_bone = 0;
        m_isRoot = false;
        m_isCategory = false;
    }

    void addChild(ITreeItem *item) {
        m_children.append(static_cast<TreeItem *>(item));
    }
    ITreeItem *parent() const {
        return m_parent;
    }
    ITreeItem *child(int row) const {
        return m_children.value(row);
    }
    const QString &name() const {
        return m_name;
    }
    IBone *bone() const {
        return m_bone;
    }
    bool isRoot() const {
        return m_isRoot;
    }
    bool isCategory() const {
        return m_isCategory;
    }
    int rowIndex() const {
        /* 自分が親行に対して何番目にあるかを求める */
        return m_parent ? m_parent->m_children.indexOf(const_cast<TreeItem *>(this)) : 0;
    }
    int countChildren() const {
        return m_children.count();
    }

private:
    QList<TreeItem *> m_children;
    QString m_name;
    TreeItem *m_parent;
    IBone *m_bone;
    bool m_isRoot;
    bool m_isCategory;
};

typedef QPair<QModelIndex, QByteArray> ModelIndex;

class LoadPoseCommand : public QUndoCommand
{
public:
    /*
     * BoneMotionModel で selectedModel/currentMotion に変化があるとまずいのでポインタを保存しておく
     * モデルまたモーションを削除すると このコマンドのインスタンスを格納した UndoStack も一緒に削除される
     */
    LoadPoseCommand(BoneMotionModel *bmm, VPDFilePtr pose, int frameIndex)
        : QUndoCommand(),
          m_keys(bmm->keys()),
          m_bmm(bmm),
          m_model(bmm->selectedModel()),
          m_motion(bmm->currentMotion()),
          m_pose(0),
          m_frameIndex(frameIndex)
    {
        /* 現在のフレームにある全てのボーンのキーフレーム情報を参照する。キーフレームがあれば undo のために保存しておく */
        foreach (PMDMotionModel::ITreeItem *item, m_keys.values()) {
            const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, frameIndex);
            const QVariant &data = index.data(BoneMotionModel::kBinaryDataRole);
            if (data.canConvert(QVariant::ByteArray))
                m_modelIndices.append(ModelIndex(index, data.toByteArray()));
        }
        m_pose = pose.data()->clone();
        setText(QApplication::tr("Load a pose to %1").arg(frameIndex));
    }
    virtual ~LoadPoseCommand() {
        delete m_pose;
    }

    virtual void undo() {
        /* 現在のフレームを削除しておき、さらに全てのボーンのモデルのデータを空にしておく(=削除) */
        Array<IKeyframe *> keyframes;
        m_motion->getKeyframes(m_frameIndex, 0, IKeyframe::kBone, keyframes);
        const int nkeyframes = keyframes.count();
        for (int i = 0; i < nkeyframes; i++) {
            IKeyframe *keyframe = keyframes[i];
            m_motion->deleteKeyframe(keyframe);
        }
        foreach (PMDMotionModel::ITreeItem *item, m_keys.values()) {
            const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, m_frameIndex);
            m_bmm->setData(index, QVariant());
        }
        /* 削除後のインデックス更新を忘れなく行う */
        m_motion->update(IKeyframe::kBone);
        /*
         * コンストラクタで保存したボーン情報を復元して置換する。注意点として replaceKeyFrame でメモリの所有者が
         * BoneAnimation に移動するのでこちらで管理する必要がなくなる
         */
        QScopedPointer<IBoneKeyframe> boneKeyframe;
        Factory *factory = m_bmm->factory();
        foreach (const ModelIndex &index, m_modelIndices) {
            const QByteArray &bytes = index.second;
            m_bmm->setData(index.first, bytes, Qt::EditRole);
            boneKeyframe.reset(factory->createBoneKeyframe(m_motion));
            boneKeyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            m_motion->replaceKeyframe(boneKeyframe.take());
        }
        /*
         * replaceKeyframe (内部的には addKeyframe を呼んでいる) によって変更が必要になる
         * 内部インデックスの更新を行うため、update をかけておく
         */
        m_motion->update(IKeyframe::kBone);
        m_bmm->refreshModel(m_model);
    }
    virtual void redo() {
        QScopedPointer<IBoneKeyframe> newBoneKeyframe;
        Quaternion rotation;
        /* ポーズにあるボーン情報を参照する */
        Factory *factory = m_bmm->factory();
        foreach (VPDFile::Bone *bone, m_pose->bones()) {
            const QString &key = bone->name;
            /* ポーズにあるボーンがモデルの方に実在する */
            if (m_keys.contains(key)) {
                /*
                 * ポーズにあるボーン情報を元にキーフレームを作成し、モデルに登録した上で現在登録されているキーフレームを置換する
                 * replaceKeyFrame でメモリの所有者が BoneAnimation に移動する点は同じ
                 */
                const Vector4 &v = bone->rotation;
                const QModelIndex &modelIndex = m_bmm->frameIndexToModelIndex(m_keys[key], m_frameIndex);
                CString s(key);
                rotation.setValue(v.x(), v.y(), v.z(), v.w());
                newBoneKeyframe.reset(factory->createBoneKeyframe(m_motion));
                newBoneKeyframe->setDefaultInterpolationParameter();
                newBoneKeyframe->setName(&s);
                newBoneKeyframe->setPosition(bone->position);
                newBoneKeyframe->setRotation(rotation);
                newBoneKeyframe->setTimeIndex(m_frameIndex);
                QByteArray bytes(newBoneKeyframe->estimateSize(), '0');
                newBoneKeyframe->write(reinterpret_cast<uint8_t *>(bytes.data()));
                m_bmm->setData(modelIndex, bytes, Qt::EditRole);
                m_motion->replaceKeyframe(newBoneKeyframe.take());
            }
        }
        /* #undo のコメント通りのため、省略 */
        m_motion->update(IKeyframe::kBone);
        m_bmm->refreshModel(m_model);
    }

private:
    const BoneMotionModel::Keys m_keys;
    QList<ModelIndex> m_modelIndices;
    BoneMotionModel *m_bmm;
    IModel *m_model;
    IMotion *m_motion;
    VPDFile *m_pose;
    int m_frameIndex;
};

class SetKeyframesCommand : public QUndoCommand
{
public:
    /*
     * BoneMotionModel で selectedModel/currentMotion に変化があるとまずいのでポインタを保存しておく
     * モデルまたモーションを削除すると このコマンドのインスタンスを格納した UndoStack も一緒に削除される
     */
    SetKeyframesCommand(BoneMotionModel *bmm, const BoneMotionModel::KeyFramePairList &keyframes)
        : QUndoCommand(),
          m_keys(bmm->keys()),
          m_bmm(bmm),
          m_model(bmm->selectedModel()),
          m_motion(bmm->currentMotion()),
          m_bone(bmm->selectedBone())
    {
        QSet<int> indexProceeded;
        /* 現在選択中のモデルにある全てのボーンを取り出す */
        const BoneMotionModel::TreeItemList &items = m_keys.values();
        /* フレームインデックスがまたがるので複雑だが対象のキーフレームを全て保存しておく */
        foreach (const BoneMotionModel::KeyFramePair &frame, keyframes) {
            int frameIndex = frame.first;
            /* フレーム単位での重複を避けるためにスキップ処理を設ける */
            if (!indexProceeded.contains(frameIndex)) {
                /* モデルの全てのボーンを対象にデータがあるか確認し、存在している場合のみボーンのキーフレームの生データを保存する */
                foreach (PMDMotionModel::ITreeItem *item, items) {
                    const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, frameIndex);
                    const QVariant &data = index.data(BoneMotionModel::kBinaryDataRole);
                    if (data.canConvert(QVariant::ByteArray))
                        m_modelIndices.append(ModelIndex(index, data.toByteArray()));
                }
                indexProceeded.insert(frameIndex);
            }
        }
        m_keyframes = keyframes;
        m_frameIndices = indexProceeded.toList();
        setText(QApplication::tr("Register bone keyframes of %1").arg(internal::toQStringFromModel(m_model)));
    }
    virtual ~SetKeyframesCommand() {
    }

    virtual void undo() {
        /* 対象のキーフレームのインデックスを全て削除、さらにモデルのデータも削除 */
        const BoneMotionModel::TreeItemList &items = m_keys.values();
        Array<IKeyframe *> keyframes;
        foreach (int frameIndex, m_frameIndices) {
            keyframes.clear();
            m_motion->getKeyframes(frameIndex, 0, IKeyframe::kBone, keyframes);
            const int nkeyframes = keyframes.count();
            for (int i = 0; i < nkeyframes; i++) {
                IKeyframe *keyframe = keyframes[i];
                m_motion->deleteKeyframe(keyframe);
            }
            foreach (PMDMotionModel::ITreeItem *item, items) {
                const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, frameIndex);
                m_bmm->setData(index, QVariant());
            }
        }
        /* 削除後のインデックス更新を忘れなく行う */
        m_motion->update(IKeyframe::kBone);
        /* コンストラクタで保存したキーフレームの生データからボーンのキーフレームに復元して置換する */
        Factory *factory = m_bmm->factory();
        QScopedPointer<IBoneKeyframe> frame;
        foreach (const ModelIndex &index, m_modelIndices) {
            const QByteArray &bytes = index.second;
            m_bmm->setData(index.first, bytes, Qt::EditRole);
            frame.reset(factory->createBoneKeyframe(m_motion));
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            m_motion->replaceKeyframe(frame.take());
        }
        /* LoadPoseCommand#undo の通りのため、省略 */
        m_motion->update(IKeyframe::kBone);
        m_bmm->refreshModel(m_model);
    }
    virtual void redo() {
        QString key;
        QScopedPointer<IBoneKeyframe> newBoneKeyframe;
        /* すべてのキーフレーム情報を登録する */
        foreach (const BoneMotionModel::KeyFramePair &pair, m_keyframes) {
            int frameIndex = pair.first;
            const BoneMotionModel::KeyFramePtr &data = pair.second;
            IBoneKeyframe *boneKeyframe = data.data();
            /* キーフレームの対象ボーン名を取得する */
            if (boneKeyframe) {
                key = internal::toQStringFromBoneKeyframe(boneKeyframe);
            }
            else if (m_bone) {
                key = internal::toQStringFromBone(m_bone);
            }
            else {
                qWarning("No bone is selected or null");
                continue;
            }
            /* モデルにボーン名が存在するかを確認する */
            if (m_keys.contains(key)) {
                /*
                 * キーフレームをコピーし、モデルにデータを登録した上で現在登録されているキーフレームを置換する
                 * (前のキーフレームの情報が入ってる可能性があるので、置換することで重複が発生することを防ぐ)
                 *
                 * ※ 置換の現実装は find => delete => add なので find の探索コストがネックになるため多いと時間がかかる
                 */
                const QModelIndex &modelIndex = m_bmm->frameIndexToModelIndex(m_keys[key], frameIndex);
                if (boneKeyframe->timeIndex() >= 0) {
                    QByteArray bytes(boneKeyframe->estimateSize(), '0');
                    newBoneKeyframe.reset(boneKeyframe->clone());
                    newBoneKeyframe->setTimeIndex(frameIndex);
                    newBoneKeyframe->write(reinterpret_cast<uint8_t *>(bytes.data()));
                    m_motion->replaceKeyframe(newBoneKeyframe.take());
                    m_bmm->setData(modelIndex, bytes);
                }
                else {
                    /* 元フレームのインデックスが 0 未満の時は削除 */
                    IKeyframe *frameToDelete = m_motion->findBoneKeyframe(frameIndex, boneKeyframe->name(), 0);
                    m_motion->deleteKeyframe(frameToDelete);
                    m_bmm->setData(modelIndex, QVariant());
                }
            }
            else {
                qWarning("Tried registering not bone key frame: %s", qPrintable(key));
                continue;
            }
        }
        /* LoadPoseCommand#undo の通りのため、省略 */
        m_motion->update(IKeyframe::kBone);
        m_bmm->refreshModel(m_model);
    }

private:
    const BoneMotionModel::Keys m_keys;
    /* undo で復元する対象のキーフレームの番号 */
    QList<int> m_frameIndices;
    /* m_frameIndices に加えて undo で復元する用のキーフレームの集合 */
    QList<ModelIndex> m_modelIndices;
    /* 実際に登録する用のキーフレームの集合 */
    BoneMotionModel::KeyFramePairList m_keyframes;
    BoneMotionModel *m_bmm;
    IModel *m_model;
    IMotion *m_motion;
    IBone *m_bone;
};

class ResetAllCommand : public QUndoCommand
{
public:
    ResetAllCommand(const vpvl2::Scene *scene, IModel *model)
        : QUndoCommand(),
          m_state(scene, model)
    {
        /* 全てのボーンの情報を保存しておく */
        m_state.save();
        setText(QApplication::tr("Reset all bones of %1").arg(internal::toQStringFromModel(model)));
    }
    virtual ~ResetAllCommand() {
    }

    void undo() {
        /* コンストラクタで保存したボーン情報を復元し、シークせずにモデルを更新しておく */
        m_state.restore();
    }
    void redo() {
        /* 全てのボーンをリセットし、シークせずにモデルを更新しておく */
        m_state.resetBones();
    }

private:
    PMDMotionModel::State m_state;
};

class SetBoneCommand : public QUndoCommand
{
public:
    SetBoneCommand(const vpvl2::Scene *scene, IModel *model, const PMDMotionModel::State &state)
        : QUndoCommand(),
          m_oldState(scene, 0),
          m_newState(scene, model)
    {
        m_oldState.copyFrom(state);
        /* 前と後の全てのボーンの情報を保存しておく */
        m_newState.save();
        setText(QApplication::tr("Set bones of %1").arg(internal::toQStringFromModel(model)));
    }

    void undo() {
        /* コンストラクタで呼ばれる前のボーン情報を復元し、シークせずにモデルを更新しておく */
        m_oldState.restore();
    }
    void redo() {
        /* コンストラクタで呼ばれた時点のボーン情報を復元し、シークせずにモデルを更新しておく */
        m_newState.restore();
    }

private:
    PMDMotionModel::State m_oldState;
    PMDMotionModel::State m_newState;
};

static const Quaternion UIRotateGlobalAxisAngle(const IBone *bone, const Scalar &value, int flags)
{
    Quaternion rot = Quaternion::getIdentity();
    /* 固定軸の場合は固定軸をそのまま使って変形させる */
    if (bone->hasFixedAxes()) {
        rot.setRotation(bone->fixedAxis(), radian(value));
    }
    else {
        /*
         * 0x0000ff00 <= ff の部分に X/Y/Z のいずれかの軸のフラグが入ってる
         * また、座標系の関係でX軸とY軸は値を反転させる
         */
        switch ((flags & 0xff00) >> 8) {
        case 'X':
            rot.setRotation(Vector3(1, 0, 0), -radian(value));
            break;
        case 'Y':
            rot.setRotation(Vector3(0, 1, 0), -radian(value));
            break;
        case 'Z':
            rot.setRotation(Vector3(0, 0, 1), radian(value));
            break;
        }
    }
    return rot;
}

static const Vector3 UITranslateFromView(const Transform &transform, const Vector3 &delta)
{
    const Matrix3x3 &matrix = transform.getBasis();
    Vector3 value = kZeroV3;
    value += matrix[0] * delta.x();
    value += matrix[1] * delta.y();
    value += matrix[2] * delta.z();
    return value;
}

static const Quaternion UIRotateViewAxisAngle(const IBone *bone, const Scalar &value, int flags, const Transform &transform)
{
    Quaternion rot = Quaternion::getIdentity();
    /* 固定軸の場合は固定軸をそのまま使って変形させる */
    if (bone->hasFixedAxes()) {
        rot.setRotation(bone->fixedAxis(), radian(value));
    }
    else {
        const Matrix3x3 &matrix = transform.getBasis();
        /*
         * 0x0000ff00 <= ff の部分に X/Y/Z のいずれかの軸のフラグが入ってる
         * また、座標系の関係でX軸とY軸は値を反転させる
         */
        switch ((flags & 0xff00) >> 8) {
        case 'X':
            rot.setRotation(matrix.getRow(0), -radian(value));
            break;
        case 'Y':
            rot.setRotation(matrix.getRow(1), -radian(value));
            break;
        case 'Z':
            rot.setRotation(matrix.getRow(2), radian(value));
            break;
        }
    }
    return rot;
}

static const Quaternion UIRotateLocalAxisAngle(const IBone *bone, const Scalar &value, int flags, const Quaternion &rotation)
{
    Quaternion rot = Quaternion::getIdentity();
    /* 固定軸の場合は固定軸をそのまま使って変形させる */
    if (bone->hasFixedAxes()) {
        rot.setRotation(bone->fixedAxis(), radian(value));
    }
    else {
        Vector3 axisX(1, 0, 0), axisY(0, 1, 0), axisZ(0, 0, 1);
        /* ボーンにローカル軸を持っているか？ */
        if (bone->hasLocalAxes()) {
            Matrix3x3 axes = Matrix3x3::getIdentity();
            bone->getLocalAxes(axes);
            axisX = axes[0];
            axisY = axes[1];
            axisZ = axes[2];
        }
        else {
            Matrix3x3 matrix(rotation);
            axisX = matrix[0] * axisX;
            axisY = matrix[1] * axisY;
            axisZ = matrix[2] * axisZ;
        }
        /*
         * 0x0000ff00 <= ff の部分に X/Y/Z のいずれかの軸のフラグが入ってる
         * また、座標系の関係でX軸とY軸は値を反転させる
         */
        switch ((flags & 0xff00) >> 8) {
        case 'X':
            rot.setRotation(axisX, -radian(value));
            break;
        case 'Y':
            rot.setRotation(axisY, -radian(value));
            break;
        case 'Z':
            rot.setRotation(axisZ, radian(value));
            break;
        }
    }
    return rot;
}

}

BoneMotionModel::BoneMotionModel(Factory *factory,
                                 QUndoGroup *undo,
                                 QObject *parent) :
    PMDMotionModel(undo, parent),
    m_state(0, 0),
    m_factory(factory),
    m_viewTransform(Transform::getIdentity())
{
}

BoneMotionModel::~BoneMotionModel()
{
}

void BoneMotionModel::saveMotion(IMotion *motion)
{
    if (m_model) {
        /* モデルの ByteArray を BoneKeyFrame に読ませて積んでおくだけの簡単な処理 */
        QScopedPointer<IBoneKeyframe> newBoneKeyframe;
        foreach (const QVariant &value, values()) {
            const QByteArray &bytes = value.toByteArray();
            newBoneKeyframe.reset(m_factory->createBoneKeyframe(motion));
            newBoneKeyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            motion->addKeyframe(newBoneKeyframe.take());
        }
        motion->update(IKeyframe::kBone);
        setModified(false);
    }
    else {
        qWarning("No model is selected to save motion.");
    }
}

void BoneMotionModel::addKeyframesByModelIndices(const QModelIndexList &indices)
{
    KeyFramePairList boneKeyframes;
    IModel *model = selectedModel();
    /* モデルのインデックスを参照し、存在するボーンに対してボーンの現在の値からボーンのキーフレームにコピーする */
    foreach (const QModelIndex &index, indices) {
        int frameIndex = toTimeIndex(index);
        if (frameIndex >= 0) {
            const QString &name = nameFromModelIndex(index);
            CString s(name);
            IBone *bone = model->findBone(&s);
            if (bone) {
                /* 補間パラメータは SetFramesCommand の中で設定されるため、初期化のみ */
                KeyFramePtr newKeyframe(m_factory->createBoneKeyframe(m_motion));
                newKeyframe->setDefaultInterpolationParameter();
                newKeyframe->setName(bone->name());
                newKeyframe->setPosition(bone->position());
                newKeyframe->setRotation(bone->rotation());
                boneKeyframes.append(KeyFramePair(frameIndex, newKeyframe));
            }
        }
    }
    setKeyframes(boneKeyframes);
}

void BoneMotionModel::copyKeyframesByModelIndices(const QModelIndexList &indices, int frameIndex)
{
    if (m_model && m_motion) {
        /* 前回呼ばれた copyFrames で作成したデータを破棄しておく */
        m_copiedKeyframes.clear();
        /* モデル内のすべてのボーン名を参照し、データがあるものだけを BoneKeyFrame に移しておく */
        foreach (const QModelIndex &index, indices) {
            if (index.isValid()) {
                const QVariant &variant = index.data(kBinaryDataRole);
                if (variant.canConvert(QVariant::ByteArray)) {
                    const QByteArray &bytes = variant.toByteArray();
                    KeyFramePtr newKeyframe(m_factory->createBoneKeyframe(m_motion));
                    newKeyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
                    /* 予め差分をとっておき、pasteKeyframes でペースト先の差分をたすようにする */
                    int diff = newKeyframe->timeIndex() - frameIndex;
                    m_copiedKeyframes.append(KeyFramePair(diff, newKeyframe));
                }
            }
        }
    }
}

void BoneMotionModel::pasteKeyframesByTimeIndex(int frameIndex)
{
    /* m_frames が #copyFrames でコピーされていること前提 */
    if (m_model && m_motion && !m_copiedKeyframes.isEmpty()) {
        /*
         * m_frames のデータを引数のフレームインデックスと一緒に積ませて SetFramesCommand として作成する
         * m_frames のデータは破棄されないので、#copyFrames で破棄するようになってる
         */
        KeyFramePairList keyframes;
        foreach (const KeyFramePair &pair, m_copiedKeyframes) {
            KeyFramePtr keyframe(pair.second->clone());
            /* コピー先にフレームインデックスを更新する */
            int newFrameIndex = frameIndex + pair.first;
            keyframes.append(KeyFramePair(newFrameIndex, keyframe));
        }
        addUndoCommand(new SetKeyframesCommand(this, keyframes));
    }
}

void BoneMotionModel::pasteReversedFrame(int frameIndex)
{
    const QString &right = "右";
    const QString &left = "左";
    QSet<QString> registered;
    /* m_frames が #copyFrames でコピーされていること前提 */
    if (m_model && m_motion && !m_copiedKeyframes.isEmpty()) {
        KeyFramePairList keyframes;
        KeyFramePtr newKeyframe;
        QScopedPointer<IBoneKeyframe> keyframe;
        /* 基本的な処理は pasteFrame と同等だが、「左」「右」の名前を持つボーンは特別扱い */
        foreach (const KeyFramePair &pair, m_copiedKeyframes) {
            keyframe.reset(pair.second->clone());
            const QString &name = internal::toQStringFromBoneKeyframe(keyframe.data());
            /* 二重登録防止のため、「左」「右」はどちらか出てきたら処理は一回のみ */
            if (!registered.contains(name)) {
                bool isRight = name.startsWith(right);
                bool isLeft = name.startsWith(left);
                /* 最初の名前に「左」「右」が入っている場合、置換した上で位置と回転の値を左右反転させる */
                if (isRight || isLeft) {
                    QString key = name;
                    if (isRight)
                        key.replace(right, left);
                    else if (isLeft)
                        key.replace(left, right);
                    CString s(key);
                    newKeyframe = KeyFramePtr(keyframe->clone());
                    newKeyframe->setName(&s);
                    Vector3 position = newKeyframe->position();
                    position.setValue(-position.x(), position.y(), position.z());
                    newKeyframe->setPosition(position);
                    Quaternion rotation = newKeyframe->rotation();
                    rotation.setValue(rotation.x(), -rotation.y(), -rotation.z(), rotation.w());
                    newKeyframe->setRotation(rotation);
                    registered.insert(key);
                }
            }
            else {
                newKeyframe = KeyFramePtr(pair.second->clone());
            }
            /* コピー先にフレームインデックスを更新する */
            int newFrameIndex = frameIndex + pair.first;
            keyframes.append(KeyFramePair(newFrameIndex, newKeyframe));
        }
        addUndoCommand(new SetKeyframesCommand(this, keyframes));
    }
}

void BoneMotionModel::saveTransform()
{
    if (m_model) {
        /*
         * モデルの状態を保存しておく。メモリリーク防止のため、前の状態は破棄しておく
         */
        m_state.setModel(m_model);
        m_state.save();
        Array<IBone *> bones;
        m_model->getBones(bones);
        int nbones = bones.count();
        for (int i = 0; i < nbones; i++) {
            IBone *bone = bones[i];
            m_boneTransformStates.insert(bone, QPair<Vector3, Quaternion>(bone->position(), bone->rotation()));
        }
    }
}

void BoneMotionModel::commitTransform()
{
    if (m_model) {
        /* 状態を圧縮し、ボーン変形があれば SetBoneCommand を作成して UndoStack に追加 */
        if (m_state.compact())
            addUndoCommand(new SetBoneCommand(m_scene, m_model, m_state));
        m_boneTransformStates.clear();
    }
}

const QString BoneMotionModel::nameFromModelIndex(const QModelIndex &index) const
{
    return static_cast<TreeItem *>(index.internalPointer())->name();
}

const QModelIndexList BoneMotionModel::modelIndicesFromBones(const QList<IBone *> &bones, int frameIndex) const
{
    const QSet<IBone *> &boneSet = bones.toSet();
    QModelIndexList indices;
    foreach (PMDMotionModel::ITreeItem *item, keys().values()) {
        TreeItem *treeItem = static_cast<TreeItem *>(item);
        if (boneSet.contains(treeItem->bone())) {
            const QModelIndex &index = frameIndexToModelIndex(item, frameIndex);
            indices.append(index);
        }
    }
    return indices;
}

BoneMotionModel::KeyFramePairList BoneMotionModel::keyframesFromModelIndices(const QModelIndexList &indices) const
{
    KeyFramePairList keyframes;
    foreach (const QModelIndex &index, indices) {
        int frameIndex = toTimeIndex(index);
        TreeItem *treeItem = static_cast<TreeItem *>(index.internalPointer());
        if (treeItem->isRoot()) {
            continue;
        }
        else if (treeItem->isCategory()) {
            int nchildren = treeItem->countChildren();
            for (int i = 0; i < nchildren; i++) {
                ITreeItem *childTreeItem = treeItem->child(i);
                const QModelIndex childIndex = frameIndexToModelIndex(childTreeItem, frameIndex);
                const QVariant &variant = childIndex.data(BoneMotionModel::kBinaryDataRole);
                if (variant.canConvert(QVariant::ByteArray)) {
                    KeyFramePtr keyframe(m_factory->createBoneKeyframe(m_motion));
                    keyframe->read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
                    keyframes.append(KeyFramePair(frameIndex, keyframe));
                }
            }
        }
        else {
            const QVariant &variant = index.data(BoneMotionModel::kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                KeyFramePtr keyframe(m_factory->createBoneKeyframe(m_motion));
                keyframe->read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
                keyframes.append(KeyFramePair(frameIndex, keyframe));
            }
        }
    }
    return keyframes;
}

void BoneMotionModel::loadPose(VPDFilePtr pose, IModel *model, int frameIndex)
{
    if (model == m_model && m_motion) {
        addUndoCommand(new LoadPoseCommand(this, pose, frameIndex));
        qDebug("Loaded a pose to the model: %s", qPrintable(internal::toQStringFromModel(model)));
    }
    else {
        qWarning("Tried loading pose to invalid model or without motion: %s", qPrintable(internal::toQStringFromModel(model)));
    }
}

void BoneMotionModel::savePose(VPDFile *pose, IModel *model, int frameIndex)
{
    if (model == m_model) {
        VPDFile::BoneList bones;
        QScopedPointer<IBoneKeyframe> keyframe;
        /* モデルにある全てのボーンを参照し、現在のキーフレームでデータが入ってるものを VPDFile::Bone に変換する */
        foreach (ITreeItem *item, keys()) {
            const QModelIndex &modelIndex = frameIndexToModelIndex(item, frameIndex);
            const QVariant &variant = modelIndex.data(BoneMotionModel::kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                VPDFile::Bone *bone = new VPDFile::Bone();
                keyframe.reset(m_factory->createBoneKeyframe(m_motion));
                keyframe->read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
                const Quaternion &q = keyframe->rotation();
                bone->name = internal::toQStringFromBoneKeyframe(keyframe.data());
                bone->position = keyframe->position();
                bone->rotation = Vector4(q.x(), q.y(), q.z(), q.w());
                bones.append(bone);
            }
        }
        pose->setBones(bones);
    }
    else {
        qWarning("Tried loading pose to invalid model or without motion: %s", qPrintable(internal::toQStringFromModel(model)));
    }
}

void BoneMotionModel::setKeyframes(const KeyFramePairList &keyframes)
{
    if (m_model && m_motion)
        addUndoCommand(new SetKeyframesCommand(this, keyframes));
    else
        qWarning("No model or motion to register bone frames.");
}

void BoneMotionModel::setPMDModel(IModel *model)
{
    /* 引数のモデルが現在選択中のものであれば二重処理になってしまうので、スキップする */
    if (m_model == model)
        return;
    if (model) {
        /* PMD の二重登録防止 */
        if (!hasPMDModel(model)) {
            /* ルートを作成 */
            RootPtr ptr(new TreeItem("", 0, true, false, 0));
            TreeItem *r = static_cast<TreeItem *>(ptr.data());
            Array<ILabel *> labels;
            Keys keys;
            model->getLabels(labels);
            /* ボーンのカテゴリからルートの子供であるカテゴリアイテムを作成する */
            const int nlabels = labels.count();
            QScopedPointer<TreeItem> parent, child;
            for (int i = 0; i < nlabels; i++) {
                const ILabel *label = labels[i];
                const int nchildren = label->count();
                if (label->isSpecial()) {
                    /* 特殊枠でかつ先頭ボーンかどうか */
                    static const CString kRoot("Root");
                    if (nchildren > 0 && label->name()->equals(&kRoot)) {
                        const IBone *bone = label->bone(0);
                        if (bone) {
                            /* カテゴリ名は trimmed を呼ばないと PMD で表示上余計な空白が生じる */
                            const QString &category = internal::toQStringFromBone(bone).trimmed();
                            parent.reset(new TreeItem(category, 0, false, true, r));
                        }
                    }
                    /* 表情ラベルはスキップする */
                    if (parent.isNull())
                        continue;
                }
                else {
                    const QString &category = internal::toQStringFromString(label->name()).trimmed();
                    parent.reset(new TreeItem(category, 0, false, true, r));
                }
                /* カテゴリに属するボーン名を求めてカテゴリアイテムに追加する。また、ボーン名をキー名として追加 */
                for (int j = 0; j < nchildren; j++) {
                    IBone *bone = label->bone(j);
                    if (bone) {
                        const QString &name = internal::toQStringFromBone(bone);
                        child.reset(new TreeItem(name, bone, false, false, parent.data()));
                        parent->addChild(child.data());
                        keys.insert(name, child.take());
                    }
                }
                /* カテゴリアイテムをルートアイテムに追加 */
                r->addChild(parent.take());
            }
            addPMDModel(model, ptr, keys);
        }
        else {
            /* キーリストが空でもモデルが存在し、スキップされるので実害なし */
            addPMDModel(model, rootPtr(model), Keys());
        }
        m_model = model;
        emit modelDidChange(model);
        /* ボーン選択(最初のボーンを選択状態にする) */
        Array<IBone *> bones;
        model->getBones(bones);
        if (bones.count() > 0) {
            QList<IBone *> selectedBones;
            selectedBones.append(bones[0]);
            selectBones(selectedBones);
        }
        qDebug("Set a model in BoneMotionModel: %s", qPrintable(internal::toQStringFromModel(model)));
    }
    else {
        m_model = 0;
        selectBones(QList<IBone *>());
        emit modelDidChange(0);
    }
    /* テーブルモデルを更新 */
    reset();
}

void BoneMotionModel::loadMotion(IMotion *motion, const IModel *model)
{
    /* 現在のモデルが対象のモデルと一致していることを確認しておく */
    if (model == m_model) {
        const int nkeyframes = motion->countKeyframes(IKeyframe::kBone);
        const Keys &keys = this->keys();
        QScopedPointer<IBoneKeyframe> newKeyframe;
        /* フレーム列の最大数をモーションのフレーム数に更新する */
        setFrameIndexColumnMax(motion);
        reset();
        /* モーションのすべてのキーフレームを参照し、モデルのボーン名に存在するものだけ登録する */
        for (int i = 0; i < nkeyframes; i++) {
            const IBoneKeyframe *keyframe = motion->findBoneKeyframeAt(i);
            const QString &key = internal::toQStringFromBoneKeyframe(keyframe);
            if (keys.contains(key)) {
                int frameIndex = static_cast<int>(keyframe->timeIndex());
                QByteArray bytes(keyframe->estimateSize(), '0');
                ITreeItem *item = keys[key];
                /* この時点で新しい QModelIndex が作成される */
                const QModelIndex &modelIndex = frameIndexToModelIndex(item, frameIndex);
                newKeyframe.reset(m_factory->createBoneKeyframe(motion));
                newKeyframe->setName(keyframe->name());
                newKeyframe->setPosition(keyframe->position());
                newKeyframe->setRotation(keyframe->rotation());
                newKeyframe->setTimeIndex(frameIndex);
                QuadWord v;
                for (int i = 0; i < IBoneKeyframe::kMaxInterpolationType; i++) {
                    IBoneKeyframe::InterpolationType type = static_cast<IBoneKeyframe::InterpolationType>(i);
                    keyframe->getInterpolationParameter(type, v);
                    newKeyframe->setInterpolationParameter(type, v);
                }
                newKeyframe->write(reinterpret_cast<uint8_t *>(bytes.data()));
                /* キーフレームのバイナリデータが QModelIndex の QVariant に登録される。この方が管理が楽になる */
                setData(modelIndex, bytes);
            }
        }
        /* 読み込まれたモーションを現在のモーションとして登録する。あとは LoadCommand#undo と同じ */
        m_motion = motion;
        refreshModel(m_model);
        setModified(false);
        qDebug("Loaded a motion to the model in BoneMotionModel: %s", qPrintable(internal::toQStringFromModel(model)));
    }
    else {
        qDebug("Tried loading a motion to different model, ignored: %s", qPrintable(internal::toQStringFromModel(model)));
    }
}

void BoneMotionModel::removeMotion()
{
    /* コピーしたキーフレーム、選択されたボーンとモデルに登録されているデータが削除される。ボーン名は削除されない */
    m_copiedKeyframes.clear();
    m_selectedBones.clear();
    m_motion = 0;
    setModified(false);
    removePMDMotion(m_model);
    reset();
}

void BoneMotionModel::removeModel()
{
    /*
     * モーション削除に加えて PMD を論理削除する。巻き戻し情報も削除されるため巻戻しが不可になる
     * PMD は SceneLoader で管理されるため、PMD のメモリの解放はしない
     */
    removeMotion();
    removePMDModel(m_model);
    reset();
    m_boneTransformStates.clear();
    emit modelDidChange(0);
}

void BoneMotionModel::deleteKeyframesByModelIndices(const QModelIndexList &indices)
{
    KeyFramePairList keyframes;
    /* ここでは削除するキーフレームを決定するのみ。実際に削除するのは SetFramesCommand である点に注意 */
    foreach (const QModelIndex &index, indices) {
        if (index.isValid() && index.column() > 1) {
            TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
            if (IBone *bone = item->bone()) {
                IBoneKeyframe *keyframeToDelete = m_motion->findBoneKeyframe(toTimeIndex(index), bone->name(), 0);
                if (keyframeToDelete) {
                    KeyFramePtr clonedKeyframe(keyframeToDelete->clone());
                    /* SetFramesCommand で削除するので削除に必要な条件である frameIndex を 0 未満の値にしておく */
                    clonedKeyframe->setTimeIndex(-1);
                    keyframes.append(KeyFramePair(keyframeToDelete->timeIndex(), clonedKeyframe));
                }
            }
        }
    }
    addUndoCommand(new SetKeyframesCommand(this, keyframes));
}

void BoneMotionModel::applyKeyframeWeightByModelIndices(const QModelIndexList &indices,
                                                        const Vector3 &position,
                                                        const Vector3 &rotation)
{
    KeyFramePairList keyframes;
    Quaternion newRotation;
    foreach (const QModelIndex &index, indices) {
        if (index.isValid()) {
            /* QModelIndex からキーフレームを取得し、その中に入っている値を補正する */
            const QVariant &variant = index.data(kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                const QByteArray &bytes = variant.toByteArray();
                KeyFramePtr keyframe(m_factory->createBoneKeyframe(m_motion));
                keyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
                const Quaternion &oldRotation = keyframe->rotation();
                newRotation.setX(oldRotation.x() * rotation.x());
                newRotation.setY(oldRotation.y() * rotation.y());
                newRotation.setZ(oldRotation.z() * rotation.z());
                newRotation.setW(oldRotation.w());
                if (newRotation.x() > 1 || newRotation.y() > 1 || newRotation.z() > 1)
                    newRotation.normalize();
                keyframe->setPosition(keyframe->position() * position);
                keyframe->setRotation(newRotation);
                keyframes.append(KeyFramePair(toTimeIndex(index), keyframe));
            }
        }
    }
    setKeyframes(keyframes);
}

void BoneMotionModel::selectBonesByModelIndices(const QModelIndexList &indices)
{
    QList<IBone *> bones;
    foreach (const QModelIndex &index, indices) {
        if (index.isValid()) {
            TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
            if (item->isCategory()) {
                int nbones = item->countChildren();
                for (int i = 0; i < nbones; i++) {
                    TreeItem *child = static_cast<TreeItem *>(item->child(i));
                    IBone *bone = child->bone();
                    bones.append(bone);
                }
            }
            else if (IBone *bone = item->bone()) {
                bones.append(bone);
            }
        }
    }
    m_selectedBones = bones;
    emit bonesDidSelect(bones);
}

void BoneMotionModel::resetBone(ResetType type)
{
    foreach (IBone *selected, m_selectedBones) {
        Vector3 pos = selected->position();
        Quaternion rot = selected->rotation();
        switch (type) {
        case kX:
            pos.setX(0.0f);
            selected->setPosition(pos);
            break;
        case kY:
            pos.setY(0.0f);
            selected->setPosition(pos);
            break;
        case kZ:
            pos.setZ(0.0f);
            selected->setPosition(pos);
            break;
        case kRotation:
            rot.setValue(0.0f, 0.0f, 0.0f, 1.0f);
            selected->setRotation(rot);
            break;
        default:
            qFatal("Unexpected reset bone type: %d", type);
        }
    }
    updateModel(m_model, true);
}

void BoneMotionModel::resetAllBones()
{
    if (m_model)
        addUndoCommand(new ResetAllCommand(m_scene, m_model));
}

void BoneMotionModel::setPosition(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    foreach (IBone *selected, m_selectedBones) {
        const Vector3 &lastPosition = selected->position();
        Vector3 position = lastPosition;
        switch (coordinate) {
        case 'x':
        case 'X':
            position.setX(value);
            break;
        case 'y':
        case 'Y':
            position.setY(value);
            break;
        case 'z':
        case 'Z':
            position.setZ(value);
            break;
        default:
            qFatal("Unexpected coordinate value: %c", coordinate);
        }
        selected->setPosition(position);
        m_scene->updateModel(m_model);
        emit positionDidChange(selected, lastPosition);
    }
}

void BoneMotionModel::setRotation(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    IBone *selected = m_selectedBones.last();
    const Quaternion &lastRotation = selected->rotation();
    Quaternion rotation = lastRotation;
    switch (coordinate) {
    case 'x':
    case 'X':
        rotation.setX(value);
        break;
    case 'y':
    case 'Y':
        rotation.setY(value);
        break;
    case 'z':
    case 'Z':
        rotation.setZ(value);
        break;
    default:
        qFatal("Unexpected coordinate value: %c", coordinate);
    }
    selected->setRotation(rotation);
    m_scene->updateModel(m_model);
    emit rotationDidChange(selected, lastRotation);
}

void BoneMotionModel::translateDelta(const Vector3 &delta, IBone *bone, int flags)
{
    /* ボーン指定がない場合は BoneMotionModel が持ってる選択状態のボーンにする */
    if (!bone) {
        if (isBoneSelected())
            bone = selectedBone();
        else
            return;
    }
    /* 差分値による更新 */
    translateInternal(bone->position(), delta, bone, flags);
}

void BoneMotionModel::translateTo(const Vector3 &position, IBone *bone, int flags)
{
    /* ボーン指定がない場合は BoneMotionModel が持ってる選択状態のボーンにする */
    if (!bone) {
        if (isBoneSelected())
            bone = selectedBone();
        else
            return;
    }
    /* 絶対値による更新 */
    Vector3 lastPosition = kZeroV3;
    if (m_boneTransformStates.contains(bone))
        lastPosition = m_boneTransformStates[bone].first;
    translateInternal(lastPosition, position, bone, flags);
}

void BoneMotionModel::rotateAngle(const Scalar &value, IBone *bone, int flags)
{
    if (!bone) {
        if (isBoneSelected())
            bone = selectedBone();
        else
            return;
    }
    Quaternion lastRotation = Quaternion::getIdentity();
    if (m_boneTransformStates.contains(bone))
        lastRotation = m_boneTransformStates[bone].second;
    /* 固定軸がある場合は変形方法に関係なく常に固定軸を使って変形されます */
    switch (flags & 0xff) {
    case 'V': { /* ビュー変形 (カメラ視点) */
        bone->setRotation(lastRotation * UIRotateViewAxisAngle(bone, value, flags, m_viewTransform));
        break;
    }
    case 'L': /* ローカル変形 */
        bone->setRotation(lastRotation * UIRotateLocalAxisAngle(bone, value, flags, lastRotation));
        break;
    case 'G': /* グローバル変形 */
        bone->setRotation(lastRotation * UIRotateGlobalAxisAngle(bone, value, flags));
        break;
    default:
        qFatal("Unexpected mode: %c", flags & 0xff);
        break;
    }
    m_scene->updateModel(m_model);
    emit rotationDidChange(bone, lastRotation);
}

void BoneMotionModel::selectBones(const QList<IBone *> &bones)
{
    /* signal/slot による循環参照防止 */
    if (bones != m_selectedBones) {
        m_selectedBones = bones;
        emit bonesDidSelect(bones);
    }
}

void BoneMotionModel::setCamera(const ICamera *camera)
{
    m_viewTransform = camera->modelViewTransform();
}

void BoneMotionModel::translateInternal(const Vector3 &position, const Vector3 &delta, IBone *bone, int flags)
{
    switch (flags & 0xff) {
    case 'V': /* ビュー変形 (カメラ視点) */
        bone->setPosition(Transform(bone->rotation(), position) * UITranslateFromView(m_viewTransform, delta));
        break;
    case 'L': /* ローカル変形 */
        bone->setPosition(Transform(bone->rotation(), position) * delta);
        break;
    case 'G': /* グローバル変形 */
        bone->setPosition(position + delta);
        break;
    default:
        qFatal("Unexpected mode: %c", flags & 0xff);
        break;
    }
    m_scene->updateModel(m_model);
    emit positionDidChange(bone, position);
}

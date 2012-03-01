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

#include <vpvl/vpvl.h>
#include <vpvl/gl2/Renderer.h>

using namespace vpvl;

namespace
{

class TreeItem : public MotionBaseModel::ITreeItem
{
public:
    TreeItem(const QString &name, Bone *bone, bool isRoot, bool isCategory, TreeItem *parent)
        : m_name(name),
          m_parent(parent),
          m_bone(bone),
          m_isRoot(isRoot),
          m_isCategory(isCategory)
    {
    }
    ~TreeItem() {
        qDeleteAll(m_children);
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
    Bone *bone() const {
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
    Bone *m_bone;
    bool m_isRoot;
    bool m_isCategory;
};

typedef QPair<QModelIndex, QByteArray> ModelIndex;

class LoadPoseCommand : public QUndoCommand
{
public:
    LoadPoseCommand(BoneMotionModel *bmm, VPDFile *pose, int frameIndex)
        : QUndoCommand(),
          m_bmm(bmm),
          m_pose(0),
          m_frameIndex(frameIndex)
    {
        /* 現在のフレームにある全てのボーンのキーフレーム情報を参照する。キーフレームがあれば undo のために保存しておく */
        foreach (PMDMotionModel::ITreeItem *item, m_bmm->keys().values()) {
            const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, frameIndex);
            const QVariant &data = index.data(BoneMotionModel::kBinaryDataRole);
            if (data.canConvert(QVariant::ByteArray))
                m_modelIndices.append(ModelIndex(index, data.toByteArray()));
        }
        m_pose = pose->clone();
    }
    virtual ~LoadPoseCommand() {
        delete m_pose;
    }

    virtual void undo() {
        BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        /* 現在のフレームを削除しておき、さらに全てのボーンのモデルのデータを空にしておく(=削除) */
        animation->deleteKeyframes(m_frameIndex);
        foreach (PMDMotionModel::ITreeItem *item, m_bmm->keys().values()) {
            const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, m_frameIndex);
            m_bmm->setData(index, QVariant());
        }
        /*
         * コンストラクタで保存したボーン情報を復元して置換する。注意点として replaceKeyFrame でメモリの所有者が
         * BoneAnimation に移動するのでこちらで管理する必要がなくなる
         */
        foreach (const ModelIndex &index, m_modelIndices) {
            const QByteArray &bytes = index.second;
            m_bmm->setData(index.first, bytes, Qt::EditRole);
            BoneKeyframe *frame = new BoneKeyframe();
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->replaceKeyframe(frame);
        }
        /*
         * BoneAnimation の内部データの更新も忘れずに。モデルをリセットした上で、
         * モーションを更新するために PMD を現在のフレームに強制シークしておく
         */
        animation->refresh();
        m_bmm->refreshModel();
    }
    virtual void redo() {
        QTextCodec *codec = internal::getTextCodec();
        BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        const BoneMotionModel::Keys &bones = m_bmm->keys();
        Quaternion rotation;
        /* ポーズにあるボーン情報を参照する */
        foreach (VPDFile::Bone *bone, m_pose->bones()) {
            const QString &key = bone->name;
            /* ポーズにあるボーンがモデルの方に実在する */
            if (bones.contains(key)) {
                /*
                 * ポーズにあるボーン情報を元にキーフレームを作成し、モデルに登録した上で現在登録されているキーフレームを置換する
                 * replaceKeyFrame でメモリの所有者が BoneAnimation に移動する点は同じ
                 */
                const Vector4 &v = bone->rotation;
                const QModelIndex &modelIndex = m_bmm->frameIndexToModelIndex(bones[key], m_frameIndex);
                rotation.setValue(v.x(), v.y(), v.z(), v.w());
                BoneKeyframe *newFrame = new BoneKeyframe();
                newFrame->setDefaultInterpolationParameter();
                newFrame->setName(reinterpret_cast<const uint8_t *>(codec->fromUnicode(key).constData()));
                newFrame->setPosition(bone->position);
                newFrame->setRotation(rotation);
                newFrame->setFrameIndex(m_frameIndex);
                QByteArray bytes(BoneKeyframe::strideSize(), '0');
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                m_bmm->setData(modelIndex, bytes, Qt::EditRole);
                animation->replaceKeyframe(newFrame);
            }
        }
        /* #undo のコメント通りのため、省略 */
        animation->refresh();
        m_bmm->refreshModel();
    }

private:
    QList<ModelIndex> m_modelIndices;
    BoneMotionModel *m_bmm;
    VPDFile *m_pose;
    int m_frameIndex;
};

class SetFramesCommand : public QUndoCommand
{
public:
    SetFramesCommand(BoneMotionModel *bmm, const BoneMotionModel::KeyFramePairList &frames)
        : QUndoCommand(),
          m_bmm(bmm),
          m_parameter(bmm->interpolationParameter())
    {
        QHash<int, bool> indexProceeded;
        /* 現在選択中のモデルにある全てのボーンを取り出す */
        const BoneMotionModel::TreeItemList &items = m_bmm->keys().values();
        /* フレームインデックスがまたがるので複雑だが対象のキーフレームを全て保存しておく */
        foreach (const BoneMotionModel::KeyFramePair &frame, frames) {
            int frameIndex = frame.first;
            /* フレーム単位での重複を避けるためにスキップ処理を設ける */
            if (!indexProceeded[frameIndex]) {
                /* モデルの全てのボーンを対象にデータがあるか確認し、存在している場合のみボーンのキーフレームの生データを保存する */
                foreach (PMDMotionModel::ITreeItem *item, items) {
                    const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, frameIndex);
                    const QVariant &data = index.data(BoneMotionModel::kBinaryDataRole);
                    if (data.canConvert(QVariant::ByteArray))
                        m_modelIndices.append(ModelIndex(index, data.toByteArray()));
                }
                indexProceeded[frameIndex] = true;
            }
        }
        m_frames = frames;
        m_frameIndices = indexProceeded.keys();
    }
    virtual ~SetFramesCommand() {
    }

    virtual void undo() {
        /* 対象のキーフレームのインデックスを全て削除、さらにモデルのデータも削除 */
        const BoneMotionModel::TreeItemList &items = m_bmm->keys().values();
        BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        foreach (int frameIndex, m_frameIndices) {
            animation->deleteKeyframes(frameIndex);
            foreach (PMDMotionModel::ITreeItem *item, items) {
                const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, frameIndex);
                m_bmm->setData(index, QVariant());
            }
        }
        /* コンストラクタで保存したキーフレームの生データからボーンのキーフレームに復元して置換する */
        foreach (const ModelIndex &index, m_modelIndices) {
            const QByteArray &bytes = index.second;
            m_bmm->setData(index.first, bytes, Qt::EditRole);
            BoneKeyframe *frame = new BoneKeyframe();
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->replaceKeyframe(frame);
        }
        /* LoadPoseCommand#undo の通りのため、省略 */
        animation->refresh();
        m_bmm->refreshModel();
    }
    virtual void redo() {
        QString key;
        BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        const BoneMotionModel::Keys &keys = m_bmm->keys();
        Bone *selected = m_bmm->selectedBone();
        /* すべてのキーフレーム情報を登録する */
        foreach (const BoneMotionModel::KeyFramePair &pair, m_frames) {
            int frameIndex = pair.first;
            BoneMotionModel::KeyFramePtr data = pair.second;
            BoneKeyframe *frame = data.data();
            /* キーフレームの対象ボーン名を取得する */
            if (frame) {
                key = internal::toQString(frame);
            }
            else if (selected) {
                key = internal::toQString(selected);
            }
            else {
                qWarning("No bone is selected or null");
                continue;
            }
            /* モデルにボーン名が存在するかを確認する */
            if (keys.contains(key)) {
                /*
                 * キーフレームをコピーし、対象のモデルのインデックスに移す。
                 * そしてモデルにデータを登録した上で現在登録されているキーフレームを置換する
                 * (前のキーフレームの情報が入ってる可能性があるので、それ故に重複が発生することを防ぐ)
                 */
                const QModelIndex &modelIndex = m_bmm->frameIndexToModelIndex(keys[key], frameIndex);
                QByteArray bytes(BoneKeyframe::strideSize(), '0');
                BoneKeyframe *newFrame = static_cast<BoneKeyframe *>(frame->clone());
                newFrame->setInterpolationParameter(BoneKeyframe::kX, m_parameter.x);
                newFrame->setInterpolationParameter(BoneKeyframe::kY, m_parameter.y);
                newFrame->setInterpolationParameter(BoneKeyframe::kZ, m_parameter.z);
                newFrame->setInterpolationParameter(BoneKeyframe::kRotation, m_parameter.rotation);
                newFrame->setFrameIndex(frameIndex);
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                animation->replaceKeyframe(newFrame);
                m_bmm->setData(modelIndex, bytes);
            }
            else {
                qWarning("Tried registering not bone key frame: %s", qPrintable(key));
                continue;
            }
        }
        /* LoadPoseCommand#undo の通りのため、省略 */
        animation->refresh();
        m_bmm->refreshModel();
    }

private:
    QList<int> m_frameIndices;
    QList<ModelIndex> m_modelIndices;
    BoneMotionModel::KeyFramePairList m_frames;
    BoneMotionModel *m_bmm;
    BoneKeyframe::InterpolationParameter m_parameter;
};

class ResetAllCommand : public QUndoCommand
{
public:
    ResetAllCommand(PMDModel *model)
        : QUndoCommand(),
          m_model(model)
    {
        /* 全てのボーンの情報を保存しておく */
        m_state = model->saveState();
    }
    virtual ~ResetAllCommand() {
        /* discardState は必ず呼び出すこと */
        m_model->discardState(m_state);
    }

    void undo() {
        /* コンストラクタで保存したボーン情報を復元し、シークせずにモデルを更新しておく */
        m_model->restoreState(m_state);
        m_model->updateImmediate();
    }
    void redo() {
        /* 全てのボーンをリセットし、シークせずにモデルを更新しておく */
        m_model->resetAllBones();
        m_model->updateImmediate();
    }

private:
    PMDModel *m_model;
    PMDModel::State *m_state;
};

class SetBoneCommand : public QUndoCommand
{
public:
    SetBoneCommand(PMDModel *model, PMDModel::State *state)
        : QUndoCommand(),
          m_model(model),
          m_newState(0),
          m_oldState(state)
    {
        /* 前と後の全てのボーンの情報を保存しておく */
        m_newState = m_model->saveState();
    }
    virtual ~SetBoneCommand() {
        /* コンストラクタで saveState が呼ばれる前提なので両方解放しておく */
        m_model->discardState(m_newState);
        m_model->discardState(m_oldState);
    }

    void undo() {
        /* コンストラクタで呼ばれる前のボーン情報を復元し、シークせずにモデルを更新しておく */
        m_model->restoreState(m_oldState);
        m_model->updateImmediate();
    }
    void redo() {
        /* コンストラクタで呼ばれた時点のボーン情報を復元し、シークせずにモデルを更新しておく */
        m_model->restoreState(m_newState);
        m_model->updateImmediate();
    }

private:
    PMDModel *m_model;
    PMDModel::State *m_newState;
    PMDModel::State *m_oldState;
};

static Bone *BoneFromModelIndex(const QModelIndex &index, PMDModel *model)
{
    /* QModelIndex -> TreeIndex -> ByteArray -> Bone の順番で対象のボーンを求めて選択状態にする作業 */
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    QByteArray bytes = internal::fromQString(item->name());
    return model->findBone(reinterpret_cast<const uint8_t *>(bytes.constData()));
}

}

BoneMotionModel::BoneMotionModel(QUndoGroup *undo, const SceneWidget *sceneWidget, QObject *parent) :
    PMDMotionModel(undo, parent),
    m_sceneWidget(sceneWidget),
    m_state(0)
{
}

BoneMotionModel::~BoneMotionModel()
{
    m_frames.releaseAll();
}

void BoneMotionModel::saveMotion(VMDMotion *motion)
{
    if (m_model) {
        /* モデルの ByteArray を BoneKeyFrame に読ませて積んでおくだけの簡単な処理 */
        BoneAnimation *animation = motion->mutableBoneAnimation();
        foreach (const QVariant &value, values()) {
            BoneKeyframe *newFrame = new BoneKeyframe();
            const QByteArray &bytes = value.toByteArray();
            newFrame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->addKeyframe(newFrame);
        }
        setModified(false);
    }
    else {
        qWarning("No model is selected to save motion.");
    }
}

void BoneMotionModel::addKeyframesByModelIndices(const QModelIndexList &indices)
{
    KeyFramePairList boneFrames;
    PMDModel *model = selectedModel();
    /* モデルのインデックスを参照し、存在するボーンに対してボーンの現在の値からボーンのキーフレームにコピーする */
    foreach (const QModelIndex &index, indices) {
        int frameIndex = toFrameIndex(index);
        if (frameIndex >= 0) {
            const QByteArray &name = nameFromModelIndex(index);
            Bone *bone = model->findBone(reinterpret_cast<const uint8_t *>(name.constData()));
            if (bone) {
                /* 補間パラメータは SetFramesCommand の中で設定されるため、初期化のみ */
                BoneKeyframe *frame = new BoneKeyframe();
                frame->setDefaultInterpolationParameter();
                frame->setName(bone->name());
                frame->setPosition(bone->position());
                frame->setRotation(bone->rotation());
                boneFrames.append(KeyFramePair(frameIndex, KeyFramePtr(frame)));
            }
        }
    }
    setFrames(boneFrames);
}

void BoneMotionModel::copyKeyframes(int frameIndex)
{
    if (m_model && m_motion) {
        /* メモリリーク防止のため、前回呼ばれた copyFrames で作成したデータを破棄しておく */
        m_frames.releaseAll();
        /* モデル内のすべてのボーン名を参照し、データがあるものだけを BoneKeyFrame に移しておく */
        foreach (PMDMotionModel::ITreeItem *item, keys().values()) {
            const QModelIndex &index = frameIndexToModelIndex(item, frameIndex);
            QVariant variant = index.data(kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                QByteArray bytes = variant.toByteArray();
                BoneKeyframe *frame = new BoneKeyframe();
                frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
                m_frames.add(frame);
            }
        }
    }
}

void BoneMotionModel::pasteKeyframes(int frameIndex)
{
    /* m_frames が #copyFrames でコピーされていること前提 */
    if (m_model && m_motion && m_frames.count() != 0) {
        /*
         * m_frames のデータを引数のフレームインデックスと一緒に積ませて SetFramesCommand として作成する
         * m_frames のデータは破棄されないので、#copyFrames で破棄するようになってる
         */
        KeyFramePairList frames;
        const int nframes = m_frames.count();
        for (int i = 0; i < nframes; i++) {
            BoneKeyframe *frame = static_cast<BoneKeyframe *>(m_frames[i]->clone());
            frames.append(KeyFramePair(frameIndex, KeyFramePtr(frame)));
        }
        addUndoCommand(new SetFramesCommand(this, frames));
    }
}

void BoneMotionModel::pasteReversedFrame(int frameIndex)
{
    const QString &right = "右";
    const QString &left = "左";
    QHash<QString, int> registered;
    /* m_frames が #copyFrames でコピーされていること前提 */
    if (m_model && m_motion && m_frames.count() != 0) {
        KeyFramePairList frames;
        const int nframes = m_frames.count();
        /* 基本的な処理は pasteFrame と同等だが、「左」「右」の名前を持つボーンは特別扱い */
        for (int i = 0; i < nframes; i++) {
            BoneKeyframe *frame = static_cast<BoneKeyframe *>(m_frames[i]), *newFrame = 0;
            const QString &name = internal::toQString(frame);
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
                    QByteArray bytes = internal::fromQString(key);
                    newFrame = static_cast<BoneKeyframe *>(frame->clone());
                    newFrame->setName(reinterpret_cast<const uint8_t *>(bytes.constData()));
                    Vector3 position = newFrame->position();
                    position.setValue(-position.x(), position.y(), position.z());
                    newFrame->setPosition(position);
                    Quaternion rotation = newFrame->rotation();
                    rotation.setValue(rotation.x(), -rotation.y(), -rotation.z(), rotation.w());
                    newFrame->setRotation(rotation);
                    registered[key] = 1;
                }
            }
            else {
                newFrame = static_cast<BoneKeyframe *>(m_frames[i]->clone());
            }
            frames.append(KeyFramePair(frameIndex, KeyFramePtr(newFrame)));
        }
        addUndoCommand(new SetFramesCommand(this, frames));
    }
}

void BoneMotionModel::saveTransform()
{
    if (m_model) {
        /* モデルの状態を保存しておく。メモリリーク防止のため、前の状態は破棄しておく */
        m_model->discardState(m_state);
        m_state = m_model->saveState();
    }
}

void BoneMotionModel::commitTransform()
{
    if (m_model && m_state) {
        /*
         * startTransform で保存したモデルの状態を SetBoneCommand に渡す
         * メモリ管理はそちらに移動するので m_state は 0 にして無効にしておく
         */
        addUndoCommand(new SetBoneCommand(m_model, m_state));
        m_state = 0;
    }
}

void BoneMotionModel::selectKeyframesByModelIndices(const QModelIndexList &indices)
{
    if (m_model) {
        QList<Bone *> bones;
        QList<KeyFramePtr> frames;
        foreach (const QModelIndex &index, indices) {
            if (index.isValid()) {
                Bone *bone = BoneFromModelIndex(index, m_model);
                if (bone)
                    bones.append(bone);
                const QVariant &data = index.data(kBinaryDataRole);
                if (data.canConvert(QVariant::ByteArray)) {
                    BoneKeyframe *frame = new BoneKeyframe();
                    frame->read(reinterpret_cast<const uint8_t *>(data.toByteArray().constData()));
                    frames.append(KeyFramePtr(frame));
                }
            }
        }
        emit keyframesDidSelect(frames);
    }
}

const QByteArray BoneMotionModel::nameFromModelIndex(const QModelIndex &index) const
{
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    return internal::fromQString(item->name());
}

void BoneMotionModel::loadPose(VPDFile *pose, PMDModel *model, int frameIndex)
{
    if (model == m_model && m_motion) {
        addUndoCommand(new LoadPoseCommand(this, pose, frameIndex));
        qDebug("Loaded a pose to the model: %s", qPrintable(internal::toQString(model)));
    }
    else {
        qWarning("Tried loading pose to invalid model or without motion: %s", qPrintable(internal::toQString(model)));
    }
}

void BoneMotionModel::savePose(VPDFile *pose, PMDModel *model, int frameIndex)
{
    if (model == m_model) {
        VPDFile::BoneList bones;
        /* モデルにある全てのボーンを参照し、現在のキーフレームでデータが入ってるものを VPDFile::Bone に変換する */
        foreach (ITreeItem *item, keys()) {
            const QModelIndex &modelIndex = frameIndexToModelIndex(item, frameIndex);
            const QVariant &variant = modelIndex.data(BoneMotionModel::kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                VPDFile::Bone *bone = new VPDFile::Bone();
                BoneKeyframe frame;
                frame.read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
                const Quaternion &q = frame.rotation();
                bone->name = internal::toQString(&frame);
                bone->position = frame.position();
                bone->rotation = Vector4(q.x(), q.y(), q.z(), q.w());
                bones.append(bone);
            }
        }
        pose->setBones(bones);
    }
    else {
        qWarning("Tried loading pose to invalid model or without motion: %s", qPrintable(internal::toQString(model)));
    }
}

void BoneMotionModel::setFrames(const KeyFramePairList &frames)
{
    if (m_model && m_motion)
        addUndoCommand(new SetFramesCommand(this, frames));
    else
        qWarning("No model or motion to register bone frames.");
}

void BoneMotionModel::setPMDModel(PMDModel *model)
{
    /* 引数のモデルが現在選択中のものであれば二重処理になってしまうので、スキップする */
    if (m_model == model)
        return;
    if (model) {
        /* PMD の二重登録防止 */
        Bone *centerBone;
        if (!hasPMDModel(model)) {
            /* ルートを作成 */
            RootPtr ptr(new TreeItem("", 0, true, false, 0));
            TreeItem *r = static_cast<TreeItem *>(ptr.data());
            Array<BoneList *> allBones;
            Array<uint8_t *> names;
            allBones.copy(model->bonesForUI());
            names.copy(model->boneCategoryNames());
            Keys keys;
            /*
             * センターボーンはセンターボーン専用でカテゴリをつける。以前はカテゴリではないが表示上カテゴリに位置していたが、
             * モーションを読み込んだ時モデルのインデックス作成において処理速度が大幅に落ちてしまったことが 0.9.0 で判明したため、
             * このような扱いにしている
             * TODO: 別のカテゴリにつけられたセンターボーンをもつモデルの対処
             */
            centerBone = Bone::centerBone(&model->bones());
            const QString &centerBoneName = internal::toQString(centerBone);
            TreeItem *centerBoneCategory = new TreeItem(centerBoneName, 0, false, true, r);
            TreeItem *centerBoneItem = new TreeItem(centerBoneName, centerBone, false, false, centerBoneCategory);
            keys.insert(centerBoneName, centerBoneItem);
            centerBoneCategory->addChild(centerBoneItem);
            r->addChild(centerBoneCategory);
            const int namesCount = model->boneCategoryNames().count();
            /* ボーンのカテゴリからルートの子供であるカテゴリアイテムを作成する */
            for (int i = 0; i < namesCount; i++) {
                const QString &category = internal::toQString(names[i]).trimmed();
                const BoneList *bones = allBones[i];
                const int bonesCount = bones->count();
                TreeItem *parent = new TreeItem(category, 0, false, true, r);
                /* カテゴリに属するボーン名を求めてカテゴリアイテムに追加する。また、ボーン名をキー名として追加 */
                for (int j = 0; j < bonesCount; j++) {
                    Bone *bone = bones->at(j);
                    const QString &name = internal::toQString(bone);
                    TreeItem *child = new TreeItem(name, bone, false, false, parent);
                    parent->addChild(child);
                    keys.insert(name, child);
                }
                /* カテゴリアイテムをルートアイテムに追加 */
                r->addChild(parent);
            }
            addPMDModel(model, ptr, keys);
        }
        else {
            /* キーリストが空でもモデルが存在し、スキップされるので実害なし */
            centerBone = Bone::centerBone(&model->bones());
            addPMDModel(model, rootPtr(model), Keys());
        }
        m_model = model;
        emit modelDidChange(model);
        /* 予めセンターボーンを選択しておく */
        QList<Bone *> bones;
        bones.append(centerBone);
        selectBones(bones);
        qDebug("Set a model in BoneMotionModel: %s", qPrintable(internal::toQString(model)));
    }
    else {
        m_model = 0;
        selectBones(QList<Bone *>());
        emit modelDidChange(0);
    }
    /* テーブルモデルを更新 */
    reset();
}

void BoneMotionModel::loadMotion(VMDMotion *motion, PMDModel *model)
{
    /* 現在のモデルが対象のモデルと一致していることを確認しておく */
    if (model == m_model) {
        const BoneAnimation &animation = motion->boneAnimation();
        const int nBoneFrames = animation.countKeyframes();
        const Keys &keys = this->keys();
        /* モーションのすべてのキーフレームを参照し、モデルのボーン名に存在するものだけ登録する */
        for (int i = 0; i < nBoneFrames; i++) {
            const BoneKeyframe *frame = animation.frameAt(i);
            const uint8_t *name = frame->name();
            const QString &key = internal::toQString(name);
            if (keys.contains(key)) {
                int frameIndex = static_cast<int>(frame->frameIndex());
                QByteArray bytes(BoneKeyframe::strideSize(), '0');
                ITreeItem *item = keys[key];
                /* この時点で新しい QModelIndex が作成される */
                const QModelIndex &modelIndex = frameIndexToModelIndex(item, frameIndex);
                BoneKeyframe newFrame;
                newFrame.setName(name);
                newFrame.setPosition(frame->position());
                newFrame.setRotation(frame->rotation());
                newFrame.setFrameIndex(frameIndex);
                QuadWord v;
                for (int i = 0; i < BoneKeyframe::kMax; i++) {
                    BoneKeyframe::InterpolationType type = static_cast<BoneKeyframe::InterpolationType>(i);
                    frame->getInterpolationParameter(type, v);
                    newFrame.setInterpolationParameter(type, v);
                }
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                /* キーフレームのバイナリデータが QModelIndex の QVariant に登録される。この方が管理が楽になる */
                setData(modelIndex, bytes);
            }
        }
        /* 読み込まれたモーションを現在のモーションとして登録する。あとは LoadCommand#undo と同じ */
        m_motion = motion;
        refreshModel();
        setModified(false);
        qDebug("Loaded a motion to the model in BoneMotionModel: %s", qPrintable(internal::toQString(model)));
    }
    else {
        qDebug("Tried loading a motion to different model, ignored: %s", qPrintable(internal::toQString(model)));
    }
}

void BoneMotionModel::removeMotion()
{
    /* 選択されたボーンとモデルに登録されているデータが削除される。ボーン名は削除されない */
    m_selected.clear();
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
    emit modelDidChange(0);
}

void BoneMotionModel::deleteKeyframesByModelIndices(const QModelIndexList &indices)
{
    BoneAnimation *animation = m_motion->mutableBoneAnimation();
    foreach (const QModelIndex &index, indices) {
        if (index.isValid() && index.column() > 1) {
            /* QModelIndex にあるボーンとフレームインデックスからキーフレームを削除する */
            TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
            if (Bone *bone = item->bone())
                animation->deleteKeyframe(toFrameIndex(index), bone->name());
            setData(index, QVariant());
        }
    }
}

void BoneMotionModel::applyKeyframeWeightByModelIndices(const QModelIndexList &indices, float value)
{
    KeyFramePairList keyframes;
    foreach (const QModelIndex &index, indices) {
        if (index.isValid()) {
            /* QModelIndex にあるボーンとフレームインデックスからキーフレームを取得し、値を補正する */
            TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
            if (Bone *bone = item->bone()) {
                BoneKeyframe *keyframe = new BoneKeyframe();
                keyframe->setName(bone->name());
                keyframe->setPosition(bone->position() * value);
                keyframe->setRotation(bone->rotation() * value);
                keyframes.append(KeyFramePair(toFrameIndex(index), KeyFramePtr(keyframe)));
            }
        }
    }
    setFrames(keyframes);
}

void BoneMotionModel::selectBonesByModelIndices(const QModelIndexList &indices)
{
    QList<Bone *> bones;
    foreach (const QModelIndex &index, indices) {
        if (index.isValid()) {
            TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
            if (Bone *bone = item->bone())
                bones.append(bone);
        }
    }
    m_selected = bones;
    emit bonesDidSelect(bones);
}

void BoneMotionModel::resetBone(ResetType type)
{
    foreach (Bone *selected, m_selected) {
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
    updateModel();
}

void BoneMotionModel::resetAllBones()
{
    if (m_model)
        addUndoCommand(new ResetAllCommand(m_model));
}

void BoneMotionModel::setPosition(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    foreach (Bone *selected, m_selected) {
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
        m_model->updateImmediate();
        emit positionDidChange(selected, lastPosition);
    }
}

void BoneMotionModel::setRotation(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    Bone *selected = m_selected.last();
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
    m_model->updateImmediate();
    emit rotationDidChange(selected, lastRotation);
}

void BoneMotionModel::translate(const Vector3 &delta, Bone *bone, int flags)
{
    if (!bone) {
        if (isBoneSelected())
            bone = selectedBone();
        else
            return;
    }
    const Vector3 &lastPosition = bone->position();
    switch (flags & 0xff) {
    case 'V': {
        const Transform &modelViewTransform = m_sceneWidget->sceneLoader()->renderEngine()->scene()->modelViewTransform();
        const Vector3 &value2 = modelViewTransform.getBasis() * delta;
        bone->setPosition(Transform(bone->rotation(), lastPosition) * value2);
        break;
    }
    case 'L': {
        bone->setPosition(Transform(bone->rotation(), lastPosition) * delta);
        break;
    }
    case 'G': {
        bone->setPosition(lastPosition + delta);
        break;
    }
    default: {
        qFatal("Unexpected mode: %c", flags & 0xff);
        break;
    }
    }
    m_model->updateImmediate();
    emit positionDidChange(bone, lastPosition);
}

void BoneMotionModel::rotate(const Quaternion &delta, Bone *bone, int flags, float value)
{
    if (!bone) {
        if (isBoneSelected())
            bone = selectedBone();
        else
            return;
    }
    const Quaternion &lastRotation = bone->rotation();
    switch (flags & 0xff) {
    case 'V': {
        float matrixf[16];
        m_sceneWidget->sceneLoader()->renderEngine()->scene()->getModelViewMatrix(matrixf);
        const QMatrix4x4 &matrix = internal::toMatrix4x4(matrixf);
        const QVector4D &r = (matrix * QVector4D(delta.x(), delta.y(), delta.z(), delta.w())).normalized();
        bone->setRotation(lastRotation * Quaternion(r.x(), r.y(), r.z(), value < 0 ? r.w() : -r.w()));
        break;
    }
    case 'L': {
        /* 座標系の関係でX軸とY軸は値を反転させる */
        Quaternion rot = Quaternion::getIdentity();
        const QString &name = internal::toQString(bone);
        const Bone *child = bone->child();
        /* ボーン名によって特別扱いする必要がある */
        if ((name.indexOf("指") != -1
             || name.endsWith("腕")
             || name.endsWith("ひじ")
             || name.endsWith("手首")
             ) && child) {
            /* 子ボーンの方向をX軸、手前の方向をZ軸として設定する */
            const Vector3 &boneOrigin = bone->originPosition();
            const Vector3 &childOrigin = child->originPosition();
            /* 外積を使ってそれぞれの軸を求める */
            const Vector3 &axisX = (childOrigin - boneOrigin).normalized();
            Vector3 tmp1 = axisX;
            name.startsWith("左") ? tmp1.setY(-axisX.y()) : tmp1.setX(-axisX.x());
            Vector3 axisZ = axisX.cross(tmp1).normalized(), tmp2 = axisX;
            tmp2.setZ(-axisZ.z());
            Vector3 axisY = tmp2.cross(-axisX).normalized();
            /*  0x0000ff00 <= ff の部分に X/Y/Z のいずれかの軸のフラグが入ってる */
            switch ((flags & 0xff00) >> 8) {
            case 'X': {
                rot.setRotation(axisX, -radian(value));
                break;
            }
            case 'Y': {
                rot.setRotation(axisY, -radian(value));
                break;
            }
            case 'Z': {
                rot.setRotation(axisZ, radian(value));
                break;
            }
            }
            bone->setRotation(lastRotation * rot);
        }
        else {
            switch ((flags & 0xff00) >> 8) {
            case 'X': {
                rot.setRotation(Vector3(1, 0, 0), -radian(value));
                break;
            }
            case 'Y': {
                rot.setRotation(Vector3(0, 1, 0), -radian(value));
                break;
            }
            case 'Z': {
                rot.setRotation(Vector3(0, 0, 1), radian(value));
                break;
            }
            }
            bone->setRotation(lastRotation * rot);
        }
        break;
    }
    case 'G': {
        bone->setRotation(lastRotation * delta);
        break;
    }
    default: {
        qFatal("Unexpected mode: %c", flags & 0xff);
        break;
    }
    }
    m_model->updateImmediate();
    emit rotationDidChange(bone, lastRotation);
}

void BoneMotionModel::selectBones(const QList<Bone *> &bones)
{
    m_selected = bones;
    emit bonesDidSelect(bones);
}

Bone *BoneMotionModel::findBone(const QString &name)
{
    /* QString を扱っていること以外 PMDModel#findBone と同じ */
    const QByteArray &bytes = internal::getTextCodec()->fromUnicode(name);
    foreach (ITreeItem *item, keys()) {
        Bone *bone = static_cast<TreeItem *>(item)->bone();
        if (!qstrcmp(reinterpret_cast<const char *>(bone->name()), bytes))
            return bone;
    }
    return 0;
}

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

#include "common/SceneWidget.h"
#include "common/VPDFile.h"
#include "common/util.h"
#include "models/BoneMotionModel.h"
#include <vpvl/vpvl.h>

namespace
{

class TreeItem : public MotionBaseModel::ITreeItem
{
public:
    TreeItem(const QString &name, vpvl::Bone *bone, bool isRoot, bool isCategory, TreeItem *parent)
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
    vpvl::Bone *bone() const {
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
    vpvl::Bone *m_bone;
    bool m_isRoot;
    bool m_isCategory;
};

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
            if (index.data(BoneMotionModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
                m_indices.append(index);
        }
        m_pose = pose->clone();
    }
    virtual ~LoadPoseCommand() {
        delete m_pose;
    }

    virtual void undo() {
        vpvl::BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        /* 現在のフレームを削除しておき、さらに全てのボーンのモデルのデータを空にしておく(=削除) */
        animation->deleteKeyFrames(m_frameIndex);
        foreach (PMDMotionModel::ITreeItem *item, m_bmm->keys().values()) {
            const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, m_frameIndex);
            m_bmm->setData(index, QVariant());
        }
        /*
         * コンストラクタで保存したボーン情報を復元して置換する。注意点として replaceKeyFrame でメモリの所有者が
         * vpvl::BoneAnimation に移動するのでこちらで管理する必要がなくなる
         */
        foreach (const QModelIndex &index, m_indices) {
            const QByteArray &bytes = index.data(BoneMotionModel::kBinaryDataRole).toByteArray();
            m_bmm->setData(index, bytes, Qt::EditRole);
            vpvl::BoneKeyFrame *frame = new vpvl::BoneKeyFrame();
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->replaceKeyFrame(frame);
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
        vpvl::BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        const BoneMotionModel::Keys &bones = m_bmm->keys();
        vpvl::Quaternion rotation;
        /* ポーズにあるボーン情報を参照する */
        foreach (VPDFile::Bone *bone, m_pose->bones()) {
            const QString &key = bone->name;
            /* ポーズにあるボーンがモデルの方に実在する */
            if (bones.contains(key)) {
                /*
                 * ポーズにあるボーン情報を元にキーフレームを作成し、モデルに登録した上で現在登録されているキーフレームを置換する
                 * replaceKeyFrame でメモリの所有者が vpvl::BoneAnimation に移動する点は同じ
                 */
                const vpvl::Vector4 &v = bone->rotation;
                const QModelIndex &modelIndex = m_bmm->frameIndexToModelIndex(bones[key], m_frameIndex);
                rotation.setValue(v.x(), v.y(), v.z(), v.w());
                vpvl::BoneKeyFrame *newFrame = new vpvl::BoneKeyFrame();
                newFrame->setDefaultInterpolationParameter();
                newFrame->setName(reinterpret_cast<const uint8_t *>(codec->fromUnicode(key).constData()));
                newFrame->setPosition(bone->position);
                newFrame->setRotation(rotation);
                newFrame->setFrameIndex(m_frameIndex);
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                m_bmm->setData(modelIndex, bytes, Qt::EditRole);
                animation->replaceKeyFrame(newFrame);
            }
        }
        /* #undo のコメント通りのため、省略 */
        animation->refresh();
        m_bmm->refreshModel();
    }

private:
    QModelIndexList m_indices;
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
        const BoneMotionModel::TreeItemList &items = m_bmm->keys().values();
        /* フレームインデックスがまたがるので複雑だが対象のキーフレームを全て保存しておく */
        foreach (const BoneMotionModel::KeyFramePair &frame, frames) {
            int frameIndex = frame.first;
            /* フレーム単位での重複を避けるためにスキップ処理を設ける */
            if (!indexProceeded[frameIndex]) {
                /* モデルの全てのボーンを対象にデータがあるか確認し、あれば保存する */
                foreach (PMDMotionModel::ITreeItem *item, items) {
                    const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, frameIndex);
                    if (index.data(BoneMotionModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
                        m_indices.append(index);
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
        vpvl::BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        foreach (int frameIndex, m_frameIndices) {
            animation->deleteKeyFrames(frameIndex);
            foreach (PMDMotionModel::ITreeItem *item, m_bmm->keys().values()) {
                const QModelIndex &index = m_bmm->frameIndexToModelIndex(item, frameIndex);
                m_bmm->setData(index, QVariant());
            }
        }
        /* コンストラクタで保存したボーン情報を復元して置換する */
        foreach (const QModelIndex &index, m_indices) {
            const QByteArray &bytes = index.data(BoneMotionModel::kBinaryDataRole).toByteArray();
            m_bmm->setData(index, bytes, Qt::EditRole);
            vpvl::BoneKeyFrame *frame = new vpvl::BoneKeyFrame();
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->replaceKeyFrame(frame);
        }
        /* LoadPoseCommand#undo の通りのため、省略 */
        animation->refresh();
        m_bmm->refreshModel();
    }
    virtual void redo() {
        QString key;
        vpvl::BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        const BoneMotionModel::Keys &keys = m_bmm->keys();
        vpvl::Bone *selected = m_bmm->selectedBone();
        /* すべてのキーフレーム情報を登録する */
        foreach (const BoneMotionModel::KeyFramePair &pair, m_frames) {
            int frameIndex = pair.first;
            BoneMotionModel::KeyFramePtr data = pair.second;
            vpvl::BoneKeyFrame *frame = data.data();
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
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                vpvl::BoneKeyFrame *newFrame = static_cast<vpvl::BoneKeyFrame *>(frame->clone());
                newFrame->setInterpolationParameter(vpvl::BoneKeyFrame::kX, m_parameter.x);
                newFrame->setInterpolationParameter(vpvl::BoneKeyFrame::kY, m_parameter.y);
                newFrame->setInterpolationParameter(vpvl::BoneKeyFrame::kZ, m_parameter.z);
                newFrame->setInterpolationParameter(vpvl::BoneKeyFrame::kRotation, m_parameter.rotation);
                newFrame->setFrameIndex(frameIndex);
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                animation->replaceKeyFrame(newFrame);
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
    QModelIndexList m_indices;
    BoneMotionModel::KeyFramePairList m_frames;
    BoneMotionModel *m_bmm;
    vpvl::BoneKeyFrame::InterpolationParameter m_parameter;
};

class ResetAllCommand : public QUndoCommand
{
public:
    ResetAllCommand(vpvl::PMDModel *model)
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
    vpvl::PMDModel *m_model;
    vpvl::PMDModel::State *m_state;
};

class SetBoneCommand : public QUndoCommand
{
public:
    SetBoneCommand(vpvl::PMDModel *model, vpvl::PMDModel::State *state)
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
    vpvl::PMDModel *m_model;
    vpvl::PMDModel::State *m_newState;
    vpvl::PMDModel::State *m_oldState;
};

}

BoneMotionModel::BoneMotionModel(QUndoGroup *undo, const SceneWidget *sceneWidget, QObject *parent) :
    PMDMotionModel(undo, parent),
    m_sceneWidget(sceneWidget),
    m_state(0),
    m_mode(kLocal)
{
}

BoneMotionModel::~BoneMotionModel()
{
    m_frames.releaseAll();
}

void BoneMotionModel::saveMotion(vpvl::VMDMotion *motion)
{
    if (m_model) {
        /* モデルの ByteArray を vpvl::BoneKeyFrame に読ませて積んでおくだけの簡単な処理 */
        vpvl::BoneAnimation *animation = motion->mutableBoneAnimation();
        foreach (const QVariant &value, values()) {
            vpvl::BoneKeyFrame *newFrame = new vpvl::BoneKeyFrame();
            const QByteArray &bytes = value.toByteArray();
            newFrame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->addKeyFrame(newFrame);
        }
        setModified(false);
    }
    else {
        qWarning("No model is selected to save motion.");
    }
}

void BoneMotionModel::addKeyFramesByModelIndices(const QModelIndexList &indices)
{
    KeyFramePairList boneFrames;
    vpvl::PMDModel *model = selectedModel();
    /* モデルのインデックスを参照し、存在するボーンに対してボーンの現在の値からボーンのキーフレームにコピーする */
    foreach (const QModelIndex &index, indices) {
        int frameIndex = toFrameIndex(index);
        if (frameIndex >= 0) {
            const QByteArray &name = nameFromModelIndex(index);
            vpvl::Bone *bone = model->findBone(reinterpret_cast<const uint8_t *>(name.constData()));
            if (bone) {
                /* 補間パラメータは SetFramesCommand の中で設定されるため、初期化のみ */
                vpvl::BoneKeyFrame *frame = new vpvl::BoneKeyFrame();
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

void BoneMotionModel::copyFrames(int frameIndex)
{
    if (m_model && m_motion) {
        /* メモリリーク防止のため、前回呼ばれた copyFrames で作成したデータを破棄しておく */
        m_frames.releaseAll();
        /* モデル内のすべてのボーン名を参照し、データがあるものだけを vpvl::BoneKeyFrame に移しておく */
        foreach (PMDMotionModel::ITreeItem *item, keys().values()) {
            const QModelIndex &index = frameIndexToModelIndex(item, frameIndex);
            QVariant variant = index.data(kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                QByteArray bytes = variant.toByteArray();
                vpvl::BoneKeyFrame *frame = new vpvl::BoneKeyFrame();
                frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
                m_frames.add(frame);
            }
        }
    }
}

void BoneMotionModel::pasteFrame(int frameIndex)
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
            vpvl::BoneKeyFrame *frame = static_cast<vpvl::BoneKeyFrame *>(m_frames[i]->clone());
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
            vpvl::BoneKeyFrame *frame = static_cast<vpvl::BoneKeyFrame *>(m_frames[i]), *newFrame = 0;
            const QString name = internal::toQString(frame);
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
                    newFrame = static_cast<vpvl::BoneKeyFrame *>(frame->clone());
                    newFrame->setName(reinterpret_cast<const uint8_t *>(bytes.constData()));
                    vpvl::Vector3 position = newFrame->position();
                    position.setValue(-position.x(), position.y(), position.z());
                    newFrame->setPosition(position);
                    vpvl::Quaternion rotation = newFrame->rotation();
                    rotation.setValue(rotation.x(), -rotation.y(), -rotation.z(), rotation.w());
                    newFrame->setRotation(rotation);
                    registered[key] = 1;
                }
            }
            else {
                newFrame = static_cast<vpvl::BoneKeyFrame *>(m_frames[i]->clone());
            }
            frames.append(KeyFramePair(frameIndex, KeyFramePtr(newFrame)));
        }
        addUndoCommand(new SetFramesCommand(this, frames));
    }
}

void BoneMotionModel::startTransform()
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

void BoneMotionModel::selectByModelIndex(const QModelIndex &index)
{
    if (m_model) {
        /* QModelIndex -> TreeIndex -> ByteArray -> vpvl::Bone の順番で対象のボーンを求めて選択状態にする作業 */
        TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
        QByteArray bytes = internal::fromQString(item->name());
        vpvl::Bone *bone = m_model->findBone(reinterpret_cast<const uint8_t *>(bytes.constData()));
        /* 対象のボーンが発見した場合のみ選択状態にする */
        if (bone) {
            QList<vpvl::Bone *> bones;
            bones.append(bone);
            selectBones(bones);
        }
    }
}

const QByteArray BoneMotionModel::nameFromModelIndex(const QModelIndex &index) const
{
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    return internal::fromQString(item->name());
}

void BoneMotionModel::loadPose(VPDFile *pose, vpvl::PMDModel *model, int frameIndex)
{
    if (model == m_model && m_motion) {
        addUndoCommand(new LoadPoseCommand(this, pose, frameIndex));
        qDebug("Loaded a pose to the model: %s", qPrintable(internal::toQString(model)));
    }
    else {
        qWarning("Tried loading pose to invalid model or without motion: %s", qPrintable(internal::toQString(model)));
    }
}

void BoneMotionModel::savePose(VPDFile *pose, vpvl::PMDModel *model, int frameIndex)
{
    if (model == m_model) {
        VPDFile::BoneList bones;
        /* モデルにある全てのボーンを参照し、現在のキーフレームでデータが入ってるものを VPDFile::Bone に変換する */
        foreach (ITreeItem *item, keys()) {
            const QModelIndex &modelIndex = frameIndexToModelIndex(item, frameIndex);
            const QVariant &variant = modelIndex.data(BoneMotionModel::kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                VPDFile::Bone *bone = new VPDFile::Bone();
                vpvl::BoneKeyFrame frame;
                frame.read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
                const vpvl::Quaternion &q = frame.rotation();
                bone->name = internal::toQString(&frame);
                bone->position = frame.position();
                bone->rotation = vpvl::Vector4(q.x(), q.y(), q.z(), q.w());
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

void BoneMotionModel::setPMDModel(vpvl::PMDModel *model)
{
    if (model) {
        /* PMD の二重登録防止 */
        vpvl::Bone *centerBone;
        if (!hasPMDModel(model)) {
            /* ルートを作成 */
            RootPtr ptr(new TreeItem("", 0, true, false, 0));
            TreeItem *r = static_cast<TreeItem *>(ptr.data());
            vpvl::Array<vpvl::BoneList *> allBones;
            vpvl::Array<uint8_t *> names;
            allBones.copy(model->bonesForUI());
            names.copy(model->boneCategoryNames());
            Keys keys;
            /*
             * センターボーンはセンターボーン専用でカテゴリをつける。以前はカテゴリではないが表示上カテゴリに位置していたが、
             * モーションを読み込んだ時モデルのインデックス作成において処理速度が大幅に落ちてしまったことが 0.9.0 で判明したため、
             * このような扱いにしている
             * TODO: 別のカテゴリにつけられたセンターボーンをもつモデルの対処
             */
            centerBone = vpvl::Bone::centerBone(&model->bones());
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
                const vpvl::BoneList *bones = allBones[i];
                const int bonesCount = bones->count();
                TreeItem *parent = new TreeItem(category, 0, false, true, r);
                /* カテゴリに属するボーン名を求めてカテゴリアイテムに追加する。また、ボーン名をキー名として追加 */
                for (int j = 0; j < bonesCount; j++) {
                    vpvl::Bone *bone = bones->at(j);
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
            centerBone = vpvl::Bone::centerBone(&model->bones());
            addPMDModel(model, rootPtr(model), Keys());
        }
        m_model = model;
        emit modelDidChange(model);
        /* 予めセンターボーンを選択しておく */
        QList<vpvl::Bone *> bones;
        bones.append(centerBone);
        selectBones(bones);
        qDebug("Set a model in BoneMotionModel: %s", qPrintable(internal::toQString(model)));
    }
    else {
        m_model = 0;
        selectBones(QList<vpvl::Bone *>());
    }
    /* テーブルモデルを更新 */
    reset();
}

void BoneMotionModel::loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    /* 現在のモデルが対象のモデルと一致していることを確認しておく */
    if (model == m_model) {
        const vpvl::BoneAnimation &animation = motion->boneAnimation();
        const int nBoneFrames = animation.countKeyFrames();
        const Keys &keys = this->keys();
        /* モーションのすべてのキーフレームを参照し、モデルのボーン名に存在するものだけ登録する */
        for (int i = 0; i < nBoneFrames; i++) {
            const vpvl::BoneKeyFrame *frame = animation.frameAt(i);
            const uint8_t *name = frame->name();
            const QString &key = internal::toQString(name);
            if (keys.contains(key)) {
                int frameIndex = static_cast<int>(frame->frameIndex());
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                ITreeItem *item = keys[key];
                /* この時点で新しい QModelIndex が作成される */
                const QModelIndex &modelIndex = frameIndexToModelIndex(item, frameIndex);
                vpvl::BoneKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setPosition(frame->position());
                newFrame.setRotation(frame->rotation());
                newFrame.setFrameIndex(frameIndex);
                vpvl::QuadWord v;
                for (int i = 0; i < vpvl::BoneKeyFrame::kMax; i++) {
                    vpvl::BoneKeyFrame::InterpolationType type = static_cast<vpvl::BoneKeyFrame::InterpolationType>(i);
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
}

void BoneMotionModel::deleteFrameByModelIndex(const QModelIndex &index)
{
    if (index.isValid()) {
        /* QModelIndex にあるボーンとフレームインデックスからキーフレームを削除する */
        TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
        vpvl::Bone *bone = item->bone();
        if (bone) {
            vpvl::BoneAnimation *animation = m_motion->mutableBoneAnimation();
            animation->deleteKeyFrame(toFrameIndex(index), bone->name());
        }
        setData(index, QVariant());
    }
}

void BoneMotionModel::resetBone(ResetType type)
{
    foreach (vpvl::Bone *selected, m_selected) {
        vpvl::Vector3 pos = selected->position();
        vpvl::Quaternion rot = selected->rotation();
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

void BoneMotionModel::setMode(int value)
{
    switch (value) {
    case 0:
        m_mode = kLocal;
        break;
    case 1:
        m_mode = kGlobal;
        break;
    case 2:
        m_mode = kView;
        break;
    }
}

void BoneMotionModel::setPosition(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    foreach (vpvl::Bone *selected, m_selected) {
        vpvl::Vector3 pos = selected->position();
        switch (coordinate) {
        case 'x':
        case 'X':
            pos.setX(value);
            break;
        case 'y':
        case 'Y':
            pos.setY(value);
            break;
        case 'z':
        case 'Z':
            pos.setZ(value);
            break;
        default:
            qFatal("Unexpected coordinate value: %c", coordinate);
        }
        selected->setPosition(pos);
        m_model->updateImmediate();
        emit bonePositionDidChange(selected, pos);
    }
}

void BoneMotionModel::setRotation(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    vpvl::Bone *selected = m_selected.last();
    vpvl::Quaternion rot = selected->rotation();
    switch (coordinate) {
    case 'x':
    case 'X':
        rot.setX(value);
        break;
    case 'y':
    case 'Y':
        rot.setY(value);
        break;
    case 'z':
    case 'Z':
        rot.setZ(value);
        break;
    default:
        qFatal("Unexpected coordinate value: %c", coordinate);
    }
    selected->setRotation(rot);
    m_model->updateImmediate();
    emit boneRotationDidChange(selected, rot);
}

void BoneMotionModel::translate(int coordinate, float value)
{
    vpvl::Vector3 v;
    foreach (vpvl::Bone *selected, m_selected) {
        /* X と Y は MMD と挙動を合わせるため、値を反転させておく */
        switch (coordinate) {
        case 'x':
        case 'X':
            v.setValue(-value, 0, 0);
            break;
        case 'y':
        case 'Y':
            v.setValue(0, -value, 0);
            break;
        case 'z':
        case 'Z':
            v.setValue(0, 0, value);
            break;
        default:
            qFatal("Unexpected coordinate value: %c", coordinate);
        }
        translate(selected, v);
    }
}

void BoneMotionModel::translate(vpvl::Bone *bone, const vpvl::Vector3 &v)
{
    vpvl::Vector3 dest;
    switch (m_mode) {
    case kView: {
        const vpvl::Vector3 &v2 = m_sceneWidget->scene()->modelViewTransform() * v;
        dest = vpvl::Transform(bone->rotation(), bone->position()) * v2;
        //const QVector4D &r = modelviewMatrix() * QVector4D(v.x(), v.y(), v.z(), 0.0f);
        //dest = vpvl::Transform(bone->rotation(), bone->position()) * vpvl::Vector3(r.x(), r.y(), r.z());
        break;
    }
    case kLocal: {
        dest = vpvl::Transform(bone->rotation(), bone->position()) * v;
        break;
    }
    case kGlobal: {
        dest = bone->position() + v;
        break;
    }
    default:
        break;
    }
    bone->setPosition(dest);
    m_model->updateImmediate();
    emit bonePositionDidChange(bone, dest);
}

void BoneMotionModel::rotate(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    vpvl::Bone *selected = m_selected.last();
    const vpvl::Quaternion &current = selected->rotation();
    vpvl::Quaternion rot, dest;
    /* Z 軸は MMD と挙動を合わせるため、値を反転しておく */
    switch (coordinate) {
    case 'x':
    case 'X':
        rot.setEulerZYX(0, 0, value);
        break;
    case 'y':
    case 'Y':
        rot.setEulerZYX(0, value, 0);
        break;
    case 'z':
    case 'Z':
        rot.setEulerZYX(-value, 0, 0);
        break;
    default:
        qFatal("Unexpected coordinate value: %c", coordinate);
    }
    switch (m_mode) {
    case kView: {
        QVector4D r = modelviewMatrix() * QVector4D(rot.x(), rot.y(), rot.z(), rot.w());
        r.normalize();
        dest = current * vpvl::Quaternion(r.x(), r.y(), r.z(), r.w());
        break;
    }
    case kLocal: {
        dest = current * rot;
        break;
    }
    default:
        break;
    }
    selected->setRotation(dest);
    m_model->updateImmediate();
    emit boneRotationDidChange(selected, dest);
}

void BoneMotionModel::selectBones(const QList<vpvl::Bone *> &bones)
{
    m_selected = bones;
    emit bonesDidSelect(bones);
    int frameIndex = currentFrameIndex();
    /* ボーン名が存在かどうかをチェックするためだけのハッシュを作成しておく。int は飾り */
    QHash<QString, int> keys;
    foreach (vpvl::Bone *bone, bones)
        keys.insert(internal::toQString(bone), 0);
    QList<KeyFramePtr> frames;
    vpvl::BoneKeyFrame frame;
    /* モデルのデータを参照し、キーが存在するかつ現在のフレームインデックスと同じであるものを現在選択されているキーフレームのリストに追加する */
    foreach (const QVariant &variant, values()) {
        frame.read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
        if (keys.contains(internal::toQString(&frame)) && frameIndex == frame.frameIndex())
            frames.append(KeyFramePtr(static_cast<vpvl::BoneKeyFrame *>(frame.clone())));
    }
    emit boneFramesDidSelect(frames);
}

vpvl::Bone *BoneMotionModel::findBone(const QString &name)
{
    /* QString を扱っていること以外 PMDModel#findBone と同じ */
    const QByteArray &bytes = internal::getTextCodec()->fromUnicode(name);
    foreach (ITreeItem *item, keys()) {
        vpvl::Bone *bone = static_cast<TreeItem *>(item)->bone();
        if (!qstrcmp(reinterpret_cast<const char *>(bone->name()), bytes))
            return bone;
    }
    return 0;
}

const QMatrix4x4 BoneMotionModel::modelviewMatrix() const
{
    float modelviewf[16];
    qreal modelviewd[16];
    m_sceneWidget->scene()->getModelViewMatrix(modelviewf);
    for (int i = 0; i < 16; i++)
        modelviewd[i] = modelviewf[i];
    return QMatrix4x4(modelviewd);
}

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

#include "common/util.h"
#include "models/MorphMotionModel.h"
#include <vpvl/vpvl.h>

using namespace vpvl;

namespace
{

class TreeItem : public MotionBaseModel::ITreeItem
{
public:
    TreeItem(const QString &name, Face *face, bool isRoot, bool isCategory, TreeItem *parent)
        : m_name(name),
          m_parent(parent),
          m_face(face),
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
    Face *face() const {
        return m_face;
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
    Face *m_face;
    bool m_isRoot;
    bool m_isCategory;
};

class SetFramesCommand : public QUndoCommand
{
public:
    typedef QPair<QModelIndex, QByteArray> ModelIndex;

    SetFramesCommand(MorphMotionModel *bmm, const MorphMotionModel::KeyFramePairList &frames)
        : QUndoCommand(),
          m_fmm(bmm)
    {
        QHash<int, bool> indexProceeded;
        /* 現在選択中のモデルにある全ての頂点モーフを取り出す */
        const PMDMotionModel::TreeItemList &items = m_fmm->keys().values();
        /* フレームインデックスがまたがるので複雑だが対象のキーフレームを全て保存しておく */
        foreach (const MorphMotionModel::KeyFramePair &frame, frames) {
            int frameIndex = frame.first;
            /* フレーム単位での重複を避けるためにスキップ処理を設ける */
            if (!indexProceeded[frameIndex]) {
                /* モデルの全ての頂点モーフを対象にデータがあるか確認し、存在している場合のみボーンのキーフレームの生データを保存する */
                foreach (PMDMotionModel::ITreeItem *item, items) {
                    const QModelIndex &index = m_fmm->frameIndexToModelIndex(item, frameIndex);
                    const QVariant &data = index.data(MorphMotionModel::kBinaryDataRole);
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
        const PMDMotionModel::TreeItemList &items = m_fmm->keys().values();
        FaceAnimation *animation = m_fmm->currentMotion()->mutableFaceAnimation();
        foreach (int frameIndex, m_frameIndices) {
            animation->deleteKeyframes(frameIndex);
            foreach (PMDMotionModel::ITreeItem *item, items) {
                const QModelIndex &index = m_fmm->frameIndexToModelIndex(item, frameIndex);
                m_fmm->setData(index, QVariant());
            }
        }
        /* コンストラクタで保存したキーフレームの生データから頂点モーフのキーフレームに復元して置換する */
        foreach (const ModelIndex &index, m_modelIndices) {
            const QByteArray &bytes = index.second;
            m_fmm->setData(index.first, bytes, Qt::EditRole);
            FaceKeyframe *frame = new FaceKeyframe();
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->replaceKeyframe(frame);
        }
        /*
         * FaceAnimation の内部データの更新も忘れずに。モデルをリセットした上で、
         * モーションを更新するために PMD を現在のフレームに強制シークしておく
         */
        animation->refresh();
        m_fmm->refreshModel();
    }
    virtual void redo() {
        QString key;
        FaceAnimation *animation = m_fmm->currentMotion()->mutableFaceAnimation();
        const MorphMotionModel::Keys &keys = m_fmm->keys();
        Face *selected = m_fmm->selectedFace();
        /* すべてのキーフレーム情報を登録する */
        foreach (const MorphMotionModel::KeyFramePair &pair, m_frames) {
            int frameIndex = pair.first;
            MorphMotionModel::KeyFramePtr ptr = pair.second;
            FaceKeyframe *frame = ptr.data();
            /* キーフレームの対象頂点モーフ名を取得する */
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
            /* モデルに頂点モーフ名が存在するかを確認する */
            if (keys.contains(key)) {
                /*
                 * キーフレームをコピーし、対象のモデルのインデックスに移す。
                 * そしてモデルにデータを登録した上で現在登録されているキーフレームを置換する
                 * (前のキーフレームの情報が入ってる可能性があるので、それ故に重複が発生することを防ぐ)
                 */
                const QModelIndex &modelIndex = m_fmm->frameIndexToModelIndex(keys[key], frameIndex);
                QByteArray bytes(BoneKeyframe::strideSize(), '0');
                FaceKeyframe *newFrame = static_cast<FaceKeyframe *>(frame->clone());
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                animation->replaceKeyframe(newFrame);
                m_fmm->setData(modelIndex, bytes, Qt::EditRole);
            }
            else {
                qWarning("Tried registering not face key frame: %s", qPrintable(key));
                continue;
            }
        }
        /* SetFramesCommand#undo の通りのため、省略 */
        animation->refresh();
        m_fmm->refreshModel();
    }

private:
    QList<int> m_frameIndices;
    QList<ModelIndex> m_modelIndices;
    MorphMotionModel::KeyFramePairList m_frames;
    MorphMotionModel *m_fmm;
};

class ResetAllCommand : public QUndoCommand
{
public:
    ResetAllCommand(PMDModel *model)
        : QUndoCommand(),
          m_model(model)
    {
        /* 全ての頂点モーフの情報を保存しておく */
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
        m_model->resetAllFaces();
        m_model->updateImmediate();
    }

private:
    PMDModel *m_model;
    PMDModel::State *m_state;
};

class SetFaceCommand : public QUndoCommand
{
public:
    SetFaceCommand(PMDModel *model, PMDModel::State *state)
        : QUndoCommand(),
          m_model(model),
          m_newState(0),
          m_oldState(state)
    {
        /* 前と後の全ての頂点モーフの情報を保存しておく */
        m_newState = m_model->saveState();
    }
    virtual ~SetFaceCommand() {
        /* コンストラクタで saveState が呼ばれる前提なので両方解放しておく */
        m_model->discardState(m_newState);
        m_model->discardState(m_oldState);
    }

    void undo() {
        /* コンストラクタで呼ばれる前の頂点モーフ情報を復元し、シークせずにモデルを更新しておく */
        m_model->restoreState(m_oldState);
        m_model->updateImmediate();
    }
    void redo() {
        /* コンストラクタで呼ばれた時点の頂点モーフ情報を復元し、シークせずにモデルを更新しておく */
        m_model->restoreState(m_newState);
        m_model->updateImmediate();
    }

private:
    PMDModel *m_model;
    PMDModel::State *m_newState;
    PMDModel::State *m_oldState;
};

static Face *FaceFromModelIndex(const QModelIndex &index, PMDModel *model)
{
    /* QModelIndex -> TreeIndex -> ByteArray -> Face の順番で対象の頂点モーフを求めて選択状態にする作業 */
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    QByteArray bytes = internal::fromQString(item->name());
    return model->findFace(reinterpret_cast<const uint8_t *>(bytes.constData()));
}

}

MorphMotionModel::MorphMotionModel(QUndoGroup *undo, QObject *parent)
    : PMDMotionModel(undo, parent),
      m_state(0)
{
}

MorphMotionModel::~MorphMotionModel()
{
    m_frames.releaseAll();
}

void MorphMotionModel::saveMotion(VMDMotion *motion)
{
    if (m_model) {
        /* モデルの ByteArray を BoneKeyFrame に読ませて積んでおくだけの簡単な処理 */
        FaceAnimation *animation = motion->mutableFaceAnimation();
        foreach (QVariant value, values()) {
            FaceKeyframe *newFrame = new FaceKeyframe();
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

void MorphMotionModel::addKeyframesByModelIndices(const QModelIndexList &indices)
{
    KeyFramePairList faceFrames;
    PMDModel *model = selectedModel();
    /* モデルのインデックスを参照し、存在する頂点モーフに対して頂点モーフの現在の重み係数から頂点モーフのキーフレームにコピーする */
    foreach (const QModelIndex &index, indices) {
        int frameIndex = toFrameIndex(index);
        if (frameIndex >= 0) {
            const QByteArray &name = nameFromModelIndex(index);
            Face *face = model->findFace(reinterpret_cast<const uint8_t *>(name.constData()));
            if (face) {
                FaceKeyframe *frame = new FaceKeyframe();
                frame->setName(face->name());
                frame->setWeight(face->weight());
                faceFrames.append(KeyFramePair(frameIndex, KeyFramePtr(frame)));
            }
        }
    }
    setFrames(faceFrames);
}

void MorphMotionModel::copyKeyframes(int frameIndex)
{
    if (m_model && m_motion) {
        m_frames.releaseAll();
        foreach (PMDMotionModel::ITreeItem *item, keys().values()) {
            const QModelIndex &index = frameIndexToModelIndex(item, frameIndex);
            QVariant variant = index.data(kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                QByteArray bytes = variant.toByteArray();
                FaceKeyframe *frame = new FaceKeyframe();
                frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
                m_frames.add(frame);
            }
        }
    }
}

void MorphMotionModel::pasteKeyframes(int frameIndex)
{
    if (m_model && m_motion && m_frames.count() != 0) {
        MorphMotionModel::KeyFramePairList frames;
        const int nframes = m_frames.count();
        for (int i = 0; i < nframes; i++) {
            FaceKeyframe *frame = static_cast<FaceKeyframe *>(m_frames[i]->clone());
            frames.append(KeyFramePair(frameIndex, KeyFramePtr(frame)));
        }
        addUndoCommand(new SetFramesCommand(this, frames));
    }
}

void MorphMotionModel::saveTransform()
{
    if (m_model) {
        /* モデルの状態を保存しておく。メモリリーク防止のため、前の状態は破棄しておく */
        m_model->discardState(m_state);
        m_state = m_model->saveState();
    }
}

void MorphMotionModel::commitTransform()
{
    if (m_model && m_state) {
        /*
         * startTransform で保存したモデルの状態を SetBoneCommand に渡す
         * メモリ管理はそちらに移動するので m_state は 0 にして無効にしておく
         */
        addUndoCommand(new SetFaceCommand(m_model, m_state));
        m_state = 0;
    }
}

void MorphMotionModel::selectKeyframesByModelIndices(const QModelIndexList &indices)
{
    if (m_model) {
        QList<Face *> faces;
        foreach (const QModelIndex &index, indices) {
            if (index.isValid()) {
                Face *face = FaceFromModelIndex(index, m_model);
                if (face)
                    faces.append(face);
            }
        }
        if (!faces.isEmpty())
            selectFaces(faces);
    }
}

const QByteArray MorphMotionModel::nameFromModelIndex(const QModelIndex &index) const
{
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    return internal::fromQString(item->name());
}

void MorphMotionModel::setFrames(const KeyFramePairList &frames)
{
    if (m_model && m_motion) {
        addUndoCommand(new SetFramesCommand(this, frames));
    }
    else {
        qWarning("No model or motion to register face frames.");
        return;
    }
}

void MorphMotionModel::resetAllFaces()
{
    if (m_model)
        addUndoCommand(new ResetAllCommand(m_model));
}

void MorphMotionModel::setPMDModel(PMDModel *model)
{
    /* 引数のモデルが現在選択中のものであれば二重処理になってしまうので、スキップする */
    if (m_model == model)
        return;
    if (model) {
        /* PMD の二重登録防止 */
        if (!hasPMDModel(model)) {
            /* ルートを作成 */
            RootPtr ptr(new TreeItem("", 0, true, false, 0));
            /* 予め決められたカテゴリのアイテムを作成する */
            TreeItem *r = static_cast<TreeItem *>(ptr.data());
            TreeItem *eyeblow = new TreeItem(tr("Eyeblow"), 0, false, true, static_cast<TreeItem *>(r));
            TreeItem *eye = new TreeItem(tr("Eye"), 0, false, true, static_cast<TreeItem *>(r));
            TreeItem *lip = new TreeItem(tr("Lip"), 0, false, true, static_cast<TreeItem *>(r));
            TreeItem *other = new TreeItem(tr("Other"), 0, false, true, static_cast<TreeItem *>(r));
            const FaceList &faces = model->facesForUI();
            const int nfaces = faces.count();
            Keys keys;
            for (int i = 0; i < nfaces; i++) {
                Face *face = faces[i];
                const QString &name = internal::toQString(face);
                TreeItem *child, *parent = 0;
                /* カテゴリ毎に頂点モーフを追加して整理する */
                switch (face->type()) {
                case Face::kEyeblow:
                    parent = eyeblow;
                    break;
                case Face::kEye:
                    parent = eye;
                    break;
                case Face::kLip:
                    parent = lip;
                    break;
                case Face::kOther:
                    parent = other;
                    break;
                default:
                    break;
                }
                /* 何も属さない Base を除いてカテゴリアイテムに追加する */
                if (parent) {
                    child = new TreeItem(name, face, false, false, parent);
                    parent->addChild(child);
                    keys.insert(name, child);
                }
            }
            /* カテゴリアイテムをルートに追加する */
            r->addChild(eyeblow);
            r->addChild(eye);
            r->addChild(lip);
            r->addChild(other);
            addPMDModel(model, ptr, keys);
        }
        else {
            addPMDModel(model, rootPtr(model), Keys());
        }
        m_model = model;
        emit modelDidChange(model);
        qDebug("Set a model in MorphMotionModel: %s", qPrintable(internal::toQString(model)));
    }
    else {
        m_model = 0;
        emit modelDidChange(0);
    }
    /* テーブルモデルを更新 */
    reset();
}

void MorphMotionModel::loadMotion(VMDMotion *motion, PMDModel *model)
{
    /* 現在のモデルが対象のモデルと一致していることを確認しておく */
    if (model == m_model) {
        const FaceAnimation &animation = motion->faceAnimation();
        const int nFaceFrames = animation.countKeyframes();
        /* モーションのすべてのキーフレームを参照し、モデルの頂点モーフ名に存在するものだけ登録する */
        for (int i = 0; i < nFaceFrames; i++) {
            FaceKeyframe *frame = animation.frameAt(i);
            const uint8_t *name = frame->name();
            const QString &key = internal::toQString(name);
            const Keys &keys = this->keys();
            if (keys.contains(key)) {
                int frameIndex = static_cast<int>(frame->frameIndex());
                QByteArray bytes(BoneKeyframe::strideSize(), '0');
                ITreeItem *item = keys[key];
                /* この時点で新しい QModelIndex が作成される */
                const QModelIndex &modelIndex = frameIndexToModelIndex(item, frameIndex);
                FaceKeyframe newFrame;
                newFrame.setName(name);
                newFrame.setWeight(frame->weight());
                newFrame.setFrameIndex(frameIndex);
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes);
            }
        }
        /* 読み込まれたモーションを現在のモーションとして登録する。あとは SetFramesCommand#undo と同じ */
        m_motion = motion;
        refreshModel();
        setModified(false);
        qDebug("Loaded a motion to the model in MorphMotionModel: %s", qPrintable(internal::toQString(model)));
    }
    else {
        qDebug("Tried loading a motion to different model, ignored: %s", qPrintable(internal::toQString(model)));
    }
}

void MorphMotionModel::removeMotion()
{
    /* 選択された頂点モーフとモデルに登録されているデータが削除される。頂点モーフ名は削除されない */
    m_selected.clear();
    setModified(false);
    removePMDMotion(m_model);
    reset();
}

void MorphMotionModel::removeModel()
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

void MorphMotionModel::deleteKeyframesByModelIndices(const QModelIndexList &indices)
{
    FaceAnimation *animation = m_motion->mutableFaceAnimation();
    foreach (const QModelIndex &index, indices) {
        if (index.isValid() && index.column() > 1) {
            /* QModelIndex にある頂点モーフとフレームインデックスからキーフレームを削除する */
            TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
            if (Face *face = item->face())
                animation->deleteKeyframe(toFrameIndex(index), face->name());
            setData(index, QVariant());
        }
    }
}

void MorphMotionModel::applyKeyframeWeightByModelIndices(const QModelIndexList &indices, float value)
{
    KeyFramePairList keyframes;
    foreach (const QModelIndex &index, indices) {
        if (index.isValid()) {
            /* QModelIndex にある頂点モーフとフレームインデックスからキーフレームを取得し、値を補正する */
            TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
            if (Face *face = item->face()) {
                FaceKeyframe *keyframe = new FaceKeyframe();
                keyframe->setName(face->name());
                keyframe->setWeight(face->weight() * value);
                keyframes.append(KeyFramePair(toFrameIndex(index), KeyFramePtr(keyframe)));
            }
        }
    }
    setFrames(keyframes);
}

void MorphMotionModel::selectFaces(const QList<Face *> &faces)
{
    m_selected = faces;
    emit facesDidSelect(faces);
}

Face *MorphMotionModel::findFace(const QString &name)
{
    /* QString を扱っていること以外 PMDModel#findFace と同じ */
    const QByteArray &bytes = internal::getTextCodec()->fromUnicode(name);
    foreach (ITreeItem *item, keys()) {
        Face *face = static_cast<TreeItem *>(item)->face();
        if (!qstrcmp(reinterpret_cast<const char *>(face->name()), bytes))
            return face;
    }
    return 0;
}

void MorphMotionModel::setWeight(float value)
{
    if (!m_selected.isEmpty())
        setWeight(value, m_selected.last());
}

void MorphMotionModel::setWeight(float value, Face *face)
{
    if (face) {
        face->setWeight(value);
        m_model->updateImmediate();
    }
}

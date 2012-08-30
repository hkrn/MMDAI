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

#include <vpvl2/vpvl2.h>
#include <vpvl2/qt/CString.h>

using namespace vpvl2;
using namespace vpvl2::qt;

namespace
{

class TreeItem : public MotionBaseModel::ITreeItem
{
public:
    TreeItem(const QString &name, IMorph *morph, bool isRoot, bool isCategory, TreeItem *parent)
        : m_name(name),
          m_parent(parent),
          m_morph(morph),
          m_isRoot(isRoot),
          m_isCategory(isCategory)
    {
    }
    ~TreeItem() {
        qDeleteAll(m_children);
        m_parent = 0;
        m_morph = 0;
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
    IMorph *morph() const {
        return m_morph;
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
    IMorph *m_morph;
    bool m_isRoot;
    bool m_isCategory;
};

class SetKeyframesCommand : public QUndoCommand
{
public:
    typedef QPair<QModelIndex, QByteArray> ModelIndex;

    /*
     * MorphMotionModel で selectedModel/currentMotion に変化があるとまずいのでポインタを保存しておく
     * モデルまたモーションを削除すると このコマンドのインスタンスを格納した UndoStack も一緒に削除される
     */
    SetKeyframesCommand(MorphMotionModel *fmm, const MorphMotionModel::KeyFramePairList &keyframes)
        : QUndoCommand(),
          m_keys(fmm->keys()),
          m_fmm(fmm),
          m_model(fmm->selectedModel()),
          m_motion(fmm->currentMotion()),
          m_morph(fmm->selectedMorph())
    {
        QSet<int> indexProceeded;
        /* 現在選択中のモデルにある全ての頂点モーフを取り出す */
        const PMDMotionModel::TreeItemList &items = m_keys.values();
        /* フレームインデックスがまたがるので複雑だが対象のキーフレームを全て保存しておく */
        foreach (const MorphMotionModel::KeyFramePair &frame, keyframes) {
            int frameIndex = frame.first;
            /* フレーム単位での重複を避けるためにスキップ処理を設ける */
            if (!indexProceeded.contains(frameIndex)) {
                /* モデルの全ての頂点モーフを対象にデータがあるか確認し、存在している場合のみボーンのキーフレームの生データを保存する */
                foreach (PMDMotionModel::ITreeItem *item, items) {
                    const QModelIndex &index = m_fmm->frameIndexToModelIndex(item, frameIndex);
                    const QVariant &data = index.data(MorphMotionModel::kBinaryDataRole);
                    if (data.canConvert(QVariant::ByteArray))
                        m_modelIndices.append(ModelIndex(index, data.toByteArray()));
                }
                indexProceeded.insert(frameIndex);
            }
        }
        m_keyframes = keyframes;
        m_frameIndices = indexProceeded.toList();
        setText(QApplication::tr("Register morph keyframes of %1").arg(internal::toQStringFromModel(m_model)));
    }
    ~SetKeyframesCommand() {
    }

    void undo() {
        /* 対象のキーフレームのインデックスを全て削除、さらにモデルのデータも削除 */
        const PMDMotionModel::TreeItemList &items = m_keys.values();
        Array<IKeyframe *> keyframes;
        foreach (int frameIndex, m_frameIndices) {
            keyframes.clear();
            m_motion->getKeyframes(frameIndex, 0, IKeyframe::kMorph, keyframes);
            const int nkeyframes = keyframes.count();
            for (int i = 0; i < nkeyframes; i++) {
                IKeyframe *keyframe = keyframes[i];
                m_motion->deleteKeyframe(keyframe);
            }
            foreach (PMDMotionModel::ITreeItem *item, items) {
                const QModelIndex &index = m_fmm->frameIndexToModelIndex(item, frameIndex);
                m_fmm->setData(index, QVariant());
            }
        }
        /* 削除後のインデックス更新を忘れなく行う */
        m_motion->update(IKeyframe::kMorph);
        /* コンストラクタで保存したキーフレームの生データから頂点モーフのキーフレームに復元して置換する */
        Factory *factory = m_fmm->factory();
        QScopedPointer<IMorphKeyframe> frame;
        foreach (const ModelIndex &index, m_modelIndices) {
            const QByteArray &bytes = index.second;
            m_fmm->setData(index.first, bytes, Qt::EditRole);
            frame.reset(factory->createMorphKeyframe(m_motion));
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            m_motion->addKeyframe(frame.take());
        }
        /* addKeyframe によって変更が必要になる内部インデックスの更新を行うため、update をかけておく */
        m_motion->update(IKeyframe::kMorph);
        m_fmm->refreshModel(m_model);
    }
    virtual void redo() {
        QScopedPointer<IMorphKeyframe> newMorphKeyframe;
        QString key;
        /* すべてのキーフレーム情報を登録する */
        foreach (const MorphMotionModel::KeyFramePair &pair, m_keyframes) {
            int frameIndex = pair.first;
            const MorphMotionModel::KeyFramePtr &ptr = pair.second;
            IMorphKeyframe *morphKeyframe = ptr.data();
            /* キーフレームの対象頂点モーフ名を取得する */
            if (morphKeyframe) {
                key = internal::toQStringFromMorphKeyframe(morphKeyframe);
            }
            else if (m_morph) {
                key = internal::toQStringFromMorph(m_morph);
            }
            else {
                qWarning("No bone is selected or null");
                continue;
            }
            /* モデルに頂点モーフ名が存在するかを確認する */
            if (m_keys.contains(key)) {
                /*
                 * キーフレームをコピーし、モデルにデータを登録した上で現在登録されているキーフレームを置換する
                 * (前のキーフレームの情報が入ってる可能性があるので、置換することで重複が発生することを防ぐ)
                 *
                 * ※ 置換の現実装は find => delete => add なので find の探索コストがネックになるため多いと時間がかかる
                 */
                const QModelIndex &modelIndex = m_fmm->frameIndexToModelIndex(m_keys[key], frameIndex);
                if (morphKeyframe->timeIndex() >= 0) {
                    QByteArray bytes(morphKeyframe->estimateSize(), '0');
                    newMorphKeyframe.reset(morphKeyframe->clone());
                    newMorphKeyframe->setTimeIndex(frameIndex);
                    newMorphKeyframe->write(reinterpret_cast<uint8_t *>(bytes.data()));
                    m_motion->replaceKeyframe(newMorphKeyframe.take());
                    m_fmm->setData(modelIndex, bytes, Qt::EditRole);
                }
                else {
                    /* 元フレームのインデックスが 0 未満の時は削除 */
                    IKeyframe *frameToDelete = m_motion->findMorphKeyframe(frameIndex, morphKeyframe->name(), 0);
                    m_motion->deleteKeyframe(frameToDelete);
                    m_fmm->setData(modelIndex, QVariant());
                }
            }
            else {
                qWarning("Tried registering not morph key frame: %s", qPrintable(key));
                continue;
            }
        }
        /* SetFramesCommand#undo の通りのため、省略 */
        m_motion->update(IKeyframe::kMorph);
        m_fmm->refreshModel(m_model);
    }

private:
    const MorphMotionModel::Keys m_keys;
    /* undo で復元する対象のキーフレームの番号 */
    QList<int> m_frameIndices;
    /* m_frameIndices に加えて undo で復元する用のキーフレームの集合 */
    QList<ModelIndex> m_modelIndices;
    /* 実際に登録する用のキーフレームの集合 */
    MorphMotionModel::KeyFramePairList m_keyframes;
    MorphMotionModel *m_fmm;
    IModel *m_model;
    IMotion *m_motion;
    IMorph *m_morph;
};

class ResetAllCommand : public QUndoCommand
{
public:
    ResetAllCommand(const vpvl2::Scene *scene, IModel *model)
        : QUndoCommand(),
          m_state(scene, model)
    {
        /* 全ての頂点モーフの情報を保存しておく */
        m_state.save();
        setText(QApplication::tr("Reset all morphs of %1").arg(internal::toQStringFromModel(model)));
    }

    void undo() {
        /* コンストラクタで保存したボーン情報を復元し、シークせずにモデルを更新しておく */
        m_state.restore();
    }
    void redo() {
        /* 全てのボーンをリセットし、シークせずにモデルを更新しておく */
        m_state.resetMorphs();
    }

private:
    PMDMotionModel::State m_state;
};

class SetMorphCommand : public QUndoCommand
{
public:
    SetMorphCommand(const vpvl2::Scene *scene, IModel *model, const PMDMotionModel::State &oldState)
        : QUndoCommand(),
          m_oldState(scene, 0),
          m_newState(scene, model)
    {
        m_oldState.copyFrom(oldState);
        /* 前と後の全ての頂点モーフの情報を保存しておく */
        m_newState.save();
        setText(QApplication::tr("Set morphs of %1").arg(internal::toQStringFromModel(model)));
    }
    virtual ~SetMorphCommand() {
    }

    void undo() {
        /* コンストラクタで呼ばれる前の頂点モーフ情報を復元し、シークせずにモデルを更新しておく */
        m_oldState.restore();
    }
    void redo() {
        /* コンストラクタで呼ばれた時点の頂点モーフ情報を復元し、シークせずにモデルを更新しておく */
        m_newState.restore();
    }

private:
    IModel *m_model;
    PMDMotionModel::State m_oldState;
    PMDMotionModel::State m_newState;
};

static IMorph *UIMorphFromModelIndex(const QModelIndex &index, IModel *model)
{
    /* QModelIndex -> TreeIndex -> ByteArray -> Face の順番で対象の頂点モーフを求めて選択状態にする作業 */
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    const CString s(item->name());
    return model->findMorph(&s);
}

}

MorphMotionModel::MorphMotionModel(Factory *factory, QUndoGroup *undo, QObject *parent)
    : PMDMotionModel(undo, parent),
      m_factory(factory),
      m_state(0, 0)
{
}

MorphMotionModel::~MorphMotionModel()
{
}

void MorphMotionModel::saveMotion(IMotion *motion)
{
    if (m_model) {
        /* モデルの ByteArray を BoneKeyFrame に読ませて積んでおくだけの簡単な処理 */
        foreach (QVariant value, values()) {
            IMorphKeyframe *newFrame = m_factory->createMorphKeyframe(motion);
            const QByteArray &bytes = value.toByteArray();
            newFrame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            motion->addKeyframe(newFrame);
        }
        setModified(false);
    }
    else {
        qWarning("No model is selected to save motion.");
    }
}

void MorphMotionModel::addKeyframesByModelIndices(const QModelIndexList &indices)
{
    KeyFramePairList keyframes;
    IModel *model = selectedModel();
    /* モデルのインデックスを参照し、存在する頂点モーフに対して頂点モーフの現在の重み係数から頂点モーフのキーフレームにコピーする */
    foreach (const QModelIndex &index, indices) {
        int frameIndex = toTimeIndex(index);
        if (frameIndex >= 0) {
            const QString &name = nameFromModelIndex(index);
            CString s(name);
            IMorph *morph = model->findMorph(&s);
            if (morph) {
                KeyFramePtr keyframe(m_factory->createMorphKeyframe(m_motion));
                keyframe->setName(morph->name());
                keyframe->setWeight(morph->weight());
                keyframes.append(KeyFramePair(frameIndex, keyframe));
            }
        }
    }
    setKeyframes(keyframes);
}

void MorphMotionModel::copyKeyframesByModelIndices(const QModelIndexList &indices, int frameIndex)
{
    if (m_model && m_motion) {
        /* 前回呼ばれた copyFrames で作成したデータを破棄しておく */
        m_copiedKeyframes.clear();
        foreach (const QModelIndex &index, indices) {
            const QVariant &variant = index.data(kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                const QByteArray &bytes = variant.toByteArray();
                KeyFramePtr keyframe(m_factory->createMorphKeyframe(m_motion));
                keyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
                /* 予め差分をとっておき、pasteKeyframes でペースト先の差分をたすようにする */
                int diff = keyframe->timeIndex() - frameIndex;
                m_copiedKeyframes.append(KeyFramePair(diff, keyframe));
            }
        }
    }
}

void MorphMotionModel::pasteKeyframesByTimeIndex(int frameIndex)
{
    if (m_model && m_motion && !m_copiedKeyframes.isEmpty()) {
        MorphMotionModel::KeyFramePairList keyframes;
        foreach (const KeyFramePair &pair, m_copiedKeyframes) {
            KeyFramePtr keyframe(static_cast<IMorphKeyframe *>(pair.second->clone()));
            /* コピー先にフレームインデックスを更新する */
            int newFrameIndex = frameIndex + pair.first;
            keyframes.append(KeyFramePair(newFrameIndex, keyframe));
        }
        addUndoCommand(new SetKeyframesCommand(this, keyframes));
    }
}

void MorphMotionModel::saveTransform()
{
    /* モデルの状態を保存しておく */
    if (m_model) {
        m_state.setModel(m_model);
        m_state.save();
    }
}

void MorphMotionModel::commitTransform()
{
    /* 状態を圧縮し、モーフ変形があれば SetMorphCommand を作成して UndoStack に追加 */
    if (m_model && m_state.compact())
        addUndoCommand(new SetMorphCommand(m_scene, m_model, m_state));
}

void MorphMotionModel::selectKeyframesByModelIndices(const QModelIndexList &indices)
{
    if (m_model) {
        QList<IMorph *> morphs;
        foreach (const QModelIndex &index, indices) {
            if (index.isValid()) {
                IMorph *morph = UIMorphFromModelIndex(index, m_model);
                if (morph)
                    morphs.append(morph);
            }
        }
        if (!morphs.isEmpty())
            selectMorphs(morphs);
    }
}

const QString MorphMotionModel::nameFromModelIndex(const QModelIndex &index) const
{
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    return item->name();
}

void MorphMotionModel::setKeyframes(const KeyFramePairList &keyframes)
{
    if (m_model && m_motion) {
        addUndoCommand(new SetKeyframesCommand(this, keyframes));
    }
    else {
        qWarning("No model or motion to register morph frames.");
        return;
    }
}

void MorphMotionModel::resetAllMorphs()
{
    if (m_model)
        addUndoCommand(new ResetAllCommand(m_scene, m_model));
}

void MorphMotionModel::setPMDModel(IModel *model)
{
    /* 引数のモデルが現在選択中のものであれば二重処理になってしまうので、スキップする */
    if (m_model == model)
        return;
    if (model) {
        /* PMD の二重登録防止 */
        if (!hasPMDModel(model)) {
            /* ルートを作成 */
            RootPtr rootItemPtr(new TreeItem("", 0, true, false, 0));
            TreeItem *rootItem = static_cast<TreeItem *>(rootItemPtr.data());
            /* 予め決められたカテゴリのアイテムを作成する */
            QScopedPointer<TreeItem> eyeblow(new TreeItem(tr("Eyeblow"), 0, false, true, rootItem));
            QScopedPointer<TreeItem> eye(new TreeItem(tr("Eye"), 0, false, true, rootItem));
            QScopedPointer<TreeItem> lip(new TreeItem(tr("Lip"), 0, false, true, rootItem));
            QScopedPointer<TreeItem> other(new TreeItem(tr("Other"), 0, false, true, rootItem));
            Array<IMorph *> morphs;
            model->getMorphs(morphs);
            const int nmorphs = morphs.count();
            Keys keys;
            for (int i = 0; i < nmorphs; i++) {
                IMorph *morph = morphs[i];
                const QString &name = internal::toQStringFromMorph(morph);
                TreeItem *child, *parent = 0;
                /* カテゴリ毎に頂点モーフを追加して整理する */
                switch (morph->category()) {
                case IMorph::kEyeblow:
                    parent = eyeblow.data();
                    break;
                case IMorph::kEye:
                    parent = eye.data();
                    break;
                case IMorph::kLip:
                    parent = lip.data();
                    break;
                case IMorph::kOther:
                    parent = other.data();
                    break;
                default:
                    break;
                }
                /* 何も属さない Base を除いてカテゴリアイテムに追加する */
                if (parent) {
                    child = new TreeItem(name, morph, false, false, parent);
                    parent->addChild(child);
                    keys.insert(name, child);
                }
            }
            /* カテゴリアイテムをルートに追加する */
            rootItemPtr->addChild(eyeblow.take());
            rootItemPtr->addChild(eye.take());
            rootItemPtr->addChild(lip.take());
            rootItemPtr->addChild(other.take());
            addPMDModel(model, rootItemPtr, keys);
        }
        else {
            addPMDModel(model, rootPtr(model), Keys());
        }
        m_model = model;
        emit modelDidChange(model);
        qDebug("Set a model in MorphMotionModel: %s", qPrintable(internal::toQStringFromModel(model)));
    }
    else {
        m_model = 0;
        emit modelDidChange(0);
    }
    /* テーブルモデルを更新 */
    setFrameIndexColumnMax(0);
    reset();
}

void MorphMotionModel::loadMotion(IMotion *motion, IModel *model)
{
    /* 現在のモデルが対象のモデルと一致していることを確認しておく */
    if (model == m_model) {
        const int nkeyframes = motion->countKeyframes(IKeyframe::kMorph);
        /* フレーム列の最大数をモーションのフレーム数に更新する */
        setFrameIndexColumnMax(motion);
        reset();
        /* モーションのすべてのキーフレームを参照し、モデルの頂点モーフ名に存在するものだけ登録する */
        for (int i = 0; i < nkeyframes; i++) {
            IMorphKeyframe *keyframe = motion->findMorphKeyframeAt(i);
            const QString &key = internal::toQStringFromMorphKeyframe(keyframe);
            const Keys &keys = this->keys();
            if (keys.contains(key)) {
                int frameIndex = static_cast<int>(keyframe->timeIndex());
                QByteArray bytes(keyframe->estimateSize(), '0');
                ITreeItem *item = keys[key];
                /* この時点で新しい QModelIndex が作成される */
                const QModelIndex &modelIndex = frameIndexToModelIndex(item, frameIndex);
                IMorphKeyframe *newFrame = m_factory->createMorphKeyframe(motion);
                newFrame->setName(keyframe->name());
                newFrame->setWeight(keyframe->weight());
                newFrame->setTimeIndex(frameIndex);
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes);
            }
        }
        /* 読み込まれたモーションを現在のモーションとして登録する。あとは SetFramesCommand#undo と同じ */
        m_motion = motion;
        refreshModel(m_model);
        setModified(false);
        qDebug("Loaded a motion to the model in MorphMotionModel: %s", qPrintable(internal::toQStringFromModel(model)));
    }
    else {
        qDebug("Tried loading a motion to different model, ignored: %s", qPrintable(internal::toQStringFromModel(model)));
    }
}

void MorphMotionModel::removeMotion()
{
    /* コピーしたキーフレーム、選択された頂点モーフとモデルに登録されているデータが削除される。頂点モーフ名は削除されない */
    m_selectedMorphs.clear();
    m_copiedKeyframes.clear();
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
    KeyFramePairList keyframes;
    /* ここでは削除するキーフレームを決定するのみ。実際に削除するのは SetFramesCommand である点に注意 */
    foreach (const QModelIndex &index, indices) {
        if (index.isValid() && index.column() > 1) {
            TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
            if (IMorph *morph = item->morph()) {
                IMorphKeyframe *keyframeToDelete = m_motion->findMorphKeyframe(toTimeIndex(index), morph->name(), 0);
                if (keyframeToDelete) {
                    KeyFramePtr clonedKeyframe(keyframeToDelete->clone());
                    /* SetFramesCommand で削除するので削除に必要な条件である frameIndex を 0 未満の値にしておく */
                    clonedKeyframe->setTimeIndex(-1);
                    keyframes.append(KeyFramePair(keyframeToDelete->timeIndex(), clonedKeyframe));
                }
            }
        }
    }
    if (!keyframes.isEmpty())
        addUndoCommand(new SetKeyframesCommand(this, keyframes));
}

void MorphMotionModel::applyKeyframeWeightByModelIndices(const QModelIndexList &indices, float value)
{
    KeyFramePairList keyframes;
    foreach (const QModelIndex &index, indices) {
        if (index.isValid()) {
            /* QModelIndex からキーフレームを取得し、その中に入っている値を補正する */
            const QVariant &variant = index.data(kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                const QByteArray &bytes = variant.toByteArray();
                KeyFramePtr keyframe(m_factory->createMorphKeyframe(m_motion));
                keyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
                keyframe->setWeight(keyframe->weight() * value);
                keyframes.append(KeyFramePair(toTimeIndex(index), keyframe));
            }
        }
    }
    setKeyframes(keyframes);
}

void MorphMotionModel::selectMorphs(const QList<IMorph *> &morphs)
{
    m_selectedMorphs = morphs;
    emit morphsDidSelect(morphs);
}

void MorphMotionModel::setWeight(IMorph::WeightPrecision &value)
{
    if (!m_selectedMorphs.isEmpty())
        setWeight(value, m_selectedMorphs.last());
}

void MorphMotionModel::setWeight(const IMorph::WeightPrecision &value, IMorph *morph)
{
    if (morph) {
        /* 一度頂点がリセットされるので、頂点モーフのみ更新を行う */
        m_model->resetVertices();
        Array<IMorph *> morphs;
        m_model->getMorphs(morphs);
        const int nmorphs = morphs.count();
        for (int i = 0; i < nmorphs; i++) {
            IMorph *m = morphs[i];
            /* 頂点モーフでかつ変更するモーフ以外を更新するようにする */
            if (morph != m && morph->type() == IMorph::kVertex)
                m->setWeight(m->weight());
        }
        morph->setWeight(value);
        m_scene->updateModel(m_model);
    }
}

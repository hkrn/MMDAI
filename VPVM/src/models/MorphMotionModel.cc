/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "MorphMotionModel.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/qt/Util.h>

#include <QApplication> /* for tr */

/* lupdate cannot parse tr() syntax correctly */

using namespace vpvm;
using namespace vpvl2;
using namespace vpvl2::qt;

namespace
{

class TreeItem : public MotionBaseModel::ITreeItem
{
public:
    TreeItem(const QString &name, IMorph *morph, bool isRoot, bool isCategory, TreeItem *parent)
        : m_name(name),
          m_parentRef(parent),
          m_morphRef(morph),
          m_isRoot(isRoot),
          m_isCategory(isCategory)
    {
    }
    ~TreeItem() {
        qDeleteAll(m_children);
        m_parentRef = 0;
        m_morphRef = 0;
        m_isRoot = false;
        m_isCategory = false;
    }

    void addChild(ITreeItem *item) {
        m_children.append(static_cast<TreeItem *>(item));
    }
    ITreeItem *parent() const {
        return m_parentRef;
    }
    ITreeItem *child(int row) const {
        return m_children.value(row);
    }
    const QString &name() const {
        return m_name;
    }
    IMorph *morph() const {
        return m_morphRef;
    }
    bool isRoot() const {
        return m_isRoot;
    }
    bool isCategory() const {
        return m_isCategory;
    }
    int rowIndex() const {
        /* 自分が親行に対して何番目にあるかを求める */
        return m_parentRef ? m_parentRef->m_children.indexOf(const_cast<TreeItem *>(this)) : 0;
    }
    int countChildren() const {
        return m_children.count();
    }

private:
    QList<TreeItem *> m_children;
    QString m_name;
    TreeItem *m_parentRef;
    IMorph *m_morphRef;
    bool m_isRoot;
    bool m_isCategory;
};

typedef QPair<QModelIndex, QByteArray> ModelIndex;

class LoadPoseCommand : public QUndoCommand
{
public:
    /*
     * MorphMotionModel で selectedModel/currentMotion に変化があるとまずいのでポインタを保存しておく
     * モデルまたモーションを削除すると このコマンドのインスタンスを格納した UndoStack も一緒に削除される
     */
    LoadPoseCommand(MorphMotionModel *mmm, PosePtr pose, int timeIndex)
        : QUndoCommand(),
          m_keys(mmm->keys()),
          m_pose(pose->clone()),
          m_mmmRef(mmm),
          m_modelRef(mmm->selectedModel()),
          m_motionRef(mmm->currentMotionRef()),
          m_timeIndex(timeIndex)
    {
        /* 現在のフレームにある全てのボーンのキーフレーム情報を参照する。キーフレームがあれば undo のために保存しておく */
        foreach (PMDMotionModel::ITreeItem *item, m_keys.values()) {
            const QModelIndex &index = m_mmmRef->timeIndexToModelIndex(item, timeIndex);
            const QVariant &data = index.data(MorphMotionModel::kBinaryDataRole);
            if (data.canConvert(QVariant::ByteArray)) {
                m_modelIndices.append(ModelIndex(index, data.toByteArray()));
            }
        }
        setText(QApplication::tr("Load a pose to %1").arg(timeIndex));
    }
    virtual ~LoadPoseCommand() {
    }

    virtual void undo() {
        /* 現在のフレームを削除しておき、さらに全てのボーンのモデルのデータを空にしておく(=削除) */
        Array<IKeyframe *> keyframes;
        m_motionRef->getKeyframes(m_timeIndex, 0, IKeyframe::kMorphKeyframe, keyframes);
        const int nkeyframes = keyframes.count();
        for (int i = 0; i < nkeyframes; i++) {
            IKeyframe *keyframe = keyframes[i];
            m_motionRef->deleteKeyframe(keyframe);
        }
        foreach (PMDMotionModel::ITreeItem *item, m_keys.values()) {
            const QModelIndex &index = m_mmmRef->timeIndexToModelIndex(item, m_timeIndex);
            m_mmmRef->setData(index, QVariant());
        }
        /* 削除後のインデックス更新を忘れなく行う */
        m_motionRef->update(IKeyframe::kMorphKeyframe);
        /*
         * コンストラクタで保存したモーフ情報を復元して置換する。注意点として replaceKeyFrame でメモリの所有者が
         * MorphAnimation に移動するのでこちらで管理する必要がなくなる
         */
        QScopedPointer<IMorphKeyframe> morphKeyframe;
        Factory *factory = m_mmmRef->factoryRef();
        foreach (const ModelIndex &index, m_modelIndices) {
            const QByteArray &bytes = index.second;
            m_mmmRef->setData(index.first, bytes, Qt::EditRole);
            morphKeyframe.reset(factory->createMorphKeyframe(m_motionRef.data()));
            morphKeyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            m_motionRef->replaceKeyframe(morphKeyframe.take());
        }
        /*
         * replaceKeyframe (内部的には addKeyframe を呼んでいる) によって変更が必要になる
         * 内部インデックスの更新を行うため、update をかけておく
         */
        m_motionRef->update(IKeyframe::kMorphKeyframe);
        m_mmmRef->refreshModel(m_modelRef);
    }
    virtual void redo() {
        QScopedPointer<IMorphKeyframe> newMorphKeyframe;
        /* ポーズにあるモーフ情報を参照する */
        Factory *factory = m_mmmRef->factoryRef();
        Array<const Pose::Morph *> morphs;
        m_pose->getMorphs(morphs);
        const int nmorphs = morphs.count();
        for (int i = 0; i < nmorphs; i++) {
            const Pose::Morph *morph = morphs[i];
            const QString &key = Util::toQString(static_cast<const String *>(morph->name())->value());
            /* ポーズにあるボーンがモデルの方に実在する */
            if (m_keys.contains(key)) {
                /*
                 * ポーズにあるボーン情報を元にキーフレームを作成し、モデルに登録した上で現在登録されているキーフレームを置換する
                 * replaceKeyFrame でメモリの所有者が BoneAnimation に移動する点は同じ
                 */
                const QModelIndex &modelIndex = m_mmmRef->timeIndexToModelIndex(m_keys[key], m_timeIndex);
                String s(Util::fromQString(key));
                newMorphKeyframe.reset(factory->createMorphKeyframe(m_motionRef.data()));
                newMorphKeyframe->setName(&s);
                newMorphKeyframe->setWeight(morph->weight());
                newMorphKeyframe->setTimeIndex(m_timeIndex);
                QByteArray bytes(newMorphKeyframe->estimateSize(), '0');
                newMorphKeyframe->write(reinterpret_cast<uint8_t *>(bytes.data()));
                m_mmmRef->setData(modelIndex, bytes, Qt::EditRole);
                m_motionRef->replaceKeyframe(newMorphKeyframe.take());
            }
        }
        /* #undo のコメント通りのため、省略 */
        m_motionRef->update(IKeyframe::kMorphKeyframe);
        m_mmmRef->refreshModel(m_modelRef);
    }

private:
    const MorphMotionModel::Keys m_keys;
    QList<ModelIndex> m_modelIndices;
    QScopedPointer<Pose> m_pose;
    MorphMotionModel *m_mmmRef;
    IModelSharedPtr m_modelRef;
    IMotionSharedPtr m_motionRef;
    int m_timeIndex;
};

class SetKeyframesCommand : public QUndoCommand
{
public:
    typedef QPair<QModelIndex, QByteArray> ModelIndex;

    /*
     * MorphMotionModel で selectedModel/currentMotion に変化があるとまずいのでポインタを保存しておく
     * モデルまたモーションを削除すると このコマンドのインスタンスを格納した UndoStack も一緒に削除される
     */
    SetKeyframesCommand(const MorphMotionModel::KeyFramePairList &keyframes,
                        IMotionSharedPtr motionRef,
                        MorphMotionModel *fmm)
        : QUndoCommand(),
          m_keys(fmm->keys()),
          m_fmmRef(fmm),
          m_modelRef(fmm->selectedModel()),
          m_motionRef(motionRef),
          m_morphRef(fmm->selectedMorph())
    {
        QSet<int> indexProceeded;
        /* 現在選択中のモデルにある全ての頂点モーフを取り出す */
        const PMDMotionModel::TreeItemList &items = m_keys.values();
        /* フレームインデックスがまたがるので複雑だが対象のキーフレームを全て保存しておく */
        foreach (const MorphMotionModel::KeyFramePair &frame, keyframes) {
            int timeIndex = frame.first;
            /* フレーム単位での重複を避けるためにスキップ処理を設ける */
            if (!indexProceeded.contains(timeIndex)) {
                /* モデルの全ての頂点モーフを対象にデータがあるか確認し、存在している場合のみボーンのキーフレームの生データを保存する */
                foreach (PMDMotionModel::ITreeItem *item, items) {
                    const QModelIndex &index = m_fmmRef->timeIndexToModelIndex(item, timeIndex);
                    const QVariant &data = index.data(MorphMotionModel::kBinaryDataRole);
                    if (data.canConvert(QVariant::ByteArray))
                        m_modelIndices.append(ModelIndex(index, data.toByteArray()));
                }
                indexProceeded.insert(timeIndex);
            }
        }
        m_keyframes = keyframes;
        m_frameIndices = indexProceeded.toList();
        setText(QApplication::tr("Register morph keyframes of %1").arg(Util::toQStringFromModel(m_modelRef.data())));
    }
    ~SetKeyframesCommand() {
    }

    void undo() {
        /* 対象のキーフレームのインデックスを全て削除、さらにモデルのデータも削除 */
        const PMDMotionModel::TreeItemList &items = m_keys.values();
        Array<IKeyframe *> keyframes;
        foreach (int timeIndex, m_frameIndices) {
            keyframes.clear();
            m_motionRef->getKeyframes(timeIndex, 0, IKeyframe::kMorphKeyframe, keyframes);
            const int nkeyframes = keyframes.count();
            for (int i = 0; i < nkeyframes; i++) {
                IKeyframe *keyframe = keyframes[i];
                m_motionRef->deleteKeyframe(keyframe);
            }
            foreach (PMDMotionModel::ITreeItem *item, items) {
                const QModelIndex &index = m_fmmRef->timeIndexToModelIndex(item, timeIndex);
                m_fmmRef->setData(index, QVariant());
            }
        }
        /* 削除後のインデックス更新を忘れなく行う */
        m_motionRef->update(IKeyframe::kMorphKeyframe);
        /* コンストラクタで保存したキーフレームの生データから頂点モーフのキーフレームに復元して置換する */
        Factory *factory = m_fmmRef->factoryRef();
        QScopedPointer<IMorphKeyframe> frame;
        foreach (const ModelIndex &index, m_modelIndices) {
            const QByteArray &bytes = index.second;
            m_fmmRef->setData(index.first, bytes, Qt::EditRole);
            frame.reset(factory->createMorphKeyframe(m_motionRef.data()));
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            m_motionRef->addKeyframe(frame.take());
        }
        /* addKeyframe によって変更が必要になる内部インデックスの更新を行うため、update をかけておく */
        m_motionRef->update(IKeyframe::kMorphKeyframe);
        m_fmmRef->refreshModel(m_modelRef);
    }
    virtual void redo() {
        QScopedPointer<IMorphKeyframe> newMorphKeyframe;
        QString key;
        /* すべてのキーフレーム情報を登録する */
        foreach (const MorphMotionModel::KeyFramePair &pair, m_keyframes) {
            int timeIndex = pair.first;
            const MorphMotionModel::KeyFramePtr &ptr = pair.second;
            IMorphKeyframe *morphKeyframe = ptr.data();
            /* キーフレームの対象頂点モーフ名を取得する */
            if (morphKeyframe) {
                key = Util::toQStringFromMorphKeyframe(morphKeyframe);
            }
            else if (m_morphRef) {
                key = Util::toQStringFromMorph(m_morphRef);
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
                const QModelIndex &modelIndex = m_fmmRef->timeIndexToModelIndex(m_keys[key], timeIndex);
                if (morphKeyframe->timeIndex() >= 0) {
                    QByteArray bytes(morphKeyframe->estimateSize(), '0');
                    newMorphKeyframe.reset(morphKeyframe->clone());
                    newMorphKeyframe->setTimeIndex(timeIndex);
                    newMorphKeyframe->write(reinterpret_cast<uint8_t *>(bytes.data()));
                    m_motionRef->replaceKeyframe(newMorphKeyframe.take());
                    m_fmmRef->setData(modelIndex, bytes, Qt::EditRole);
                }
                else {
                    /* 元フレームのインデックスが 0 未満の時は削除 */
                    IKeyframe *frameToDelete = m_motionRef->findMorphKeyframe(timeIndex, morphKeyframe->name(), 0);
                    m_motionRef->deleteKeyframe(frameToDelete);
                    m_fmmRef->setData(modelIndex, QVariant());
                }
            }
            else {
                qWarning("Tried registering not morph key frame: %s", qPrintable(key));
                continue;
            }
        }
        /* SetFramesCommand#undo の通りのため、省略 */
        m_motionRef->update(IKeyframe::kMorphKeyframe);
        m_fmmRef->refreshModel(m_modelRef);
    }

private:
    const MorphMotionModel::Keys m_keys;
    /* undo で復元する対象のキーフレームの番号 */
    QList<int> m_frameIndices;
    /* m_frameIndices に加えて undo で復元する用のキーフレームの集合 */
    QList<ModelIndex> m_modelIndices;
    /* 実際に登録する用のキーフレームの集合 */
    MorphMotionModel::KeyFramePairList m_keyframes;
    MorphMotionModel *m_fmmRef;
    IModelSharedPtr m_modelRef;
    IMotionSharedPtr m_motionRef;
    IMorph *m_morphRef;
};

class ResetAllCommand : public QUndoCommand
{
public:
    ResetAllCommand(const vpvl2::Scene *scene, IModelSharedPtr model)
        : QUndoCommand(),
          m_state(scene, model)
    {
        /* 全ての頂点モーフの情報を保存しておく */
        m_state.save();
        setText(QApplication::tr("Reset all morphs of %1").arg(Util::toQStringFromModel(model.data())));
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
    SetMorphCommand(const vpvl2::Scene *scene, IModelSharedPtr model, const PMDMotionModel::State &oldState)
        : QUndoCommand(),
          m_oldState(scene, IModelSharedPtr()),
          m_newState(scene, model)
    {
        m_oldState.copyFrom(oldState);
        /* 前と後の全ての頂点モーフの情報を保存しておく */
        m_newState.save();
        setText(QApplication::tr("Set morphs of %1").arg(Util::toQStringFromModel(model.data())));
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
    IModelSharedPtr m_model;
    PMDMotionModel::State m_oldState;
    PMDMotionModel::State m_newState;
};

static IMorph *UIMorphFromModelIndex(const QModelIndex &index, IModelSharedPtr model)
{
    /* QModelIndex -> TreeIndex -> ByteArray -> Face の順番で対象の頂点モーフを求めて選択状態にする作業 */
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    const String s(Util::fromQString(item->name()));
    return model->findMorph(&s);
}

}

namespace vpvm
{

MorphMotionModel::MorphMotionModel(Factory *factoryRef, QUndoGroup *undoRef, QObject *parent)
    : PMDMotionModel(undoRef, parent),
      m_factoryRef(factoryRef),
      m_state(0, IModelSharedPtr())
{
}

MorphMotionModel::~MorphMotionModel()
{
    m_factoryRef = 0;
}

void MorphMotionModel::saveMotion(IMotion *motion)
{
    if (m_modelRef) {
        /* モデルの ByteArray を BoneKeyFrame に読ませて積んでおくだけの簡単な処理 */
        foreach (QVariant value, values()) {
            IMorphKeyframe *newFrame = m_factoryRef->createMorphKeyframe(motion);
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
    if (IMotionSharedPtr motionRef = currentMotionRef()) {
        KeyFramePairList keyframes;
        IModelSharedPtr model = selectedModel();
        /* モデルのインデックスを参照し、存在する頂点モーフに対して頂点モーフの現在の重み係数から頂点モーフのキーフレームにコピーする */
        foreach (const QModelIndex &index, indices) {
            int timeIndex = toTimeIndex(index);
            if (timeIndex >= 0) {
                const QString &name = nameFromModelIndex(index);
                String s(Util::fromQString(name));
                IMorph *morph = model->findMorph(&s);
                if (morph) {
                    KeyFramePtr keyframe(m_factoryRef->createMorphKeyframe(motionRef.data()));
                    keyframe->setName(morph->name());
                    keyframe->setWeight(morph->weight());
                    keyframes.append(KeyFramePair(timeIndex, keyframe));
                }
            }
        }
        setKeyframes(keyframes);
    }
}

void MorphMotionModel::copyKeyframesByModelIndices(const QModelIndexList &indices, int timeIndex)
{
    if (IMotionSharedPtr motionRef = currentMotionRef()) {
        /* 前回呼ばれた copyFrames で作成したデータを破棄しておく */
        m_copiedKeyframes.clear();
        foreach (const QModelIndex &index, indices) {
            const QVariant &variant = index.data(kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                const QByteArray &bytes = variant.toByteArray();
                KeyFramePtr keyframe(m_factoryRef->createMorphKeyframe(motionRef.data()));
                keyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
                /* 予め差分をとっておき、pasteKeyframes でペースト先の差分をたすようにする */
                int diff = keyframe->timeIndex() - timeIndex;
                m_copiedKeyframes.append(KeyFramePair(diff, keyframe));
            }
        }
    }
}

void MorphMotionModel::pasteKeyframesByTimeIndex(int timeIndex)
{
    IMotionSharedPtr motionRef = currentMotionRef();
    if (motionRef && !m_copiedKeyframes.isEmpty()) {
        MorphMotionModel::KeyFramePairList keyframes;
        foreach (const KeyFramePair &pair, m_copiedKeyframes) {
            KeyFramePtr keyframe(static_cast<IMorphKeyframe *>(pair.second->clone()));
            /* コピー先にフレームインデックスを更新する */
            int newTimeIndex = timeIndex + pair.first;
            keyframes.append(KeyFramePair(newTimeIndex, keyframe));
        }
        addUndoCommand(new SetKeyframesCommand(keyframes, motionRef, this));
    }
}

void MorphMotionModel::saveTransform()
{
    /* モデルの状態を保存しておく */
    if (m_modelRef) {
        m_state.setModelRef(m_modelRef);
        m_state.save();
    }
}

void MorphMotionModel::commitTransform()
{
    /* 状態を圧縮し、モーフ変形があれば SetMorphCommand を作成して UndoStack に追加 */
    if (m_modelRef && m_state.compact())
        addUndoCommand(new SetMorphCommand(m_sceneRef, m_modelRef, m_state));
}

void MorphMotionModel::selectKeyframesByModelIndices(const QModelIndexList &indices)
{
    if (m_modelRef) {
        QList<IMorph *> morphs;
        foreach (const QModelIndex &index, indices) {
            if (index.isValid()) {
                IMorph *morph = UIMorphFromModelIndex(index, m_modelRef);
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

const QModelIndexList MorphMotionModel::modelIndicesFromMorphs(const QList<IMorph *> &morphs, int timeIndex) const
{
    const QSet<IMorph *> &morphSet = morphs.toSet();
    QModelIndexList indices;
    foreach (PMDMotionModel::ITreeItem *item, keys().values()) {
        TreeItem *treeItem = static_cast<TreeItem *>(item);
        if (morphSet.contains(treeItem->morph())) {
            const QModelIndex &index = timeIndexToModelIndex(item, timeIndex);
            indices.append(index);
        }
    }
    return indices;
}

void MorphMotionModel::loadPose(PosePtr pose, IModelSharedPtr model, int timeIndex)
{
    IMotionSharedPtr motionRef = currentMotionRef();
    if (model == m_modelRef && motionRef) {
        addUndoCommand(new LoadPoseCommand(this, pose, timeIndex));
        qDebug("Loaded a pose to the model: %s", qPrintable(Util::toQStringFromModel(model.data())));
    }
    else {
        qWarning("Tried loading pose to invalid model or without motion: %s", qPrintable(Util::toQStringFromModel(model.data())));
    }
}

void MorphMotionModel::setKeyframes(const KeyFramePairList &keyframes)
{
    if (IMotionSharedPtr motionRef = currentMotionRef()) {
        addUndoCommand(new SetKeyframesCommand(keyframes, motionRef, this));
    }
    else {
        qWarning("No model or motion to register morph frames.");
        return;
    }
}

void MorphMotionModel::resetAllMorphs()
{
    if (m_modelRef)
        addUndoCommand(new ResetAllCommand(m_sceneRef, m_modelRef));
}

void MorphMotionModel::setPMDModel(IModelSharedPtr model)
{
    /* 引数のモデルが現在選択中のものであれば二重処理になってしまうので、スキップする */
    if (m_modelRef == model)
        return;
    if (model) {
        /* PMD の二重登録防止 */
        if (!hasPMDModel(model)) {
            /* ルートを作成 */
            RootPtr rootItemPtr(new TreeItem("", 0, true, false, 0));
            TreeItem *rootItem = static_cast<TreeItem *>(rootItemPtr.data());
            /* 予め決められたカテゴリのアイテムを作成する */
            QScopedPointer<TreeItem> eyeblow(new TreeItem(vpvm::MorphMotionModel::tr("Eyeblow"), 0, false, true, rootItem));
            QScopedPointer<TreeItem> eye(new TreeItem(vpvm::MorphMotionModel::tr("Eye"), 0, false, true, rootItem));
            QScopedPointer<TreeItem> lip(new TreeItem(vpvm::MorphMotionModel::tr("Lip"), 0, false, true, rootItem));
            QScopedPointer<TreeItem> other(new TreeItem(vpvm::MorphMotionModel::tr("Other"), 0, false, true, rootItem));
            Array<IMorph *> morphs;
            model->getMorphRefs(morphs);
            const int nmorphs = morphs.count();
            Keys keys;
            for (int i = 0; i < nmorphs; i++) {
                IMorph *morph = morphs[i];
                const QString &name = Util::toQStringFromMorph(morph);
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
        m_modelRef = model;
        m_state.setModelRef(model);
        emit modelDidChange(model);
        qDebug("Set a model in MorphMotionModel: %s", qPrintable(Util::toQStringFromModel(model.data())));
    }
    else {
        m_modelRef.clear();
        emit modelDidChange(m_modelRef);
    }
    /* テーブルモデルを更新 */
    setTimeIndexColumnMax(0);
    resetModel();
}

void MorphMotionModel::loadMotion(IMotionSharedPtr motion, const IModelSharedPtr model)
{
    /* 現在のモデルが対象のモデルと一致していることを確認しておく */
    if (model == m_modelRef) {
        const int nkeyframes = motion->countKeyframes(IKeyframe::kMorphKeyframe);
        const QString &modelName = Util::toQStringFromModel(model.data()),
                &loadingProgressText = vpvm::MorphMotionModel::tr("Loading morph keyframes %1 of %2 to %3");
        /* フレーム列の最大数をモーションのフレーム数に更新する */
        setTimeIndexColumnMax(motion);
        resetModel();
        emit motionDidOpenProgress(vpvm::MorphMotionModel::tr("Loading a motion of morphs to %1").arg(Util::toQStringFromModel(model.data())), false);
        /* モーションのすべてのキーフレームを参照し、モデルの頂点モーフ名に存在するものだけ登録する */
        int updateInterval = qMax(int(nkeyframes / 100), 1);
        for (int i = 0; i < nkeyframes; i++) {
            IMorphKeyframe *keyframe = motion->findMorphKeyframeAt(i);
            const QString &key = Util::toQStringFromMorphKeyframe(keyframe);
            const Keys &keys = this->keys();
            if (keys.contains(key)) {
                int timeIndex = static_cast<int>(keyframe->timeIndex());
                QByteArray bytes(keyframe->estimateSize(), '0');
                ITreeItem *item = keys[key];
                /* この時点で新しい QModelIndex が作成される */
                const QModelIndex &modelIndex = timeIndexToModelIndex(item, timeIndex);
                IMorphKeyframe *newFrame = m_factoryRef->createMorphKeyframe(motion.data());
                newFrame->setName(keyframe->name());
                newFrame->setWeight(keyframe->weight());
                newFrame->setTimeIndex(timeIndex);
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes);
            }
            if ((i % updateInterval) == 0) {
                emit motionDidUpdateProgress(i, nkeyframes, loadingProgressText.arg(i).arg(nkeyframes).arg(modelName));
            }
        }
        /* 読み込まれたモーションを現在のモーションとして登録する。あとは SetFramesCommand#undo と同じ */
        addPMDModelMotion(model, motion);
        refreshModel(m_modelRef);
        setModified(false);
        emit motionDidLoad();
        qDebug("Loaded a motion to the model in MorphMotionModel: %s", qPrintable(Util::toQStringFromModel(model.data())));
    }
    else {
        qDebug("Tried loading a motion to different model, ignored: %s", qPrintable(Util::toQStringFromModel(model.data())));
    }
}

void MorphMotionModel::removeMotion()
{
    /* コピーしたキーフレーム、選択された頂点モーフとモデルに登録されているデータが削除される。頂点モーフ名は削除されない */
    m_selectedMorphs.clear();
    m_copiedKeyframes.clear();
    setModified(false);
    removePMDMotion(m_modelRef);
    resetModel();
}

void MorphMotionModel::removeModel()
{
    /*
     * モーション削除に加えて PMD を論理削除する。巻き戻し情報も削除されるため巻戻しが不可になる
     * PMD は SceneLoader で管理されるため、PMD のメモリの解放はしない
     */
    removeMotion();
    removePMDModel(m_modelRef);
    resetModel();
    emit modelDidChange(m_modelRef);
}

void MorphMotionModel::deleteKeyframesByModelIndices(const QModelIndexList &indices)
{
    if (IMotionSharedPtr motionRef = currentMotionRef()) {
        KeyFramePairList keyframes;
        /* ここでは削除するキーフレームを決定するのみ。実際に削除するのは SetFramesCommand である点に注意 */
        foreach (const QModelIndex &index, indices) {
            if (index.isValid() && index.column() > 1) {
                TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
                if (IMorph *morph = item->morph()) {
                    IMorphKeyframe *keyframeToDelete = motionRef->findMorphKeyframe(toTimeIndex(index), morph->name(), 0);
                    if (keyframeToDelete) {
                        KeyFramePtr clonedKeyframe(keyframeToDelete->clone());
                        /* SetFramesCommand で削除するので削除に必要な条件である timeIndex を 0 未満の値にしておく */
                        clonedKeyframe->setTimeIndex(-1);
                        keyframes.append(KeyFramePair(keyframeToDelete->timeIndex(), clonedKeyframe));
                    }
                }
            }
        }
        if (!keyframes.isEmpty())
            addUndoCommand(new SetKeyframesCommand(keyframes, motionRef, this));
    }
}

void MorphMotionModel::applyKeyframeWeightByModelIndices(const QModelIndexList &indices, float value)
{
    if (IMotionSharedPtr motionRef = currentMotionRef()) {
        KeyFramePairList keyframes;
        foreach (const QModelIndex &index, indices) {
            if (index.isValid()) {
                /* QModelIndex からキーフレームを取得し、その中に入っている値を補正する */
                const QVariant &variant = index.data(kBinaryDataRole);
                if (variant.canConvert(QVariant::ByteArray)) {
                    const QByteArray &bytes = variant.toByteArray();
                    KeyFramePtr keyframe(m_factoryRef->createMorphKeyframe(motionRef.data()));
                    keyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
                    keyframe->setWeight(keyframe->weight() * value);
                    keyframes.append(KeyFramePair(toTimeIndex(index), keyframe));
                }
            }
        }
        setKeyframes(keyframes);
    }
}

void MorphMotionModel::selectMorphsByModelIndices(const QModelIndexList &indices)
{
    QList<IMorph *> morphs;
    foreach (const QModelIndex &index, indices) {
        if (index.isValid()) {
            TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
            if (item->isCategory()) {
                int nbones = item->countChildren();
                for (int i = 0; i < nbones; i++) {
                    TreeItem *child = static_cast<TreeItem *>(item->child(i));
                    IMorph *bone = child->morph();
                    morphs.append(bone);
                }
            }
            else if (IMorph *bone = item->morph()) {
                morphs.append(bone);
            }
        }
    }
    selectMorphs(morphs);
}

bool MorphMotionModel::isSelectionIdentical(const QList<IMorph *> &morphs)
{
    return Util::compare(morphs, m_selectedMorphs);
}

void MorphMotionModel::selectMorphs(const QList<IMorph *> &morphs)
{
    /* signal/slot による循環参照防止 */
    if (!Util::compare(morphs, m_selectedMorphs)) {
        m_selectedMorphs = morphs;
        emit morphsDidSelect(morphs);
    }
}

void MorphMotionModel::setWeight(IMorph::WeightPrecision &value)
{
    if (!m_selectedMorphs.isEmpty())
        setWeight(value, m_selectedMorphs.last());
}

void MorphMotionModel::setWeight(const IMorph::WeightPrecision &value, IMorph *morph)
{
    if (morph) {
        Array<IMorph *> morphs;
        m_modelRef->getMorphRefs(morphs);
        const int nmorphs = morphs.count();
        for (int i = 0; i < nmorphs; i++) {
            IMorph *m = morphs[i];
            /* 頂点モーフでかつ変更するモーフ以外を更新するようにする */
            if (morph != m && morph->type() == IMorph::kVertexMorph)
                m->setWeight(m->weight());
        }
        morph->setWeight(value);
        m_sceneRef->updateModel(m_modelRef.data());
    }
}

void MorphMotionModel::setSceneRef(const Scene *value)
{
    PMDMotionModel::setSceneRef(value);
    m_state.setSceneRef(value);
}

} /* namespace vpvm */

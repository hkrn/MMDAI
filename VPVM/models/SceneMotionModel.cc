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
#include "models/SceneMotionModel.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

namespace {

class RootTreeItem : public MotionBaseModel::ITreeItem
{
public:
    RootTreeItem()
    {
    }
    ~RootTreeItem() {
        qDeleteAll(m_children);
    }

    void addChild(ITreeItem *item) {
        m_children.append(item);
    }
    ITreeItem *parent() const {
        return 0;
    }
    ITreeItem *child(int row) const {
        return m_children.value(row);
    }
    const QString &name() const {
        static const QString empty = "";
        return empty;
    }
    bool isRoot() const {
        return true;
    }
    bool isCategory() const {
        return false;
    }
    int rowIndex() const {
        return 0;
    }
    int countChildren() const {
        return m_children.count();
    }

private:
    QList<ITreeItem *> m_children;
};

class CameraTreeItem : public MotionBaseModel::ITreeItem
{
public:
    CameraTreeItem(RootTreeItem *root)
        : m_parent(root)
    {
    }
    ~CameraTreeItem() {
    }

    void addChild(ITreeItem * /* item */) {
    }
    ITreeItem *parent() const {
        return m_parent;
    }
    ITreeItem *child(int /* row */) const {
        return 0;
    }
    const QString &name() const {
        /* 参照にしないこと */
        static const QString name = QApplication::tr("Camera");
        return name;
    }
    bool isRoot() const {
        return false;
    }
    bool isCategory() const {
        return true;
    }
    int rowIndex() const {
        return 0;
    }
    int countChildren() const {
        return 0;
    }

private:
    ITreeItem *m_parent;
};

class LightTreeItem : public MotionBaseModel::ITreeItem
{
public:
    LightTreeItem(RootTreeItem *root)
        : m_parent(root)
    {
    }
    ~LightTreeItem() {
    }

    void addChild(ITreeItem * /* item */) {
    }
    ITreeItem *parent() const {
        return m_parent;
    }
    ITreeItem *child(int /* row */) const {
        return 0;
    }
    const QString &name() const {
        static const QString name = QApplication::tr("Light");
        return name;
    }
    bool isRoot() const {
        return false;
    }
    bool isCategory() const {
        return true;
    }
    int rowIndex() const {
        return 1;
    }
    int countChildren() const {
        return 0;
    }

private:
    ITreeItem *m_parent;
};

class SetKeyframesCommand : public QUndoCommand
{
public:
    SetKeyframesCommand(SceneMotionModel *smm,
                        const SceneMotionModel::CameraKeyframePairList &cameraKeyframes,
                        const SceneMotionModel::LightKeyframePairList &lightKeyframes,
                        SceneMotionModel::ITreeItem *cameraTreeItem,
                        SceneMotionModel::ITreeItem *lightTreeItem)
        : QUndoCommand(),
          m_smm(smm),
          m_cameraTreeItem(cameraTreeItem),
          m_lightTreeItem(lightTreeItem)
    {
        /* 処理する前のカメラのキーフレームを保存 */
        QSet<int> cameraIndices;
        foreach (const SceneMotionModel::CameraKeyframePair &keyframe, cameraKeyframes) {
            int frameIndex = keyframe.first;
            if (!cameraIndices.contains(frameIndex)) {
                const QModelIndex &index = m_smm->frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
                if (index.data(SceneMotionModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
                    m_cameraIndices.append(index);
                cameraIndices.insert(frameIndex);
            }
        }
        m_cameraKeyframes = cameraKeyframes;
        m_cameraFrameIndices = cameraIndices.toList();
        /* 処理する前の照明のキーフレームを保存 */
        QSet<int> lightIndices;
        foreach (const SceneMotionModel::LightKeyframePair &keyframe, lightKeyframes) {
            int frameIndex = keyframe.first;
            if (!lightIndices.contains(frameIndex)) {
                const QModelIndex &index = m_smm->frameIndexToModelIndex(m_lightTreeItem, frameIndex);
                if (index.data(SceneMotionModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
                    m_lightIndices.append(index);
                lightIndices.insert(frameIndex);
            }
        }
        m_lightKeyframes = lightKeyframes;
        m_lightFrameIndices = lightIndices.toList();
        setText(QApplication::tr("Register camera/light keyframes"));
    }
    virtual ~SetKeyframesCommand() {
    }

    virtual void undo() {
        /* 処理したキーフレームを一旦削除 */
        IMotion *motion = m_smm->currentMotion();
        foreach (int frameIndex, m_cameraFrameIndices) {
            motion->deleteKeyframes(frameIndex, IKeyframe::kCamera);
            const QModelIndex &index = m_smm->frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
            m_smm->setData(index, QVariant());
        }
        /* コンストラクタで保存したキーフレームを復元 */
        Factory *factory = m_smm->factory();
        QScopedPointer<ICameraKeyframe> newCameraKeyframe;
        foreach (const QModelIndex &index, m_cameraIndices) {
            const QByteArray &bytes = index.data(SceneMotionModel::kBinaryDataRole).toByteArray();
            m_smm->setData(index, bytes, Qt::EditRole);
            newCameraKeyframe.reset(factory->createCameraKeyframe());
            newCameraKeyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            motion->replaceKeyframe(newCameraKeyframe.take());
        }
        /*
         * replaceKeyframe (内部的には addKeyframe を呼んでいる) によって変更が必要になる
         * 内部インデックスの更新を行うため、update をかけておく
         */
        motion->update(IKeyframe::kCamera);
        /* 処理したキーフレームを一旦削除 */
        foreach (int frameIndex, m_lightFrameIndices) {
            motion->deleteKeyframes(frameIndex, IKeyframe::kLight);
            const QModelIndex &index = m_smm->frameIndexToModelIndex(m_lightTreeItem, frameIndex);
            m_smm->setData(index, QVariant());
        }
        /* コンストラクタで保存したキーフレームを復元 */
        QScopedPointer<ILightKeyframe> newLightKeyframe;
        foreach (const QModelIndex &index, m_cameraIndices) {
            const QByteArray &bytes = index.data(SceneMotionModel::kBinaryDataRole).toByteArray();
            m_smm->setData(index, bytes, Qt::EditRole);
            newLightKeyframe.reset(factory->createLightKeyframe());
            newLightKeyframe->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            motion->replaceKeyframe(newLightKeyframe.take());
        }
        motion->update(IKeyframe::kLight);
        m_smm->refreshScene();
    }
    virtual void redo() {
        QScopedPointer<ICameraKeyframe> newCameraKeyframe;
        IMotion *motion = m_smm->currentMotion();
        /* カメラのキーフレームを上書き処理 */
        foreach (const SceneMotionModel::CameraKeyframePair &pair, m_cameraKeyframes) {
            int frameIndex = pair.first;
            const SceneMotionModel::CameraKeyframePtr &ptr = pair.second;
            ICameraKeyframe *cameraKeyframe = ptr.data();
            const QModelIndex &modelIndex = m_smm->frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
            if (cameraKeyframe->timeIndex() >= 0) {
                QByteArray bytes(cameraKeyframe->estimateSize(), '0');
                newCameraKeyframe.reset(cameraKeyframe->clone());
                newCameraKeyframe->setTimeIndex(frameIndex);
                newCameraKeyframe->write(reinterpret_cast<uint8_t *>(bytes.data()));
                motion->replaceKeyframe(newCameraKeyframe.take());
                m_smm->setData(modelIndex, bytes);
            }
            else {
                /* 元フレームのインデックスが 0 未満の時は削除 */
                IKeyframe *frameToDelete = motion->findCameraKeyframe(frameIndex);
                motion->deleteKeyframe(frameToDelete);
                m_smm->setData(modelIndex, QVariant());
            }
        }
        /* #undo と同じため、説明省略 */
        motion->update(IKeyframe::kCamera);
        /* 照明のキーフレームを上書き処理 */
        QScopedPointer<ILightKeyframe> newLightKeyframe;
        foreach (const SceneMotionModel::LightKeyframePair &pair, m_lightKeyframes) {
            int frameIndex = pair.first;
            const SceneMotionModel::LightKeyframePtr &ptr = pair.second;
            ILightKeyframe *lightKeyframe = ptr.data();
            const QModelIndex &modelIndex = m_smm->frameIndexToModelIndex(m_lightTreeItem, frameIndex);
            if (lightKeyframe->timeIndex() >= 0) {
                QByteArray bytes(lightKeyframe->estimateSize(), '0');
                newLightKeyframe.reset(lightKeyframe->clone());
                newLightKeyframe->setTimeIndex(frameIndex);
                newLightKeyframe->write(reinterpret_cast<uint8_t *>(bytes.data()));
                motion->replaceKeyframe(newLightKeyframe.take());
                m_smm->setData(modelIndex, bytes);
            }
            else {
                /* 元フレームのインデックスが 0 未満の時は削除 */
                IKeyframe *frameToDelete = motion->findLightKeyframe(frameIndex);
                motion->deleteKeyframe(frameToDelete);
                m_smm->setData(modelIndex, QVariant());
            }
        }
        /* #undo と同じため、説明省略 */
        motion->update(IKeyframe::kLight);
        m_smm->refreshScene();
    }

private:
    QList<int> m_cameraFrameIndices;
    QList<int> m_lightFrameIndices;
    QModelIndexList m_cameraIndices;
    QModelIndexList m_lightIndices;
    SceneMotionModel::CameraKeyframePairList m_cameraKeyframes;
    SceneMotionModel::LightKeyframePairList m_lightKeyframes;
    SceneMotionModel *m_smm;
    SceneMotionModel::ITreeItem *m_cameraTreeItem;
    SceneMotionModel::ITreeItem *m_lightTreeItem;
};

}

SceneMotionModel::SceneMotionModel(Factory *factory,
                                   QUndoGroup *undo,
                                   const SceneWidget *sceneWidget,
                                   QObject *parent)
    : MotionBaseModel(undo, parent),
      m_sceneWidget(sceneWidget),
      m_stack(0),
      m_factory(factory),
      m_rootTreeItem(0),
      m_cameraTreeItem(0),
      m_lightTreeItem(0)
{
    RootTreeItem *root = new RootTreeItem();
    m_rootTreeItem = root;
    m_cameraTreeItem = new CameraTreeItem(root);
    m_lightTreeItem = new LightTreeItem(root);
    m_rootTreeItem->addChild(m_cameraTreeItem);
    m_rootTreeItem->addChild(m_lightTreeItem);
}

SceneMotionModel::~SceneMotionModel()
{
    // m_cameraTreeItem/m_lightTreeItem は m_rootTreeItem に子供として追加されてるので、delete するときに同時に delete される
    delete m_rootTreeItem;
    delete m_stack;
}

QVariant SceneMotionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
    if (role == Qt::DisplayRole && index.column() == 0) {
        return item->name();
    }
    else if (role == kBinaryDataRole) {
        QVariant value;
        if (item->isCategory()) {
            if (index.row() == m_cameraTreeItem->rowIndex())
                value = m_cameraData.value(index);
            else if (index.row() == m_lightTreeItem->rowIndex())
                value = m_lightData.value(index);
        }
        return value;
    }
    else {
        return QVariant();
    }
}

bool SceneMotionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
        if (item->isCategory()) {
            bool found = false;
            if (index.row() == m_cameraTreeItem->rowIndex()) {
                m_cameraData.insert(index, value);
                found = true;
            }
            else if (index.row() == m_lightTreeItem->rowIndex()) {
                m_lightData.insert(index, value);
                found = true;
            }
            if (found) {
                setModified(true);
                emit dataChanged(index, index);
                return true;
            }
        }
    }
    return false;
}

int SceneMotionModel::columnCount(const QModelIndex & /* parent */) const
{
    return maxFrameCount() + 1;
}

int SceneMotionModel::maxFrameIndex() const
{
    return m_motion ? m_motion->maxTimeIndex() : 0;
}

bool SceneMotionModel::forceCameraUpdate() const
{
    return true;
}

const QModelIndex SceneMotionModel::frameIndexToModelIndex(ITreeItem *item, int frameIndex) const
{
    QModelIndex modelIndex;
    int rowIndex = item->rowIndex();
    if (item->isCategory() && (rowIndex == m_cameraTreeItem->rowIndex() || rowIndex == m_lightTreeItem->rowIndex())) {
        modelIndex = index(rowIndex, toModelIndex(frameIndex), QModelIndex());
        if (!modelIndex.isValid())
            createIndex(rowIndex, frameIndex, item);
    }
    else {
        const QModelIndex &parentIndex = index(item->parent()->rowIndex(), 0);
        modelIndex = index(rowIndex, toModelIndex(frameIndex), parentIndex);
        if (!modelIndex.isValid())
            createIndex(rowIndex, frameIndex, item);
    }
    return modelIndex;
}

void SceneMotionModel::saveMotion(vpvl2::IMotion *motion)
{
    if (m_cameraData.size() > 1) {
        QScopedPointer<ICameraKeyframe> cameraKeyframes;
        foreach (const QVariant &value, m_cameraData) {
            const QByteArray &bytes = value.toByteArray();
            cameraKeyframes.reset(m_factory->createCameraKeyframe());
            cameraKeyframes->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            motion->addKeyframe(cameraKeyframes.take());
        }
        /* addKeyframe によって変更が必要になる内部インデックスの更新を行うため、update をかけておく */
        motion->update(IKeyframe::kCamera);
    }
    if (m_lightData.size() > 1) {
        QScopedPointer<ILightKeyframe> lightKeyframes;
        foreach (const QVariant &value, m_lightData) {
            const QByteArray &bytes = value.toByteArray();
            lightKeyframes.reset(m_factory->createLightKeyframe());
            lightKeyframes->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            motion->addKeyframe(lightKeyframes.take());
        }
        /* addKeyframe によって変更が必要になる内部インデックスの更新を行うため、update をかけておく */
        motion->update(IKeyframe::kLight);
    }
    setModified(false);
}

void SceneMotionModel::addKeyframesByModelIndices(const QModelIndexList &indices)
{
    CameraKeyframePairList cameraKeyframes;
    LightKeyframePairList lightKeyframes;
    QScopedPointer<ICameraKeyframe> cameraKeyframe;
    QScopedPointer<ILightKeyframe> lightKeyframe;
    foreach (const QModelIndex &index, indices) {
        ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
        int frameIndex = toTimeIndex(index);
        if (frameIndex >= 0 && item->isCategory()) {
            if (index.row() == m_cameraTreeItem->rowIndex()) {
                const ICamera *camera = m_sceneWidget->sceneLoader()->scene()->camera();
                cameraKeyframe.reset(m_factory->createCameraKeyframe());
                cameraKeyframe->setDefaultInterpolationParameter();
                cameraKeyframe->setPosition(camera->position());
                cameraKeyframe->setAngle(camera->angle());
                cameraKeyframe->setFov(camera->fov());
                cameraKeyframe->setDistance(camera->distance());
                cameraKeyframes.append(CameraKeyframePair(frameIndex, CameraKeyframePtr(cameraKeyframe.take())));
            }
            else if (index.row() == m_lightTreeItem->rowIndex()) {
                const ILight *light = m_sceneWidget->sceneLoader()->scene()->light();
                lightKeyframe.reset(m_factory->createLightKeyframe());
                lightKeyframe->setColor(light->color());
                lightKeyframe->setDirection(light->direction());
                lightKeyframes.append(LightKeyframePair(frameIndex, LightKeyframePtr(lightKeyframe.take())));
            }
        }
    }
    setKeyframes(cameraKeyframes, lightKeyframes);
}

void SceneMotionModel::copyKeyframesByModelIndices(const QModelIndexList & /* indices */, int frameIndex)
{
    /* 照明データ未対応 */
    m_cameraIndex = frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
    m_lightIndex = frameIndexToModelIndex(m_lightTreeItem, frameIndex);
}

void SceneMotionModel::pasteKeyframesByTimeIndex(int frameIndex)
{
    /* 照明データ未対応 */
    CameraKeyframePairList cameraKeyframes;
    if (m_cameraIndex.isValid()) {
        const QVariant &variant = m_cameraIndex.data(SceneMotionModel::kBinaryDataRole);
        if (variant.canConvert(QVariant::ByteArray)) {
            CameraKeyframePtr keyframe(m_factory->createCameraKeyframe());
            keyframe->read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
            cameraKeyframes.append(CameraKeyframePair(frameIndex, keyframe));
        }
    }
    LightKeyframePairList lightKeyframes;
    if (m_lightIndex.isValid()) {
        const QVariant &variant = m_lightIndex.data(SceneMotionModel::kBinaryDataRole);
        if (variant.canConvert(QVariant::ByteArray)) {
            LightKeyframePtr keyframe(m_factory->createLightKeyframe());
            keyframe->read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
            lightKeyframes.append(LightKeyframePair(frameIndex, keyframe));
        }
    }
    addUndoCommand(new SetKeyframesCommand(this, cameraKeyframes, lightKeyframes, m_cameraTreeItem, m_lightTreeItem));
}

const QString SceneMotionModel::nameFromModelIndex(const QModelIndex & /* index */) const
{
    return QString();
}

SceneMotionModel::CameraKeyframePairList SceneMotionModel::keyframesFromModelIndices(const QModelIndexList &indices) const
{
    CameraKeyframePairList keyframes;
    foreach (const QModelIndex &index, indices) {
        const QVariant &variant = index.data(SceneMotionModel::kBinaryDataRole);
        if (variant.canConvert(QVariant::ByteArray)) {
            CameraKeyframePtr keyframe(m_factory->createCameraKeyframe());
            keyframe->read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
            keyframes.append(CameraKeyframePair(MotionBaseModel::toTimeIndex(index), keyframe));
        }
    }
    return keyframes;
}

void SceneMotionModel::loadMotion(IMotion *motion)
{
    if (motion) {
        const int nCameraFrames = motion->countKeyframes(IKeyframe::kCamera);
        const int nLightFrames = motion->countKeyframes(IKeyframe::kLight);
        if (nCameraFrames == 0 && nLightFrames == 0)
            return;
        m_cameraData.clear();
        m_lightData.clear();
        /* フレーム列の最大数をモーションのフレーム数に更新する */
        setFrameIndexColumnMax(motion);
        reset();
        /* カメラのキーフレームをテーブルのモデルデータにコピーする */
        QScopedPointer<ICameraKeyframe> newCameraKeyframe;
        for (int i = 0; i < nCameraFrames; i++) {
            const ICameraKeyframe *cameraKeyframe = motion->findCameraKeyframeAt(i);
            int frameIndex = static_cast<int>(cameraKeyframe->timeIndex());
            QByteArray bytes(cameraKeyframe->estimateSize(), '0');
            const QModelIndex &modelIndex = frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
            newCameraKeyframe.reset(cameraKeyframe->clone());
            newCameraKeyframe->write(reinterpret_cast<uint8_t *>(bytes.data()));
            setData(modelIndex, bytes);
        }
        /* 照明のキーフレームをテーブルのモデルデータにコピーする */
        QScopedPointer<ILightKeyframe> newLightKeyframe;
        for (int i = 0; i < nLightFrames; i++) {
            const ILightKeyframe *lightKeyframe = motion->findLightKeyframeAt(i);
            int frameIndex = static_cast<int>(lightKeyframe->timeIndex());
            QByteArray bytes(lightKeyframe->estimateSize(), '0');
            const QModelIndex &modelIndex = frameIndexToModelIndex(m_lightTreeItem, frameIndex);
            newLightKeyframe.reset(lightKeyframe->clone());
            newLightKeyframe->write(reinterpret_cast<uint8_t *>(bytes.data()));
            setData(modelIndex, bytes);
        }
        m_stack = new QUndoStack();
        m_undo->addStack(m_stack);
        m_undo->setActiveStack(m_stack);
        m_motion = motion;
        emit cameraMotionDidLoad();
    }
    reset();
}

void SceneMotionModel::setKeyframes(const CameraKeyframePairList &cameraKeyframes, const LightKeyframePairList &lightKeyframes)
{
    /* 照明データ未対応 */
    if (m_motion) {
        addUndoCommand(new SetKeyframesCommand(this, cameraKeyframes, lightKeyframes, m_cameraTreeItem, m_lightTreeItem));
    }
    else {
        qWarning("No motion to register camera frames.");
    }
}

void SceneMotionModel::setActiveUndoStack()
{
    m_undo->setActiveStack(m_stack);
}

void SceneMotionModel::refreshScene()
{
    setFrameIndexColumnMax(0);
    reset();
}

void SceneMotionModel::removeMotion()
{
    m_undo->setActiveStack(0);
    delete m_stack;
    m_stack = 0;
    m_cameraData.clear();
    m_lightData.clear();
    setModified(false);
    reset();
}

void SceneMotionModel::deleteKeyframesByModelIndices(const QModelIndexList &indices)
{
    /* 照明データ未対応 */
    CameraKeyframePairList cameraKeyframes;
    LightKeyframePairList lightKeyframes;
    foreach (const QModelIndex &index, indices) {
        if (index.isValid() && index.column() > 1) {
            ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
            if (item->isCategory()) {
                if (index.row() == m_cameraTreeItem->rowIndex()) {
                    ICameraKeyframe *frameToDelete = m_motion->findCameraKeyframe(toTimeIndex(index));
                    CameraKeyframePtr clonedCameraKeyframe(frameToDelete->clone());
                    /* SetFramesCommand で削除するので削除に必要な条件である frameIndex を 0 未満の値にしておく */
                    clonedCameraKeyframe->setTimeIndex(-1);
                    cameraKeyframes.append(CameraKeyframePair(frameToDelete->timeIndex(), clonedCameraKeyframe));
                }
                else if (index.row() == m_lightTreeItem->rowIndex()) {
                    ILightKeyframe *frameToDelete = m_motion->findLightKeyframe(toTimeIndex(index));
                    LightKeyframePtr clonedLightKeyframe(frameToDelete->clone());
                    /* SetFramesCommand で削除するので削除に必要な条件である frameIndex を 0 未満の値にしておく */
                    clonedLightKeyframe->setTimeIndex(-1);
                    lightKeyframes.append(LightKeyframePair(frameToDelete->timeIndex(), clonedLightKeyframe));
                }
            }
        }
    }
    addUndoCommand(new SetKeyframesCommand(this, cameraKeyframes, lightKeyframes, m_cameraTreeItem, m_lightTreeItem));
}

void SceneMotionModel::applyKeyframeWeightByModelIndices(const QModelIndexList & /* indices */, float /* value */)
{
    /* 現在この処理はまだ何も無い */
}

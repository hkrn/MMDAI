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

/* 今のところカメラフレームしか対応していないため、暫定実装になってる */
class SetKeyframesCommand : public QUndoCommand
{
public:
    SetKeyframesCommand(SceneMotionModel *smm,
                     const SceneMotionModel::CameraKeyFramePairList &keyframes,
                     SceneMotionModel::ITreeItem *cameraTreeItem)
        : QUndoCommand(),
          m_smm(smm),
          m_cameraTreeItem(cameraTreeItem)
    {
        QList<int> indices;
        foreach (const SceneMotionModel::CameraKeyFramePair &keyframe, keyframes) {
            int frameIndex = keyframe.first;
            const QModelIndex &index = m_smm->frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
            if (index.data(SceneMotionModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
                m_indices.append(index);
            indices.append(frameIndex);
        }
        m_keyframes = keyframes;
        m_frameIndices = indices;
    }
    virtual ~SetKeyframesCommand() {
    }

    virtual void undo() {
        IMotion *motion = m_smm->currentMotion();
        foreach (int frameIndex, m_frameIndices) {
            motion->deleteKeyframes(frameIndex, IKeyframe::kCamera);
            const QModelIndex &index = m_smm->frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
            m_smm->setData(index, QVariant());
        }
        Factory *factory = m_smm->factory();
        QScopedPointer<ICameraKeyframe> newCameraKeyframe;
        foreach (const QModelIndex &index, m_indices) {
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
        m_smm->refreshScene();
    }
    virtual void redo() {
        QScopedPointer<ICameraKeyframe> newCameraKeyframe;
        IMotion *motion = m_smm->currentMotion();
        foreach (const SceneMotionModel::CameraKeyFramePair &pair, m_keyframes) {
            int frameIndex = pair.first;
            const SceneMotionModel::CameraKeyFramePtr &ptr = pair.second;
            ICameraKeyframe *cameraKeyframe = ptr.data();
            const QModelIndex &modelIndex = m_smm->frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
            if (cameraKeyframe->frameIndex() >= 0) {
                QByteArray bytes(cameraKeyframe->estimateSize(), '0');
                newCameraKeyframe.reset(cameraKeyframe->clone());
                newCameraKeyframe->setFrameIndex(frameIndex);
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
        m_smm->refreshScene();
    }

private:
    QList<int> m_frameIndices;
    QModelIndexList m_indices;
    SceneMotionModel::CameraKeyFramePairList m_keyframes;
    SceneMotionModel *m_smm;
    SceneMotionModel::ITreeItem *m_cameraTreeItem;
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
    QScopedPointer<RootTreeItem> rootTreeItem(new RootTreeItem());
    m_cameraTreeItem = new CameraTreeItem(rootTreeItem.data());
    m_lightTreeItem = new LightTreeItem(rootTreeItem.data());
    m_rootTreeItem = rootTreeItem.take();
    m_rootTreeItem->addChild(m_cameraTreeItem);
    //root->addChild(light);
}

SceneMotionModel::~SceneMotionModel()
{
    // m_cameraTreeItem は m_rootTreeItem に子供として追加されてるので、delete するときに同時に delete される
    delete m_lightTreeItem;
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
    return maxFrameCount() + 2;
}

int SceneMotionModel::maxFrameCount() const
{
    return 54000;
}

int SceneMotionModel::maxFrameIndex() const
{
    return m_motion ? m_motion->maxFrameIndex() : 0;
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
    /* 照明データ未対応 */
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
    setModified(false);
}

void SceneMotionModel::addKeyframesByModelIndices(const QModelIndexList &indices)
{
    CameraKeyFramePairList sceneFrames;
    QScopedPointer<ICameraKeyframe> cameraKeyframe;
    QScopedPointer<ILightKeyframe> lightKeyframe;
    foreach (const QModelIndex &index, indices) {
        ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
        int frameIndex = toFrameIndex(index);
        if (frameIndex >= 0 && item->isCategory()) {
            if (index.row() == m_cameraTreeItem->rowIndex()) {
                const Scene::ICamera *camera = m_sceneWidget->sceneLoader()->scene()->camera();
                cameraKeyframe.reset(m_factory->createCameraKeyframe());
                cameraKeyframe->setDefaultInterpolationParameter();
                cameraKeyframe->setPosition(camera->position());
                cameraKeyframe->setAngle(camera->angle());
                cameraKeyframe->setFovy(camera->fov());
                cameraKeyframe->setDistance(camera->distance());
                sceneFrames.append(CameraKeyFramePair(frameIndex, CameraKeyFramePtr(cameraKeyframe.take())));
            }
            else if (index.row() == m_lightTreeItem->rowIndex()) {
                const Scene::ILight *light = m_sceneWidget->sceneLoader()->scene()->light();
                lightKeyframe.reset(m_factory->createLightKeyframe());
                lightKeyframe->setColor(light->color());
                lightKeyframe->setDirection(light->direction());
                //sceneFrames.append(CameraKeyFramePair(frameIndex, CameraKeyFramePtr(lightKeyframe.take())));
            }
        }
    }
    setKeyframes(sceneFrames);
}

void SceneMotionModel::copyKeyframesByModelIndices(const QModelIndexList & /* indices */, int frameIndex)
{
    /* 照明データ未対応 */
    m_cameraIndex = frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
}

void SceneMotionModel::pasteKeyframesByFrameIndex(int frameIndex)
{
    /* 照明データ未対応 */
    if (m_cameraIndex.isValid()) {
        const QVariant &variant = m_cameraIndex.data(SceneMotionModel::kBinaryDataRole);
        if (variant.canConvert(QVariant::ByteArray)) {
            CameraKeyFramePairList keyframes;
            CameraKeyFramePtr keyframe(m_factory->createCameraKeyframe());
            keyframe->read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
            keyframes.append(CameraKeyFramePair(frameIndex, keyframe));
            addUndoCommand(new SetKeyframesCommand(this, keyframes, m_cameraTreeItem));
        }
    }
}

const QString SceneMotionModel::nameFromModelIndex(const QModelIndex & /* index */) const
{
    return QString();
}

SceneMotionModel::CameraKeyFramePairList SceneMotionModel::keyframesFromModelIndices(const QModelIndexList &indices) const
{
    CameraKeyFramePairList keyframes;
    foreach (const QModelIndex &index, indices) {
        const QVariant &variant = index.data(SceneMotionModel::kBinaryDataRole);
        if (variant.canConvert(QVariant::ByteArray)) {
            CameraKeyFramePtr keyframe(m_factory->createCameraKeyframe());
            keyframe->read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
            keyframes.append(CameraKeyFramePair(MotionBaseModel::toFrameIndex(index), keyframe));
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
        /* カメラのキーフレームをテーブルのモデルデータにコピーする */
        QScopedPointer<ICameraKeyframe> newCameraKeyframe;
        for (int i = 0; i < nCameraFrames; i++) {
            const ICameraKeyframe *cameraKeyframe = motion->findCameraKeyframeAt(i);
            int frameIndex = static_cast<int>(cameraKeyframe->frameIndex());
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
            int frameIndex = static_cast<int>(lightKeyframe->frameIndex());
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
    }
    reset();
}

void SceneMotionModel::setKeyframes(const CameraKeyFramePairList &frames)
{
    /* 照明データ未対応 */
    if (m_motion)
        addUndoCommand(new SetKeyframesCommand(this, frames, m_cameraTreeItem));
    else
        qWarning("No motion to register camera frames.");
}

void SceneMotionModel::setActiveUndoStack()
{
    m_undo->setActiveStack(m_stack);
}

void SceneMotionModel::refreshScene()
{
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
    CameraKeyFramePairList cameraKeyframes;
    LightKeyFramePairList lightKeyframes;
    foreach (const QModelIndex &index, indices) {
        if (index.isValid() && index.column() > 1) {
            ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
            if (item->isCategory()) {
                if (index.row() == m_cameraTreeItem->rowIndex()) {
                    ICameraKeyframe *frameToDelete = m_motion->findCameraKeyframe(toFrameIndex(index));
                    CameraKeyFramePtr clonedCameraKeyframe(frameToDelete->clone());
                    /* SetFramesCommand で削除するので削除に必要な条件である frameIndex を 0 未満の値にしておく */
                    clonedCameraKeyframe->setFrameIndex(-1);
                    cameraKeyframes.append(CameraKeyFramePair(frameToDelete->frameIndex(), clonedCameraKeyframe));
                }
                else if (index.row() == m_lightTreeItem->rowIndex()) {
                    ILightKeyframe *frameToDelete = m_motion->findLightKeyframe(toFrameIndex(index));
                    LightKeyFramePtr clonedLightKeyframe(frameToDelete->clone());
                    /* SetFramesCommand で削除するので削除に必要な条件である frameIndex を 0 未満の値にしておく */
                    clonedLightKeyframe->setFrameIndex(-1);
                    lightKeyframes.append(LightKeyFramePair(frameToDelete->frameIndex(), clonedLightKeyframe));
                }
            }
        }
    }
    addUndoCommand(new SetKeyframesCommand(this, cameraKeyframes, m_cameraTreeItem));
}

void SceneMotionModel::applyKeyframeWeightByModelIndices(const QModelIndexList & /* indices */, float /* value */)
{
    /* 現在この処理はまだ何も無い */
}

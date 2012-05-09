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
class SetFramesCommand : public QUndoCommand
{
public:
    SetFramesCommand(SceneMotionModel *smm,
                     const SceneMotionModel::KeyFramePairList &frames,
                     SceneMotionModel::ITreeItem *cameraTreeItem)
        : QUndoCommand(),
          m_smm(smm),
          m_cameraTreeItem(cameraTreeItem),
          m_parameter(smm->cameraInterpolationParameter())
    {
        QList<int> indices;
        foreach (const SceneMotionModel::KeyFramePair &frame, frames) {
            int frameIndex = frame.first;
            const QModelIndex &index = m_smm->frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
            if (index.data(SceneMotionModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
                m_indices.append(index);
            indices.append(frameIndex);
        }
        m_frames = frames;
        m_frameIndices = indices;
    }
    virtual ~SetFramesCommand() {
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
        foreach (const SceneMotionModel::KeyFramePair &pair, m_frames) {
            int frameIndex = pair.first;
            const SceneMotionModel::KeyFramePtr &ptr = pair.second;
            ICameraKeyframe *cameraKeyframe = reinterpret_cast<ICameraKeyframe *>(ptr.data());
            const QModelIndex &modelIndex = m_smm->frameIndexToModelIndex(m_cameraTreeItem, frameIndex);
            if (cameraKeyframe->frameIndex() >= 0) {
                QByteArray bytes(cameraKeyframe->estimateSize(), '0');
                newCameraKeyframe.reset(cameraKeyframe->clone());
                newCameraKeyframe->setInterpolationParameter(ICameraKeyframe::kX, m_parameter.x);
                newCameraKeyframe->setInterpolationParameter(ICameraKeyframe::kY, m_parameter.y);
                newCameraKeyframe->setInterpolationParameter(ICameraKeyframe::kZ, m_parameter.z);
                newCameraKeyframe->setInterpolationParameter(ICameraKeyframe::kRotation, m_parameter.rotation);
                newCameraKeyframe->setInterpolationParameter(ICameraKeyframe::kFovy, m_parameter.fovy);
                newCameraKeyframe->setInterpolationParameter(ICameraKeyframe::kDistance, m_parameter.distance);
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
    SceneMotionModel::KeyFramePairList m_frames;
    SceneMotionModel *m_smm;
    SceneMotionModel::ITreeItem *m_cameraTreeItem;
    ICameraKeyframe::InterpolationParameter m_parameter;
};

}

SceneMotionModel::SceneMotionModel(Factory *factory,
                                   QUndoGroup *undo,
                                   const SceneWidget *sceneWidget,
                                   QObject *parent)
    : MotionBaseModel(undo, parent),
      m_sceneWidget(sceneWidget),
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
        QScopedPointer<ICameraKeyframe> newFrame;
        foreach (const QVariant &value, m_cameraData) {
            const QByteArray &bytes = value.toByteArray();
            newFrame.reset(m_factory->createCameraKeyframe());
            newFrame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            motion->addKeyframe(newFrame.take());
        }
        /* addKeyframe によって変更が必要になる内部インデックスの更新を行うため、update をかけておく */
        motion->update(IKeyframe::kCamera);
    }
    setModified(false);
}

void SceneMotionModel::addKeyframesByModelIndices(const QModelIndexList &indices)
{
    KeyFramePairList sceneFrames;
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
                cameraKeyframe->setFovy(camera->fovy());
                cameraKeyframe->setDistance(camera->distance());
                sceneFrames.append(KeyFramePair(frameIndex, KeyFramePtr(cameraKeyframe.take())));
            }
            else if (index.row() == m_lightTreeItem->rowIndex()) {
                const Scene::ILight *light = m_sceneWidget->sceneLoader()->scene()->light();
                lightKeyframe.reset(m_factory->createLightKeyframe());
                lightKeyframe->setColor(light->color());
                lightKeyframe->setDirection(light->direction());
                sceneFrames.append(KeyFramePair(frameIndex, KeyFramePtr(lightKeyframe.take())));
            }
        }
    }
    setFrames(sceneFrames);
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
            KeyFramePairList keyframes;
            KeyFramePtr keyframe(m_factory->createCameraKeyframe());
            keyframe->read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
            keyframes.append(KeyFramePair(frameIndex, keyframe));
            addUndoCommand(new SetFramesCommand(this, keyframes, m_cameraTreeItem));
        }
    }
}

const QString SceneMotionModel::nameFromModelIndex(const QModelIndex & /* index */) const
{
    return QString();
}

SceneMotionModel::KeyFramePairList SceneMotionModel::keyframesFromModelIndices(const QModelIndexList &indices) const
{
    KeyFramePairList keyframes;
    foreach (const QModelIndex &index, indices) {
        const QVariant &variant = index.data(SceneMotionModel::kBinaryDataRole);
        if (variant.canConvert(QVariant::ByteArray)) {
            KeyFramePtr keyframe(m_factory->createCameraKeyframe());
            keyframe->read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
            keyframes.append(KeyFramePair(MotionBaseModel::toFrameIndex(index), keyframe));
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
        m_motion = motion;
    }
    reset();
}

void SceneMotionModel::setFrames(const KeyFramePairList &frames)
{
    /* 照明データ未対応 */
    if (m_motion)
        addUndoCommand(new SetFramesCommand(this, frames, m_cameraTreeItem));
    else
        qWarning("No motion to register camera frames.");
}

void SceneMotionModel::refreshScene()
{
    reset();
}

void SceneMotionModel::removeMotion()
{
    m_cameraData.clear();
    m_lightData.clear();
    setModified(false);
    reset();
}

void SceneMotionModel::deleteKeyframesByModelIndices(const QModelIndexList &indices)
{
    /* 照明データ未対応 */
    KeyFramePairList frames;
    QScopedPointer<ICameraKeyframe> clonedCameraKeyframe;
    QScopedPointer<ILightKeyframe> clonedLightKeyframe;
    foreach (const QModelIndex &index, indices) {
        if (index.isValid() && index.column() > 1) {
            ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
            if (item->isCategory()) {
                if (index.row() == m_cameraTreeItem->rowIndex()) {
                    ICameraKeyframe *frameToDelete = m_motion->findCameraKeyframe(toFrameIndex(index));
                    clonedCameraKeyframe.reset(frameToDelete->clone());
                    /* SetFramesCommand で削除するので削除に必要な条件である frameIndex を 0 未満の値にしておく */
                    clonedCameraKeyframe->setFrameIndex(-1);
                    frames.append(KeyFramePair(frameToDelete->frameIndex(), KeyFramePtr(clonedCameraKeyframe.take())));
                }
                else if (index.row() == m_lightTreeItem->rowIndex()) {
                    ILightKeyframe *frameToDelete = m_motion->findLightKeyframe(toFrameIndex(index));
                    clonedLightKeyframe.reset(frameToDelete->clone());
                    /* SetFramesCommand で削除するので削除に必要な条件である frameIndex を 0 未満の値にしておく */
                    clonedLightKeyframe->setFrameIndex(-1);
                    frames.append(KeyFramePair(frameToDelete->frameIndex(), KeyFramePtr(clonedLightKeyframe.take())));
                }
            }
        }
    }
    addUndoCommand(new SetFramesCommand(this, frames, m_cameraTreeItem));
}

void SceneMotionModel::applyKeyframeWeightByModelIndices(const QModelIndexList & /* indices */, float /* value */)
{
    /* 現在この処理はまだ何も無い */
}

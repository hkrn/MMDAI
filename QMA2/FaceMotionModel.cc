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

#include "FaceMotionModel.h"
#include "util.h"
#include <vpvl/vpvl.h>

namespace
{

class TreeItem : public MotionBaseModel::ITreeItem
{
public:
    TreeItem(const QString &name, vpvl::Face *face, bool isRoot, TreeItem *parent)
        : m_name(name),
          m_parent(parent),
          m_face(face),
          m_isRoot(isRoot)
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
    vpvl::Face *face() const {
        return m_face;
    }
    bool isRoot() const {
        return m_isRoot;
    }
    int rowIndex() const {
        return m_parent ? m_parent->m_children.indexOf(const_cast<TreeItem *>(this)) : 0;
    }
    int countChildren() const {
        return m_children.count();
    }

private:
    QList<TreeItem *> m_children;
    QString m_name;
    TreeItem *m_parent;
    vpvl::Face *m_face;
    bool m_isRoot;
};

class SetFramesCommand : public QUndoCommand
{
public:
    SetFramesCommand(FaceMotionModel *bmm, const FaceMotionModel::KeyFramePairList &frames)
        : QUndoCommand(),
          m_fmm(bmm)
    {
        QHash<int, bool> indexProceeded;
        const MotionBaseModel::TreeItemList &items = m_fmm->keys().values();
        foreach (const FaceMotionModel::KeyFramePair &frame, frames) {
            int frameIndex = frame.first;
            if (!indexProceeded[frameIndex]) {
                foreach (MotionBaseModel::ITreeItem *item, items) {
                    const QModelIndex &index = m_fmm->frameToIndex(item, frameIndex);
                    if (index.data(FaceMotionModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
                        m_indices.append(index);
                }
                indexProceeded[frameIndex] = true;
            }
        }
        m_frames = frames;
        m_frameIndices = indexProceeded.keys();
        execute();
    }
    virtual ~SetFramesCommand() {
    }

    virtual void undo() {
        vpvl::FaceAnimation *animation = m_fmm->currentMotion()->mutableFaceAnimation();
        foreach (int frameIndex, m_frameIndices) {
            animation->deleteKeyFrames(frameIndex);
            foreach (MotionBaseModel::ITreeItem *item, m_fmm->keys().values()) {
                const QModelIndex &index = m_fmm->frameToIndex(item, frameIndex);
                m_fmm->setData(index, FaceMotionModel::kInvalidData, Qt::EditRole);
            }
        }
        foreach (const QModelIndex &index, m_indices) {
            const QByteArray &bytes = index.data(FaceMotionModel::kBinaryDataRole).toByteArray();
            m_fmm->setData(index, bytes, Qt::EditRole);
            vpvl::FaceKeyFrame *frame = new vpvl::FaceKeyFrame();
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->addKeyFrame(frame);
        }
        animation->refresh();
        m_fmm->refreshModel();
    }
    virtual void redo() {
        execute();
    }

private:
    void execute() {
        QString key;
        vpvl::FaceAnimation *animation = m_fmm->currentMotion()->mutableFaceAnimation();
        const FaceMotionModel::Keys &keys = m_fmm->keys();
        vpvl::Face *selected = m_fmm->selectedFace();
        foreach (const FaceMotionModel::KeyFramePair &pair, m_frames) {
            int frameIndex = pair.first;
            FaceMotionModel::KeyFramePtr ptr = pair.second;
            vpvl::FaceKeyFrame *frame = ptr.data();
            if (frame) {
                key = internal::toQString(frame->name());
            }
            else if (selected) {
                key = internal::toQString(selected->name());
            }
            else {
                qWarning("No bone is selected or null");
                continue;
            }
            if (keys.contains(key)) {
                const QModelIndex &modelIndex = m_fmm->frameToIndex(keys[key], frameIndex);
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                vpvl::FaceKeyFrame *newFrame = static_cast<vpvl::FaceKeyFrame *>(frame->clone());
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                animation->addKeyFrame(newFrame);
                m_fmm->setData(modelIndex, bytes, Qt::EditRole);
            }
            else {
                qWarning("Tried registering not face key frame: %s", qPrintable(key));
                continue;
            }
        }
        animation->refresh();
        m_fmm->refreshModel();
    }

    QList<int> m_frameIndices;
    QModelIndexList m_indices;
    FaceMotionModel::KeyFramePairList m_frames;
    FaceMotionModel *m_fmm;
};

class ResetAllCommand : public QUndoCommand
{
public:
    ResetAllCommand(vpvl::PMDModel *model)
        : QUndoCommand(),
          m_model(model)
    {
        m_state = model->saveState();
    }
    virtual ~ResetAllCommand() {
        m_model->discardState(m_state);
    }

    void undo() {
        m_model->restoreState(m_state);
        m_model->updateImmediate();
    }
    void redo() {
        execute();
    }

private:
    void execute() {
        m_model->resetAllFaces();
        m_model->updateImmediate();
    }

    vpvl::PMDModel *m_model;
    vpvl::PMDModel::State *m_state;
};

class SetFaceCommand : public QUndoCommand
{
public:
    SetFaceCommand(vpvl::PMDModel *model, vpvl::PMDModel::State *state)
        : QUndoCommand(),
          m_model(model),
          m_newState(0),
          m_oldState(state)
    {
        m_newState = m_model->saveState();
    }
    virtual ~SetFaceCommand() {
        m_model->discardState(m_newState);
        m_model->discardState(m_oldState);
    }

    void undo() {
        m_model->restoreState(m_oldState);
        m_model->updateImmediate();
    }
    void redo() {
        execute();
    }

private:
    void execute() {
        m_model->restoreState(m_newState);
        m_model->updateImmediate();
    }

    vpvl::PMDModel *m_model;
    vpvl::PMDModel::State *m_newState;
    vpvl::PMDModel::State *m_oldState;
};

}

FaceMotionModel::FaceMotionModel(QUndoGroup *undo, QObject *parent)
    : MotionBaseModel(undo, parent),
      m_state(0)
{
    m_root = new TreeItem("", 0, true, 0);
}

FaceMotionModel::~FaceMotionModel()
{
    m_frames.releaseAll();
}

void FaceMotionModel::saveMotion(vpvl::VMDMotion *motion)
{
    if (m_model) {
        vpvl::FaceAnimation *animation = motion->mutableFaceAnimation();
        foreach (QVariant value, values()) {
            vpvl::FaceKeyFrame *newFrame = new vpvl::FaceKeyFrame();
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

void FaceMotionModel::copyFrames(int frameIndex)
{
    if (m_model && m_motion) {
        m_frames.releaseAll();
        m_motion->faceAnimation().copyKeyFrames(frameIndex, m_frames);
    }
}

void FaceMotionModel::pasteFrame(int frameIndex)
{
    if (m_model && m_motion && m_frames.count() != 0) {
        FaceMotionModel::KeyFramePairList frames;
        const int nframes = m_frames.count();
        for (int i = 0; i < nframes; i++) {
            vpvl::FaceKeyFrame *frame = static_cast<vpvl::FaceKeyFrame *>(m_frames[i]->clone());
            frames.append(KeyFramePair(frameIndex, KeyFramePtr(frame)));
        }
        addUndoCommand(new SetFramesCommand(this, frames));
    }
}

void FaceMotionModel::startTransform()
{
    if (m_model) {
        m_model->discardState(m_state);
        m_state = m_model->saveState();
    }
}

void FaceMotionModel::commitTransform()
{
    if (m_model && m_state) {
        addUndoCommand(new SetFaceCommand(m_model, m_state));
        m_state = 0;
    }
}

void FaceMotionModel::setFrames(const KeyFramePairList &frames)
{
    if (m_model && m_motion) {
        addUndoCommand(new SetFramesCommand(this, frames));
    }
    else {
        qWarning("No model or motion to register face frames.");
        return;
    }
}

void FaceMotionModel::resetAllFaces()
{
    if (m_model)
        addUndoCommand(new ResetAllCommand(m_model));
}

void FaceMotionModel::setPMDModel(vpvl::PMDModel *model)
{
    clearKeys();
    if (model) {
        const vpvl::FaceList &faces = model->facesForUI();
        const int nfaces = faces.count();
        TreeItem *eyeblow = new TreeItem(tr("Eyeblow"), 0, false, static_cast<TreeItem *>(m_root));
        TreeItem *eye = new TreeItem(tr("Eye"), 0, false, static_cast<TreeItem *>(m_root));
        TreeItem *lip = new TreeItem(tr("Lip"), 0, false, static_cast<TreeItem *>(m_root));
        TreeItem *other = new TreeItem(tr("Other"), 0, false, static_cast<TreeItem *>(m_root));
        Keys &keys = m_keys[model];
        for (int i = 0; i < nfaces; i++) {
            vpvl::Face *face = faces[i];
            const QString &name = internal::toQString(face);
            TreeItem *child, *parent = 0;
            switch (face->type()) {
            case vpvl::Face::kEyeblow:
                parent = eyeblow;
                break;
            case vpvl::Face::kEye:
                parent = eye;
                break;
            case vpvl::Face::kLip:
                parent = lip;
                break;
            case vpvl::Face::kOther:
                parent = other;
                break;
            default:
                break;
            }
            if (parent) {
                child = new TreeItem(name, face, false, parent);
                parent->addChild(child);
                keys.insert(name, child);
            }
        }
        m_root->addChild(eyeblow);
        m_root->addChild(eye);
        m_root->addChild(lip);
        m_root->addChild(other);
        MotionBaseModel::setPMDModel(model);
        qDebug("Set a model in FaceMotionModel: %s", qPrintable(internal::toQString(model)));
    }
    reset();
}

void FaceMotionModel::loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (model == m_model) {
        const vpvl::FaceAnimation &animation = motion->faceAnimation();
        const int nFaceFrames = animation.countKeyFrames();
        for (int i = 0; i < nFaceFrames; i++) {
            vpvl::FaceKeyFrame *frame = animation.frameAt(i);
            const uint8_t *name = frame->name();
            const QString &key = internal::toQString(name);
            const Keys &keys = this->keys();
            if (keys.contains(key)) {
                int frameIndex = static_cast<int>(frame->frameIndex());
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                ITreeItem *item = keys[key];
                const QModelIndex &modelIndex = frameToIndex(item, frameIndex);
                vpvl::FaceKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setWeight(frame->weight());
                newFrame.setFrameIndex(frameIndex);
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes);
            }
        }
        m_motion = motion;
        refreshModel();
        setModified(false);
        qDebug("Loaded a motion to the model in FaceMotionModel: %s", qPrintable(internal::toQString(model)));
    }
    else {
        qDebug("Tried loading a motion to different model, ignored: %s", qPrintable(internal::toQString(model)));
    }
}

void FaceMotionModel::deleteMotion()
{
    m_selected.clear();
    clearValues();
    setModified(false);
    reset();
    resetAllFaces();
}

void FaceMotionModel::deleteModel()
{
    deleteMotion();
    clearKeys();
    delete m_root;
    m_root = new TreeItem("", 0, true, 0);
    setPMDModel(0);
    reset();
}

void FaceMotionModel::deleteFrame(const QModelIndex &index)
{
    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    vpvl::Face *face = item->face();
    m_motion->mutableFaceAnimation()->deleteKeyFrame(index.column(), face->name());
    setData(index, kInvalidData, Qt::EditRole);
}

void FaceMotionModel::selectFaces(const QList<vpvl::Face *> &faces)
{
    m_selected = faces;
    emit facesDidSelect(faces);
}

vpvl::Face *FaceMotionModel::findFace(const QString &name)
{
    const QByteArray &bytes = internal::getTextCodec()->fromUnicode(name);
    foreach (ITreeItem *item, keys()) {
        vpvl::Face *face = static_cast<TreeItem *>(item)->face();
        if (!qstrcmp(reinterpret_cast<const char *>(face->name()), bytes))
            return face;
    }
    return 0;
}

void FaceMotionModel::setWeight(float value)
{
    if (!m_selected.isEmpty())
        setWeight(value, m_selected.last());
}

void FaceMotionModel::setWeight(float value, vpvl::Face *face)
{
    if (face) {
        face->setWeight(value);
        updateModel();
    }
}

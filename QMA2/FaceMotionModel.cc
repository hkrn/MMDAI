#include "FaceMotionModel.h"
#include "util.h"
#include <vpvl/vpvl.h>

namespace
{

class CopyFramesCommand : public QUndoCommand
{
public:
    CopyFramesCommand(FaceMotionModel *model)
        : QUndoCommand(),
          m_model(model)
    {
    }
    virtual ~CopyFramesCommand() {}

    void undo() {}
    void redo() {}

private:
    FaceMotionModel *m_model;
};

class SetFramesCommand : public QUndoCommand
{
public:
    SetFramesCommand(FaceMotionModel *model)
        : QUndoCommand(),
          m_model(model)
    {
    }
    virtual ~SetFramesCommand() {}

    void undo() {}
    void redo() {}

private:
    FaceMotionModel *m_model;
};

class ResetAllCommand : public QUndoCommand
{
public:
    ResetAllCommand(FaceMotionModel *model)
        : QUndoCommand(),
          m_model(model)
    {
    }
    virtual ~ResetAllCommand() {}

    void undo() {}
    void redo() {}

private:
    FaceMotionModel *m_model;
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
}

FaceMotionModel::~FaceMotionModel()
{
}

void FaceMotionModel::saveMotion(vpvl::VMDMotion *motion)
{
    if (m_model) {
        vpvl::FaceAnimation *animation = motion->mutableFaceAnimation();
        foreach (QVariant value, values()) {
            vpvl::FaceKeyFrame *newFrame = new vpvl::FaceKeyFrame();
            const QByteArray &bytes = value.toByteArray();
            newFrame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->addFrame(newFrame);
        }
        setModified(false);
    }
    else {
        qWarning("No model is selected to save motion.");
    }
}

void FaceMotionModel::copyFrames(int /* frameIndex */)
{
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

void FaceMotionModel::setFrames(const QList<Frame> &frames)
{
    if (!m_model || !m_motion) {
        qWarning("No model or motion to register a bone frame.");
        return;
    }
    QString key;
    foreach (Frame pair, frames) {
        int frameIndex = pair.first;
        vpvl::Face *face = pair.second;
        key = internal::toQString(face->name());
        int i = keys().indexOf(key);
        if (i != -1) {
            const QModelIndex &modelIndex = index(i, frameIndex);
            vpvl::FaceAnimation *animation = m_motion->mutableFaceAnimation();
            vpvl::FaceKeyFrame *newFrame = new vpvl::FaceKeyFrame();
            newFrame->setName(face->name());
            newFrame->setWeight(face->weight());
            newFrame->setFrameIndex(frameIndex);
            QByteArray bytes(vpvl::FaceKeyFrame::strideSize(), '0');
            newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
            animation->addFrame(newFrame);
            animation->refresh();
            setData(modelIndex, bytes, Qt::EditRole);
        }
        else {
            qWarning("Tried registering not face key frame: %s", qPrintable(key));
        }
    }
}

void FaceMotionModel::resetAllFaces()
{
    if (m_model) {
        m_model->resetAllFaces();
        updateModel();
    }
}

void FaceMotionModel::setPMDModel(vpvl::PMDModel *model)
{
    m_faces.clear();
    clearKeys();
    if (model) {
        const vpvl::FaceList &faces = model->facesForUI();
        uint32_t nFaces = faces.count();
        Keys k = keys(model);
        for (uint32_t i = 0; i < nFaces; i++) {
            vpvl::Face *face = faces[i];
            appendKey(internal::toQString(face), model);
            m_faces.append(face);
        }
    }
    MotionBaseModel::setPMDModel(model);
    qDebug("Set a model in FaceMotionModel: %s", qPrintable(internal::toQString(model)));
    reset();
}

void FaceMotionModel::loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (model == m_model) {
        const vpvl::FaceAnimation &animation = motion->faceAnimation();
        uint32_t nFaceFrames = animation.countFrames();
        for (uint32_t i = 0; i < nFaceFrames; i++) {
            vpvl::FaceKeyFrame *frame = animation.frameAt(i);
            const uint8_t *name = frame->name();
            QString key = internal::toQString(name);
            int i = keys().indexOf(key);
            if (i != -1) {
                uint32_t frameIndex = frame->frameIndex();
                QModelIndex modelIndex = index(i, frameIndex);
                vpvl::FaceKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setWeight(frame->weight());
                newFrame.setFrameIndex(frameIndex);
                QByteArray bytes(vpvl::FaceKeyFrame::strideSize(), '0');
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes, Qt::EditRole);
            }
        }
        m_motion = motion;
        refreshModel();
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
    m_faces.clear();
    deleteMotion();
    clearKeys();
    setPMDModel(0);
    reset();
}

void FaceMotionModel::deleteFrame(const QModelIndex &index)
{
    m_motion->mutableFaceAnimation()->deleteFrame(index.column(), m_faces[index.row()]->name());
    setData(index, kInvalidData, Qt::EditRole);
}

void FaceMotionModel::selectFaces(QList<vpvl::Face *> faces)
{
    m_selected = faces;
}

vpvl::Face *FaceMotionModel::selectFace(int rowIndex)
{
    m_selected.clear();
    vpvl::Face *face = m_faces[rowIndex];
    m_selected.append(face);
    return face;
}

vpvl::Face *FaceMotionModel::findFace(const QString &name)
{
    QByteArray bytes = internal::getTextCodec()->fromUnicode(name);
    foreach (vpvl::Face *face, m_faces) {
        if (!qstrcmp(reinterpret_cast<const char *>(face->name()), bytes))
            return face;
    }
    return 0;
}

QList<vpvl::Face *> FaceMotionModel::facesByIndices(const QModelIndexList &indices)
{
    QList<vpvl::Face *> faces;
    foreach (QModelIndex index, indices) {
        if (index.isValid())
            faces.append(m_faces[index.row()]);
    }
    return faces;
}

QList<vpvl::Face *> FaceMotionModel::facesFromIndices(const QModelIndexList &indices)
{
    QList<vpvl::Face *> faces;
    foreach (QModelIndex index, indices)
        faces.append(index.isValid() ? m_faces[index.row()] : 0);
    return faces;
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

void FaceMotionModel::clearKeys()
{
    m_faces.clear();
    MotionBaseModel::clearKeys();
}

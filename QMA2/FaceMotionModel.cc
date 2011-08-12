#include "FaceMotionModel.h"
#include "util.h"
#include <vpvl/vpvl.h>

FaceMotionModel::FaceMotionModel(QObject *parent) :
    MotionBaseModel(parent)
{
}

void FaceMotionModel::saveMotion(vpvl::VMDMotion *motion)
{
    vpvl::FaceAnimation *animation = motion->mutableFaceAnimation();
    foreach (QVariant value, m_values) {
        vpvl::FaceKeyFrame *newFrame = new vpvl::FaceKeyFrame();
        QByteArray bytes = value.toByteArray();
        newFrame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
        animation->addFrame(newFrame);
    }
}

void FaceMotionModel::registerKeyFrame(vpvl::Face *face, int frameIndex)
{
    QString key = internal::toQString(face->name());
    int i = m_keys.indexOf(key);
    if (i != -1) {
        QModelIndex modelIndex = index(i, frameIndex);
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
        qWarning("tried registering not face key frame: %s", key.toUtf8().constData());
    }
}

void FaceMotionModel::setPMDModel(vpvl::PMDModel *model)
{
    m_keys.clear();
    m_faces.clear();
    if (model) {
        vpvl::FaceList faces = model->facesForUI();
        uint32_t nFaces = faces.size();
        for (uint32_t i = 0; i < nFaces; i++) {
            vpvl::Face *face = faces[i];
            m_keys.append(internal::toQString(face));
            m_faces.append(face);
        }
        reset();
    }
    m_model = model;
    emit modelDidChange(model);
}

bool FaceMotionModel::loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (model == m_model) {
        const vpvl::FaceAnimation &animation = motion->faceAnimation();
        uint32_t nFaceFrames = animation.countFrames();
        for (uint32_t i = 0; i < nFaceFrames; i++) {
            vpvl::FaceKeyFrame *frame = animation.frameAt(i);
            const uint8_t *name = frame->name();
            QString key = internal::toQString(name);
            int i = m_keys.indexOf(key);
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
        reset();
        return true;
    }
    else {
        return false;
    }
}

void FaceMotionModel::clear()
{
    m_faces.clear();
    m_selected.clear();
    m_keys.clear();
    m_values.clear();
    m_model = 0;
    reset();
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

QList<vpvl::Face *> FaceMotionModel::facesFromIndices(const QModelIndexList &indices)
{
    QList<vpvl::Face *> faces;
    foreach (QModelIndex index, indices)
        faces.append(index.isValid() ? m_faces[index.row()] : 0);
    return faces;
}

void FaceMotionModel::setWeight(float value)
{
    setWeight(value, m_selected.last());
}

void FaceMotionModel::setWeight(float value, vpvl::Face *face)
{
    if (face) {
        face->setWeight(value);
        updateModel();
    }
}

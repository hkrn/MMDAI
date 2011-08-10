#include "FaceMotionModel.h"
#include "util.h"
#include <vpvl/vpvl.h>

FaceMotionModel::FaceMotionModel(QObject *parent) :
    MotionBaseModel(parent),
    m_selected(0)
{
}

void FaceMotionModel::saveMotion(vpvl::VMDMotion *motion)
{
    vpvl::FaceKeyFrameList *frames = motion->mutableFaceAnimation()->mutableFrames();
    foreach (QVariant value, m_values) {
        vpvl::FaceKeyFrame *newFrame = new vpvl::FaceKeyFrame();
        QByteArray bytes = value.toByteArray();
        newFrame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
        frames->push_back(newFrame);
    }
}

void FaceMotionModel::registerKeyFrame(vpvl::Face *face, int frameIndex)
{
    QString key = internal::toQString(face->name());
    int i = m_keys.indexOf(key);
    if (i != -1) {
        QModelIndex modelIndex = index(i, frameIndex);
        vpvl::FaceAnimation *animation = m_motion->mutableFaceAnimation();
        vpvl::FaceKeyFrameList *frames = animation->mutableFrames();
        vpvl::FaceKeyFrame *newFrame = new vpvl::FaceKeyFrame();
        newFrame->setName(face->name());
        newFrame->setWeight(face->weight());
        newFrame->setFrameIndex(frameIndex);
        QByteArray bytes(vpvl::FaceKeyFrame::stride(), '0');
        newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
        frames->push_back(newFrame);
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
        const vpvl::FaceKeyFrameList faceFrames = motion->faceAnimation().frames();
        uint32_t nFaceFrames = faceFrames.size();
        for (uint32_t i = 0; i < nFaceFrames; i++) {
            vpvl::FaceKeyFrame *frame = faceFrames[i];
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
                QByteArray bytes(vpvl::FaceKeyFrame::stride(), '0');
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

vpvl::Face *FaceMotionModel::selectFace(int rowIndex)
{
    vpvl::Face *face = m_selected = m_faces[rowIndex];
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
    setWeight(value, m_selected);
}

void FaceMotionModel::setWeight(float value, vpvl::Face *face)
{
    if (face) {
        face->setWeight(value);
        updateModel();
    }
}

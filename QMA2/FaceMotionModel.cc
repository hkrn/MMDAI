#include "FaceMotionModel.h"
#include "util.h"
#include <vpvl/vpvl.h>

FaceMotionModel::FaceMotionModel(QObject *parent) :
    MotionBaseModel(parent),
    m_selected(0)
{
}

void FaceMotionModel::registerKeyFrame(vpvl::Face *face, int frameIndex)
{
    QString key = internal::toQString(face->name());
    int i = m_keys.indexOf(key);
    if (i != -1) {
        QModelIndex modelIndex = index(i, frameIndex);
        QByteArray bytes(vpvl::FaceKeyFrame::stride(), '0');
        vpvl::FaceKeyFrame frame;
        frame.setName(face->name());
        frame.setWeight(face->weight());
        frame.setFrameIndex(frameIndex);
        frame.write(reinterpret_cast<uint8_t *>(bytes.data()));
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
        vpvl::FaceList faces = model->faces();
        uint32_t nFaces = faces.size();
        for (uint32_t i = 0; i < nFaces; i++) {
            vpvl::Face *face = faces.at(i);
            if (face->type() != vpvl::Face::kBase) {
                m_keys.append(internal::toQString(face));
                m_faces.append(face);
            }
        }
        reset();
    }
    m_model = model;
}

bool FaceMotionModel::loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (model == m_model) {
        const vpvl::FaceKeyFrameList faceFrames = motion->faceMotion().frames();
        uint32_t nFaceFrames = faceFrames.size();
        for (uint32_t i = 0; i < nFaceFrames; i++) {
            vpvl::FaceKeyFrame *frame = faceFrames[i];
            const uint8_t *name = frame->name();
            QString key = internal::toQString(name);
            int i = m_keys.indexOf(key);
            if (i != -1) {
                uint32_t frameIndex = frame->frameIndex();
                QByteArray bytes(vpvl::FaceKeyFrame::stride(), '0');
                QModelIndex modelIndex = index(i, frameIndex);
                vpvl::FaceKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setWeight(frame->weight());
                newFrame.setFrameIndex(frameIndex);
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes, Qt::EditRole);
            }
        }
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

QList<vpvl::Face *> FaceMotionModel::facesFromIndices(const QModelIndexList &indices)
{
    QList<vpvl::Face *> faces;
    foreach (QModelIndex index, indices)
        faces.append(index.isValid() ? m_faces[index.row()] : 0);
    return faces;
}

void FaceMotionModel::setWeight(float value)
{
    if (m_selected) {
        m_selected->setWeight(value);
        updateModel();
    }
}

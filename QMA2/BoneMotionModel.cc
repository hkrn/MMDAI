#include "BoneMotionModel.h"
#include "util.h"
#include <vpvl/vpvl.h>

BoneMotionModel::BoneMotionModel(QObject *parent) :
    MotionBaseModel(parent),
    m_selected(0)
{
}

void BoneMotionModel::loadPose(vpvl::VPDPose *pose, vpvl::PMDModel *model, int frameIndex)
{
    if (model == m_model) {
        const vpvl::VPDPose::BoneList boneFrames = pose->bones();
        uint32_t nBoneFrames = boneFrames.size();
        for (uint32_t i = 0; i < nBoneFrames; i++) {
            vpvl::VPDPose::Bone *frame = boneFrames[i];
            const uint8_t *name = frame->name;
            QString key = internal::toQString(name);
            int i = m_keys.indexOf(key);
            if (i != -1) {
                QByteArray bytes(vpvl::BoneKeyFrame::stride(), '0');
                QModelIndex modelIndex = index(i, frameIndex);
                btQuaternion rotation;
                const btVector4 &v = frame->rotation;
                rotation.setValue(v.x(), v.y(), v.z(), v.w());
                vpvl::BoneKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setPosition(frame->position);
                newFrame.setRotation(rotation);
                newFrame.setFrameIndex(frameIndex);
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes, Qt::EditRole);
            }
        }
    }
}

void BoneMotionModel::registerKeyFrame(vpvl::Bone *bone, int frameIndex)
{
    QString key = internal::toQString(bone->name());
    int i = m_keys.indexOf(key);
    if (i != -1) {
        QModelIndex modelIndex = index(i, frameIndex);
        QByteArray bytes(vpvl::BoneKeyFrame::stride(), '0');
        vpvl::BoneKeyFrame frame;
        frame.setName(bone->name());
        frame.setPosition(bone->position());
        frame.setRotation(bone->rotation());
        frame.setFrameIndex(frameIndex);
        frame.write(reinterpret_cast<uint8_t *>(bytes.data()));
        setData(modelIndex, bytes, Qt::EditRole);
    }
    else {
        qWarning("tried registering not bone key frame: %s", key.toUtf8().constData());
    }
}

void BoneMotionModel::setPMDModel(vpvl::PMDModel *model)
{
    m_keys.clear();
    m_bones.clear();
    if (model) {
        vpvl::BoneList bones = model->bones();
        uint32_t nBones = bones.size();
        for (uint32_t i = 0; i < nBones; i++) {
            vpvl::Bone *bone = bones.at(i);
            if (bone->isVisible()) {
                m_keys.append(internal::toQString(bone));
                m_bones.append(bone);
            }
        }
        reset();
    }
    m_model = model;
}

void BoneMotionModel::loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (model == m_model) {
        const vpvl::BoneKeyFrameList boneFrames = motion->bone().frames();
        uint32_t nBoneFrames = boneFrames.size();
        for (uint32_t i = 0; i < nBoneFrames; i++) {
            vpvl::BoneKeyFrame *frame = boneFrames[i];
            const uint8_t *name = frame->name();
            QString key = internal::toQString(name);
            int i = m_keys.indexOf(key);
            if (i != -1) {
                uint32_t frameIndex = frame->frameIndex();
                QByteArray bytes(vpvl::BoneKeyFrame::stride(), '0');
                QModelIndex modelIndex = index(i, frameIndex);
                vpvl::BoneKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setPosition(frame->position());
                newFrame.setRotation(frame->rotation());
                newFrame.setFrameIndex(frameIndex);
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes, Qt::EditRole);
            }
        }
    }
}

void BoneMotionModel::resetBone(ResetType type)
{
    if (m_selected) {
        btVector3 pos = m_selected->position();
        btQuaternion rot = m_selected->rotation();
        switch (type) {
        case kX:
            pos.setX(0.0f);
            m_selected->setPosition(pos);
            break;
        case kY:
            pos.setY(0.0f);
            m_selected->setPosition(pos);
            break;
        case kZ:
            pos.setZ(0.0f);
            m_selected->setPosition(pos);
            break;
        case kRotation:
            rot.setValue(0.0f, 0.0f, 0.0f, 1.0f);
            m_selected->setRotation(rot);
            break;
        default:
            qFatal("Unexpected reset bone type: %d", type);
        }
    }
}

vpvl::Bone *BoneMotionModel::selectBone(int rowIndex)
{
    vpvl::Bone *bone = m_selected = m_bones[rowIndex];
    return bone;
}

QList<vpvl::Bone *> BoneMotionModel::bonesFromIndices(const QModelIndexList &indices)
{
    QList<vpvl::Bone *> bones;
    foreach (QModelIndex index, indices) {
        bones.append(index.isValid() ? m_bones[index.row()] : 0);
    }
    return bones;
}

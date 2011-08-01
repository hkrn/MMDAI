#include "BoneMotionModel.h"
#include "util.h"
#include <vpvl/vpvl.h>

BoneMotionModel::BoneMotionModel(QObject *parent) :
    MotionBaseModel(parent),
    m_selected(0),
    m_mode(kLocal)
{
}

bool BoneMotionModel::loadPose(vpvl::VPDPose *pose, vpvl::PMDModel *model, int frameIndex)
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
        return true;
    }
    else {
        return false;
    }
}

bool BoneMotionModel::registerKeyFrame(vpvl::Bone *bone, int frameIndex)
{
    QString key;
    if (bone) {
        key = internal::toQString(bone->name());
    }
    else if (m_selected) {
        key = internal::toQString(m_selected->name());
    }
    else {
        qWarning("bone is not selected or null");
        return false;
    }
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
        return true;
    }
    else {
        qWarning("tried registering not bone key frame: %s", key.toUtf8().constData());
        return false;
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

bool BoneMotionModel::loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (model == m_model) {
        const vpvl::BoneKeyFrameList boneFrames = motion->boneMotion().frames();
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
        return true;
    }
    else {
        return false;
    }
}

bool BoneMotionModel::resetBone(ResetType type)
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
        return updateModel();
    }
    else {
        return false;
    }
}

bool BoneMotionModel::resetAllBones()
{
    if (m_model) {
        m_model->smearAllBonesToDefault(0.0f);
        return updateModel();
    }
    else {
        return false;
    }
}

void BoneMotionModel::setMode(int value)
{
    switch (value) {
    case 0:
        m_mode = kLocal;
        break;
    case 1:
        m_mode = kGlobal;
        break;
    case 2:
        m_mode = kView;
        break;
    }
}

void BoneMotionModel::setPosition(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    btVector3 pos = m_selected->position();
    switch (coordinate) {
    case 'x':
    case 'X':
        pos.setX(value);
        break;
    case 'y':
    case 'Y':
        pos.setY(value);
        break;
    case 'z':
    case 'Z':
        pos.setZ(value);
        break;
    default:
        qFatal("Unexpected coordinate value: %c", coordinate);
    }
    m_selected->setPosition(pos);
    updateModel();
    emit bonePositionDidChange(pos);
}

void BoneMotionModel::setRotation(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    btQuaternion rot = m_selected->rotation();
    switch (coordinate) {
    case 'x':
    case 'X':
        rot.setX(value);
        break;
    case 'y':
    case 'Y':
        rot.setY(value);
        break;
    case 'z':
    case 'Z':
        rot.setZ(value);
        break;
    default:
        qFatal("Unexpected coordinate value: %c", coordinate);
    }
    m_selected->setRotation(rot);
    updateModel();
    emit boneRotationDidChange(rot);
}

void BoneMotionModel::transform(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    btVector3 current = m_selected->position(), pos, dest;
    switch (coordinate) {
    case 'x':
    case 'X':
        pos.setValue(value, 0, 0);
        break;
    case 'y':
    case 'Y':
        pos.setValue(0, value, 0);
        break;
    case 'z':
    case 'Z':
        pos.setValue(0, 0, value);
        break;
    default:
        qFatal("Unexpected coordinate value: %c", coordinate);
    }
    switch (m_mode) {
    case kLocal:
        dest = btTransform(m_selected->rotation(), current) * pos;
        break;
    case kGlobal:
        dest = current + pos;
        break;
    default:
        break;
    }
    m_selected->setPosition(dest);
    updateModel();
    emit bonePositionDidChange(dest);
}

void BoneMotionModel::rotate(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    btQuaternion current = m_selected->rotation(), rot, dest;
    switch (coordinate) {
    case 'x':
    case 'X':
        rot.setEulerZYX(0, 0, value);
        break;
    case 'y':
    case 'Y':
        rot.setEulerZYX(0, value, 0);
        break;
    case 'z':
    case 'Z':
        rot.setEulerZYX(value, 0, 0);
        break;
    default:
        qFatal("Unexpected coordinate value: %c", coordinate);
    }
    switch (m_mode) {
    case kLocal:
        dest = current * rot;
        break;
    default:
        break;
    }
    m_selected->setRotation(dest);
    updateModel();
    emit boneRotationDidChange(dest);
}

vpvl::Bone *BoneMotionModel::selectBone(int rowIndex)
{
    vpvl::Bone *bone = m_selected = m_bones[rowIndex];
    return bone;
}

QList<vpvl::Bone *> BoneMotionModel::bonesFromIndices(const QModelIndexList &indices) const
{
    QList<vpvl::Bone *> bones;
    foreach (QModelIndex index, indices)
        bones.append(index.isValid() ? m_bones[index.row()] : 0);
    return bones;
}

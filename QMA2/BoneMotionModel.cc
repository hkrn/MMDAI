#include "BoneMotionModel.h"
#include "util.h"
#include <vpvl/vpvl.h>

BoneMotionModel::BoneMotionModel(QObject *parent) :
    MotionBaseModel(parent),
    m_mode(kLocal)
{
}

void BoneMotionModel::saveMotion(vpvl::VMDMotion *motion)
{
    vpvl::BoneAnimation *animation = motion->mutableBoneAnimation();
    foreach (QVariant value, m_values) {
        vpvl::BoneKeyFrame *newFrame = new vpvl::BoneKeyFrame();
        QByteArray bytes = value.toByteArray();
        newFrame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
        animation->addFrame(newFrame);
    }
    setModified(false);
}

bool BoneMotionModel::loadPose(vpvl::VPDPose *pose, vpvl::PMDModel *model, int frameIndex)
{
    if (model == m_model && m_motion) {
        const vpvl::VPDPose::BoneList boneFrames = pose->bones();
        vpvl::BoneAnimation *animation = m_motion->mutableBoneAnimation();
        uint32_t nBoneFrames = boneFrames.size();
        for (uint32_t i = 0; i < nBoneFrames; i++) {
            vpvl::VPDPose::Bone *frame = boneFrames[i];
            const uint8_t *name = frame->name;
            QString key = internal::toQString(name);
            int i = m_keys.indexOf(key);
            if (i != -1) {
                QModelIndex modelIndex = index(i, frameIndex);
                btQuaternion rotation;
                const btVector4 &v = frame->rotation;
                rotation.setValue(v.x(), v.y(), v.z(), v.w());
                vpvl::BoneKeyFrame *newFrame = new vpvl::BoneKeyFrame();
                newFrame->setDefaultInterpolationParameter();
                newFrame->setName(name);
                newFrame->setPosition(frame->position);
                newFrame->setRotation(rotation);
                newFrame->setFrameIndex(frameIndex);
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes, Qt::EditRole);
                animation->addFrame(newFrame);
            }
        }
        animation->refresh();
        reset();
        qDebug("Loaded a pose to the model: %s", qPrintable(internal::toQString(model)));
        return true;
    }
    else {
        qWarning("Tried loading pose to invalid model or without motion: %s", qPrintable(internal::toQString(model)));
        return false;
    }
}

bool BoneMotionModel::registerKeyFrame(vpvl::Bone *bone, int frameIndex)
{
    QString key;
    if (bone) {
        key = internal::toQString(bone->name());
    }
    else if (vpvl::Bone *selected = selectedBone()) {
        key = internal::toQString(selected->name());
    }
    else {
        qWarning("A bone is not selected or null");
        return false;
    }
    int i = m_keys.indexOf(key);
    if (i != -1 && m_motion) {
        QModelIndex modelIndex = index(i, frameIndex);
        QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
        vpvl::BoneAnimation *animation = m_motion->mutableBoneAnimation();
        vpvl::BoneKeyFrame *newFrame = new vpvl::BoneKeyFrame();
        /* FIXME: interpolation */
        newFrame->setDefaultInterpolationParameter();
        newFrame->setName(bone->name());
        newFrame->setPosition(bone->position());
        newFrame->setRotation(bone->rotation());
        newFrame->setFrameIndex(frameIndex);
        newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
        animation->addFrame(newFrame);
        animation->refresh();
        setData(modelIndex, bytes, Qt::EditRole);
        return true;
    }
    else {
        qWarning("Tried registering not bone key frame: %s", qPrintable(key));
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
    emit modelDidChange(model);
    qDebug("Set a model in BoneMotionModel: %s", qPrintable(internal::toQString(model)));
}

bool BoneMotionModel::loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (model == m_model) {
        const vpvl::BoneAnimation &animation = motion->boneAnimation();
        uint32_t nBoneFrames = animation.countFrames();
        for (uint32_t i = 0; i < nBoneFrames; i++) {
            vpvl::BoneKeyFrame *frame = animation.frameAt(i);
            const uint8_t *name = frame->name();
            QString key = internal::toQString(name);
            int i = m_keys.indexOf(key);
            if (i != -1) {
                uint32_t frameIndex = frame->frameIndex();
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                QModelIndex modelIndex = index(i, frameIndex);
                vpvl::BoneKeyFrame newFrame;
                newFrame.setName(name);
                newFrame.setPosition(frame->position());
                newFrame.setRotation(frame->rotation());
                newFrame.setFrameIndex(frameIndex);
                int8_t x1, x2, y1, y2;
                for (int i = 0; i < vpvl::BoneKeyFrame::kMax; i++) {
                    vpvl::BoneKeyFrame::InterpolationType type = static_cast<vpvl::BoneKeyFrame::InterpolationType>(i);
                    frame->getInterpolationParameter(type, x1, x2, y1, y2);
                    newFrame.setInterpolationParameter(type, x1, x2, y1, y2);
                }
                newFrame.write(reinterpret_cast<uint8_t *>(bytes.data()));
                setData(modelIndex, bytes, Qt::EditRole);
            }
        }
        m_motion = motion;
        reset();
        qDebug("Loaded a motion to the model in BoneMotionModel: %s", qPrintable(internal::toQString(model)));
        return true;
    }
    else {
        qDebug("Tried loading a motion to different model, ignored: %s", qPrintable(internal::toQString(model)));
        return false;
    }
}

void BoneMotionModel::clearMotion()
{
    m_bones.clear();
    m_selected.clear();
    m_values.clear();
    setModified(false);
    reset();
    resetAllBones();
}

void BoneMotionModel::clearModel()
{
    clearMotion();
    m_keys.clear();
    m_model = 0;
    reset();
}

bool BoneMotionModel::resetBone(ResetType type)
{
    foreach (vpvl::Bone *selected, m_selected) {
        btVector3 pos = selected->position();
        btQuaternion rot = selected->rotation();
        switch (type) {
        case kX:
            pos.setX(0.0f);
            selected->setPosition(pos);
            break;
        case kY:
            pos.setY(0.0f);
            selected->setPosition(pos);
            break;
        case kZ:
            pos.setZ(0.0f);
            selected->setPosition(pos);
            break;
        case kRotation:
            rot.setValue(0.0f, 0.0f, 0.0f, 1.0f);
            selected->setRotation(rot);
            break;
        default:
            qFatal("Unexpected reset bone type: %d", type);
        }
    }
    return updateModel();
}

bool BoneMotionModel::resetAllBones()
{
    if (m_model) {
        m_model->resetAllBones();
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
    foreach (vpvl::Bone *selected, m_selected) {
        btVector3 pos = selected->position();
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
        selected->setPosition(pos);
        updateModel();
        emit bonePositionDidChange(selected, pos);
    }
}

void BoneMotionModel::setRotation(int coordinate, float value)
{
    if (!isBoneSelected())
        return;
    vpvl::Bone *selected = m_selected.last();
    btQuaternion rot = selected->rotation();
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
    selected->setRotation(rot);
    updateModel();
    emit boneRotationDidChange(selected, rot);
}

void BoneMotionModel::transform(int coordinate, float value)
{
    foreach (vpvl::Bone *selected, m_selected) {
        btVector3 current = selected->position(), pos, dest;
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
            dest = btTransform(selected->rotation(), current) * pos;
            break;
        case kGlobal:
            dest = current + pos;
            break;
        default:
            break;
        }
        selected->setPosition(dest);
        updateModel();
        emit bonePositionDidChange(selected, dest);
    }
}

void BoneMotionModel::rotate(int coordinate, float value)
{
    vpvl::Bone *selected = m_selected.last();
    if (!isBoneSelected())
        return;
    btQuaternion current = selected->rotation(), rot, dest;
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
    selected->setRotation(dest);
    updateModel();
    emit boneRotationDidChange(selected, dest);
}

void BoneMotionModel::selectBones(QList<vpvl::Bone *> bones)
{
    m_selected = bones;
}

vpvl::Bone *BoneMotionModel::selectBone(int rowIndex)
{
    m_selected.clear();
    vpvl::Bone *bone = m_bones[rowIndex];
    m_selected.append(bone);
    return bone;
}

vpvl::Bone *BoneMotionModel::findBone(const QString &name)
{
    QByteArray bytes = internal::getTextCodec()->fromUnicode(name);
    foreach (vpvl::Bone *bone, m_bones) {
        if (!qstrcmp(reinterpret_cast<const char *>(bone->name()), bytes))
            return bone;
    }
    return 0;
}

QList<vpvl::Bone *> BoneMotionModel::bonesFromIndices(const QModelIndexList &indices) const
{
    QList<vpvl::Bone *> bones;
    foreach (QModelIndex index, indices)
        bones.append(index.isValid() ? m_bones[index.row()] : 0);
    return bones;
}

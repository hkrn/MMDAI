#include "BoneMotionModel.h"
#include "SceneWidget.h"
#include "VPDFile.h"
#include "util.h"
#include <vpvl/vpvl.h>

namespace
{

class TreeItem : public MotionBaseModel::ITreeItem
{
public:
    TreeItem(const QString &name, vpvl::Bone *bone, bool isRoot, TreeItem *parent)
        : m_name(name),
          m_parent(parent),
          m_bone(bone),
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
    vpvl::Bone *bone() const {
        return m_bone;
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
    vpvl::Bone *m_bone;
    bool m_isRoot;
};

class CopyFramesCommand : public QUndoCommand
{
public:
    CopyFramesCommand(BoneMotionModel *model)
        : QUndoCommand(),
          m_model(model)
    {
    }
    virtual ~CopyFramesCommand() {}

    void undo() {}
    void redo() {}

private:
    BoneMotionModel *m_model;
};

class LoadPoseCommand : public QUndoCommand
{
public:
    LoadPoseCommand(BoneMotionModel *bmm, VPDFile *pose, int frameIndex)
        : QUndoCommand(),
          m_bmm(bmm),
          m_pose(0),
          m_frameIndex(frameIndex)
    {
        int nBones = m_bmm->rowCount();
        for (int i = 0; i < nBones; i++) {
            const QModelIndex &index = bmm->index(i, frameIndex);
            if (index.data(BoneMotionModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
                m_indices.append(index);
        }
        m_pose = pose->clone();
        execute();
    }
    virtual ~LoadPoseCommand() {
        delete m_pose;
    }

    virtual void undo() {
        vpvl::BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        animation->deleteKeyFrames(m_frameIndex);
        int nBones = m_bmm->rowCount();
        for (int i = 0; i < nBones; i++) {
            const QModelIndex &index = m_bmm->index(i, m_frameIndex);
            m_bmm->setData(index, BoneMotionModel::kInvalidData, Qt::EditRole);
        }
        foreach (const QModelIndex &index, m_indices) {
            const QByteArray &bytes = index.data(BoneMotionModel::kBinaryDataRole).toByteArray();
            m_bmm->setData(index, bytes, Qt::EditRole);
            vpvl::BoneKeyFrame *frame = new vpvl::BoneKeyFrame();
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->addKeyFrame(frame);
        }
        animation->refresh();
        m_bmm->refreshModel();
    }
    virtual void redo() {
        execute();
    }

private:
    void execute() {
        QTextCodec *codec = internal::getTextCodec();
        vpvl::BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        const QMap<QString, MotionBaseModel::ITreeItem *> &bones = m_bmm->keys();
        vpvl::Quaternion rotation;
        foreach (VPDFile::Bone *bone, m_pose->bones()) {
            const QString &key = bone->name;
            if (bones.contains(key)) {
                const vpvl::Vector4 &v = bone->rotation;
                const QModelIndex &modelIndex = m_bmm->frameToIndex(bones[key], m_frameIndex);
                rotation.setValue(v.x(), v.y(), v.z(), v.w());
                vpvl::BoneKeyFrame *newFrame = new vpvl::BoneKeyFrame();
                newFrame->setDefaultInterpolationParameter();
                newFrame->setName(reinterpret_cast<const uint8_t *>(codec->fromUnicode(key).constData()));
                newFrame->setPosition(bone->position);
                newFrame->setRotation(rotation);
                newFrame->setFrameIndex(m_frameIndex);
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                m_bmm->setData(modelIndex, bytes, Qt::EditRole);
                animation->addKeyFrame(newFrame);
            }
        }
        animation->refresh();
        m_bmm->refreshModel();
    }

    QModelIndexList m_indices;
    BoneMotionModel *m_bmm;
    VPDFile *m_pose;
    int m_frameIndex;
};

class SetFramesCommand : public QUndoCommand
{
public:
    SetFramesCommand(BoneMotionModel *bmm, const QList<BoneMotionModel::Frame> &frames)
        : QUndoCommand(),
          m_bmm(bmm)
    {
        int nBones = m_bmm->rowCount();
        QHash<int, bool> indexProceeded;
        foreach (const BoneMotionModel::Frame &frame, frames) {
            int frameIndex = frame.first;
            if (!indexProceeded[frameIndex]) {
                for (int i = 0; i < nBones; i++) {
                    const QModelIndex &index = bmm->index(i, frameIndex);
                    if (index.data(BoneMotionModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
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
        vpvl::BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        int nBones = m_bmm->rowCount();
        foreach (int frameIndex, m_frameIndices) {
            animation->deleteKeyFrames(frameIndex);
            for (int i = 0; i < nBones; i++) {
                const QModelIndex &index = m_bmm->index(i, frameIndex);
                m_bmm->setData(index, BoneMotionModel::kInvalidData, Qt::EditRole);
            }
        }
        foreach (const QModelIndex &index, m_indices) {
            const QByteArray &bytes = index.data(BoneMotionModel::kBinaryDataRole).toByteArray();
            m_bmm->setData(index, bytes, Qt::EditRole);
            vpvl::BoneKeyFrame *frame = new vpvl::BoneKeyFrame();
            frame->read(reinterpret_cast<const uint8_t *>(bytes.constData()));
            animation->addKeyFrame(frame);
        }
    }
    virtual void redo() {
        execute();
    }

private:
    void execute() {
        QString key;
        vpvl::BoneAnimation *animation = m_bmm->currentMotion()->mutableBoneAnimation();
        const BoneMotionModel::Keys &keys = m_bmm->keys();
        vpvl::Bone *selected = m_bmm->selectedBone();
        foreach (const BoneMotionModel::Frame &pair, m_frames) {
            int frameIndex = pair.first;
            vpvl::Bone *bone = pair.second;
            if (bone) {
                key = internal::toQString(bone->name());
            }
            else if (selected) {
                key = internal::toQString(selected->name());
            }
            else {
                qWarning("No bone is selected or null");
                continue;
            }
            if (keys.contains(key)) {
                const QModelIndex &modelIndex = m_bmm->frameToIndex(keys[key], frameIndex);
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                vpvl::BoneKeyFrame *newFrame = new vpvl::BoneKeyFrame();
                /* FIXME: interpolation */
                newFrame->setDefaultInterpolationParameter();
                newFrame->setName(bone->name());
                newFrame->setPosition(bone->position());
                newFrame->setRotation(bone->rotation());
                newFrame->setFrameIndex(frameIndex);
                newFrame->write(reinterpret_cast<uint8_t *>(bytes.data()));
                animation->addKeyFrame(newFrame);
                animation->refresh();
                m_bmm->setData(modelIndex, bytes, Qt::EditRole);
            }
            else {
                qWarning("Tried registering not bone key frame: %s", qPrintable(key));
                continue;
            }
        }
        m_bmm->refreshModel();
    }

    QList<int> m_frameIndices;
    QModelIndexList m_indices;
    QList<BoneMotionModel::Frame> m_frames;
    BoneMotionModel *m_bmm;
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
        m_model->resetAllBones();
        m_model->updateImmediate();
    }

    vpvl::PMDModel *m_model;
    vpvl::PMDModel::State *m_state;
};

class SetBoneCommand : public QUndoCommand
{
public:
    SetBoneCommand(vpvl::PMDModel *model, vpvl::PMDModel::State *state)
        : QUndoCommand(),
          m_model(model),
          m_newState(0),
          m_oldState(state)
    {
        m_newState = m_model->saveState();
    }
    virtual ~SetBoneCommand() {
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

BoneMotionModel::BoneMotionModel(QUndoGroup *undo, const SceneWidget *scene, QObject *parent) :
    MotionBaseModel(undo, parent),
    m_sceneWidget(scene),
    m_state(0),
    m_mode(kLocal)
{
    m_root = new TreeItem("", 0, true, 0);
}

BoneMotionModel::~BoneMotionModel()
{
}

void BoneMotionModel::saveMotion(vpvl::VMDMotion *motion)
{
    if (m_model) {
        vpvl::BoneAnimation *animation = motion->mutableBoneAnimation();
        foreach (const QVariant &value, values()) {
            vpvl::BoneKeyFrame *newFrame = new vpvl::BoneKeyFrame();
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

void BoneMotionModel::copyFrames(int /* frameIndex */)
{
    if (m_model && m_motion) {
    }
}

void BoneMotionModel::startTransform()
{
    if (m_model) {
        m_model->discardState(m_state);
        m_state = m_model->saveState();
    }
}

void BoneMotionModel::commitTransform()
{
    if (m_model && m_state) {
        addUndoCommand(new SetBoneCommand(m_model, m_state));
        m_state = 0;
    }
}

void BoneMotionModel::loadPose(VPDFile *pose, vpvl::PMDModel *model, int frameIndex)
{
    if (model == m_model && m_motion) {
        addUndoCommand(new LoadPoseCommand(this, pose, frameIndex));
        qDebug("Loaded a pose to the model: %s", qPrintable(internal::toQString(model)));
    }
    else {
        qWarning("Tried loading pose to invalid model or without motion: %s", qPrintable(internal::toQString(model)));
    }
}

void BoneMotionModel::savePose(VPDFile *pose, vpvl::PMDModel *model, int frameIndex)
{
    if (model == m_model) {
        VPDFile::BoneList bones;
        foreach (ITreeItem *item, keys()) {
            const QModelIndex &modelIndex = frameToIndex(item, frameIndex);
            QVariant variant = modelIndex.data(BoneMotionModel::kBinaryDataRole);
            if (variant.canConvert(QVariant::ByteArray)) {
                VPDFile::Bone *bone = new VPDFile::Bone();
                vpvl::BoneKeyFrame frame;
                frame.read(reinterpret_cast<const uint8_t *>(variant.toByteArray().constData()));
                const vpvl::Quaternion &q = frame.rotation();
                bone->name = internal::toQString(&frame);
                bone->position = frame.position();
                bone->rotation = vpvl::Vector4(q.x(), q.y(), q.z(), q.w());
                bones.append(bone);
            }
        }
        pose->setBones(bones);
    }
    else {
        qWarning("Tried loading pose to invalid model or without motion: %s", qPrintable(internal::toQString(model)));
    }
}

void BoneMotionModel::setFrames(const QList<Frame> &frames)
{
    if (m_model && m_motion) {
        addUndoCommand(new SetFramesCommand(this, frames));
    }
    else {
        qWarning("No model or motion to register a bone frame.");
        return;
    }
}

void BoneMotionModel::setPMDModel(vpvl::PMDModel *model)
{
    clearKeys();
    if (model) {
        const uint32_t namesCount = model->boneCategoryNames().count();
        vpvl::Array<vpvl::BoneList *> allBones;
        vpvl::Array<uint8_t *> names;
        allBones.copy(model->bonesForUI());
        names.copy(model->boneCategoryNames());
        Keys &keys = m_keys[model];
        for (uint32_t i = 0; i < namesCount; i++) {
            const QString category = internal::toQString(names[i]).trimmed();
            const vpvl::BoneList *bones = allBones[i];
            const uint32_t bonesCount = bones->count();
            TreeItem *parent = new TreeItem(category, 0, false, static_cast<TreeItem *>(m_root));
            for (uint32_t j = 0; j < bonesCount; j++) {
                vpvl::Bone *bone = bones->at(j);
                const QString name = internal::toQString(bone);
                TreeItem *child = new TreeItem(name, bone, false, parent);
                parent->addChild(child);
                keys.insert(name, child);
            }
            m_root->addChild(parent);
        }
    }
    MotionBaseModel::setPMDModel(model);
    qDebug("Set a model in BoneMotionModel: %s", qPrintable(internal::toQString(model)));
    reset();
}

void BoneMotionModel::loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model)
{
    if (model == m_model) {
        const vpvl::BoneAnimation &animation = motion->boneAnimation();
        const uint32_t nBoneFrames = animation.countKeyFrames();
        const Keys &keys = this->keys();
        for (uint32_t i = 0; i < nBoneFrames; i++) {
            const vpvl::BoneKeyFrame *frame = animation.frameAt(i);
            const uint8_t *name = frame->name();
            const QString key = internal::toQString(name);
            if (keys.contains(key)) {
                uint32_t frameIndex = frame->frameIndex();
                QByteArray bytes(vpvl::BoneKeyFrame::strideSize(), '0');
                ITreeItem *item = keys[key];
                const QModelIndex &modelIndex = frameToIndex(item, frameIndex);
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
                setData(modelIndex, bytes);
            }
        }
        m_motion = motion;
        refreshModel();
        qDebug("Loaded a motion to the model in BoneMotionModel: %s", qPrintable(internal::toQString(model)));
    }
    else {
        qDebug("Tried loading a motion to different model, ignored: %s", qPrintable(internal::toQString(model)));
    }
}

void BoneMotionModel::deleteMotion()
{
    m_selected.clear();
    m_values[m_model].clear();
    setModified(false);
    reset();
    resetAllBones();
}

void BoneMotionModel::deleteModel()
{
    deleteMotion();
    clearKeys();
    delete m_root;
    m_root = new TreeItem("", 0, true, 0);
    setPMDModel(0);
    reset();
}

void BoneMotionModel::deleteFrame(const QModelIndex &index)
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
        vpvl::Bone *bone = item->bone();
        m_motion->mutableBoneAnimation()->deleteKeyFrame(index.column(), bone->name());
        setData(index, kInvalidData, Qt::EditRole);
    }
}

void BoneMotionModel::resetBone(ResetType type)
{
    foreach (vpvl::Bone *selected, m_selected) {
        vpvl::Vector3 pos = selected->position();
        vpvl::Quaternion rot = selected->rotation();
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
    updateModel();
}

void BoneMotionModel::resetAllBones()
{
    if (m_model)
        addUndoCommand(new ResetAllCommand(m_model));
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
        vpvl::Vector3 pos = selected->position();
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
    vpvl::Quaternion rot = selected->rotation();
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

void BoneMotionModel::translate(int coordinate, float value)
{
    vpvl::Vector3 pos, dest;
    foreach (vpvl::Bone *selected, m_selected) {
        const vpvl::Vector3 &current = selected->position();
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
        case kView: {
            QVector4D r = modelviewMatrix() * QVector4D(pos.x(), pos.y(), pos.z(), 0.0f);
            dest = vpvl::Transform(selected->rotation(), current) * vpvl::Vector3(r.x(), r.y(), r.z());
            break;
        }
        case kLocal: {
            dest = vpvl::Transform(selected->rotation(), current) * pos;
            break;
        }
        case kGlobal: {
            dest = current + pos;
            break;
        }
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
    if (!isBoneSelected())
        return;
    vpvl::Bone *selected = m_selected.last();
    const vpvl::Quaternion &current = selected->rotation();
    vpvl::Quaternion rot, dest;
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
    case kView: {
        QVector4D r = modelviewMatrix() * QVector4D(rot.x(), rot.y(), rot.z(), rot.w());
        r.normalize();
        dest = current * vpvl::Quaternion(r.x(), r.y(), r.z(), r.w());
        break;
    }
    case kLocal: {
        dest = current * rot;
        break;
    }
    default:
        break;
    }
    selected->setRotation(dest);
    updateModel();
    emit boneRotationDidChange(selected, dest);
}

void BoneMotionModel::selectBones(const QList<vpvl::Bone *> &bones)
{
    m_selected = bones;
}

vpvl::Bone *BoneMotionModel::findBone(const QString &name)
{
    QByteArray bytes = internal::getTextCodec()->fromUnicode(name);
    foreach (ITreeItem *item, keys()) {
        vpvl::Bone *bone = static_cast<TreeItem *>(item)->bone();
        if (!qstrcmp(reinterpret_cast<const char *>(bone->name()), bytes))
            return bone;
    }
    return 0;
}

const QMatrix4x4 BoneMotionModel::modelviewMatrix() const
{
    float modelviewf[16];
    qreal modelviewd[16];
    m_sceneWidget->scene()->getModelViewMatrix(modelviewf);
    for (int i = 0; i < 16; i++)
        modelviewd[i] = modelviewf[i];
    return QMatrix4x4(modelviewd);
}

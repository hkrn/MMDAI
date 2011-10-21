#ifndef BONEMOTIONMODEL_H
#define BONEMOTIONMODEL_H

#include "MotionBaseModel.h"
#include "vpvl/BaseAnimation.h"

namespace vpvl {
class BoneKeyFrame;
}

class SceneWidget;
class VPDFile;

class BoneMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    enum TransformType {
        kLocal,
        kGlobal,
        kView
    };
    enum ResetType {
        kX,
        kY,
        kZ,
        kRotation
    };
    typedef QPair<int, vpvl::BoneKeyFrame *> Frame;

    BoneMotionModel(QUndoGroup *undo, const SceneWidget *scene, QObject *parent = 0);
    ~BoneMotionModel();

    void saveMotion(vpvl::VMDMotion *motion);
    void copyFrames(int frameIndex);
    void pasteFrame(int frameIndex);
    void startTransform();
    void commitTransform();
    void loadPose(VPDFile *pose, vpvl::PMDModel *model, int frameIndex);
    void savePose(VPDFile *pose, vpvl::PMDModel *model, int frameIndex);
    void setFrames(const QList<Frame> &frames);
    void resetBone(ResetType type);
    void resetAllBones();
    void setMode(int value);
    void setPosition(int coordinate, float value);
    void setRotation(int coordinate, float value);
    vpvl::Bone *findBone(const QString &name);
    vpvl::Bone *selectedBone() const { return m_selected.isEmpty() ? 0 : m_selected.first(); }
    bool isBoneSelected() const { return m_model != 0 && selectedBone() != 0; }

public slots:
    void setPMDModel(vpvl::PMDModel *model);
    void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void deleteMotion();
    void deleteModel();
    void deleteFrame(const QModelIndex &index);
    void translate(int coordinate, float value);
    void rotate(int coordinate, float value);
    void selectBones(const QList<vpvl::Bone *> &bones);

signals:
    void bonePositionDidChange(vpvl::Bone *bone, const vpvl::Vector3 &pos);
    void boneRotationDidChange(vpvl::Bone *bone, const vpvl::Quaternion &rot);
    void bonesDidSelect(const QList<vpvl::Bone *> &bones);

private:
    const QMatrix4x4 modelviewMatrix() const;

    QList<vpvl::Bone *> m_selected;
    vpvl::BaseKeyFrameList m_frames;
    const SceneWidget *m_sceneWidget;
    vpvl::PMDModel::State *m_state;
    TransformType m_mode;
};

#endif // BONEMOTIONMODEL_H

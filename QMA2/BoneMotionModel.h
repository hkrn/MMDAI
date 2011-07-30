#ifndef BONEMOTIONMODEL_H
#define BONEMOTIONMODEL_H

#include "MotionBaseModel.h"

class BoneMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    enum ResetType {
        kX,
        kY,
        kZ,
        kRotation
    };

    explicit BoneMotionModel(QObject *parent = 0);

    void loadPose(vpvl::VPDPose *pose, vpvl::PMDModel *model, int frameIndex);
    void registerKeyFrame(vpvl::Bone *bone, int frameIndex);
    void resetBone(ResetType type);
    vpvl::Bone *selectBone(int rowIndex);
    QList<vpvl::Bone *> bonesFromIndices(const QModelIndexList &indices);

public slots:
    void setPMDModel(vpvl::PMDModel *model);
    void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);

private:
    QList<vpvl::Bone *> m_bones;
    vpvl::Bone *m_selected;
};

#endif // BONEMOTIONMODEL_H

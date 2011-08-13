#ifndef BONEMOTIONMODEL_H
#define BONEMOTIONMODEL_H

#include "MotionBaseModel.h"
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

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

    explicit BoneMotionModel(QObject *parent = 0);

    void saveMotion(vpvl::VMDMotion *motion);
    bool loadPose(vpvl::VPDPose *pose, vpvl::PMDModel *model, int frameIndex);
    bool registerKeyFrame(vpvl::Bone *bone, int frameIndex);
    bool resetBone(ResetType type);
    bool resetAllBones();
    void setMode(int value);
    void setPosition(int coordinate, float value);
    void setRotation(int coordinate, float value);
    void transform(int coordinate, float value);
    void rotate(int coordinate, float value);
    void selectBones(QList<vpvl::Bone *> bones);
    vpvl::Bone *selectBone(int rowIndex);
    vpvl::Bone *findBone(const QString &name);
    QList<vpvl::Bone *> bonesFromIndices(const QModelIndexList &indices) const;

    vpvl::Bone *selectedBone() const { return m_selected.first(); }
    bool isBoneSelected() const { return m_model != 0 && selectedBone() != 0; }

public slots:
    void setPMDModel(vpvl::PMDModel *model);
    bool loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void clearMotion();
    void clearModel();

signals:
    void bonePositionDidChange(vpvl::Bone *bone, const btVector3 &pos);
    void boneRotationDidChange(vpvl::Bone *bone, const btQuaternion &rot);

private:
    QList<vpvl::Bone *> m_bones;
    QList<vpvl::Bone *> m_selected;
    TransformType m_mode;
};

#endif // BONEMOTIONMODEL_H

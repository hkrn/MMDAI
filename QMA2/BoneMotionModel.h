#ifndef BONEMOTIONMODEL_H
#define BONEMOTIONMODEL_H

#include "MotionBaseModel.h"
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

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
    typedef QPair<int, vpvl::Bone *> Frame;

    BoneMotionModel(QUndoGroup *undo, QObject *parent = 0);
    ~BoneMotionModel();

    void saveMotion(vpvl::VMDMotion *motion);
    void copyFrames(int frameIndex);
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
    void transform(int coordinate, float value);
    void rotate(int coordinate, float value);
    void selectBones(QList<vpvl::Bone *> bones);
    vpvl::Bone *selectBone(int rowIndex);
    vpvl::Bone *findBone(const QString &name);
    QList<vpvl::Bone *> bonesByIndices(const QModelIndexList &indices) const;
    QList<vpvl::Bone *> bonesFromIndices(const QModelIndexList &indices) const;

    vpvl::Bone *selectedBone() const { return m_selected.isEmpty() ? 0 : m_selected.first(); }
    bool isBoneSelected() const { return m_model != 0 && selectedBone() != 0; }

public slots:
    void setPMDModel(vpvl::PMDModel *model);
    void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void deleteMotion();
    void deleteModel();
    void deleteFrame(const QModelIndex &index);

signals:
    void bonePositionDidChange(vpvl::Bone *bone, const btVector3 &pos);
    void boneRotationDidChange(vpvl::Bone *bone, const btQuaternion &rot);

private:
    QList<vpvl::Bone *> m_bones;
    QList<vpvl::Bone *> m_selected;
    TransformType m_mode;
};

#endif // BONEMOTIONMODEL_H

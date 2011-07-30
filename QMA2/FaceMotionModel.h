#ifndef FACEMOTIONMODEL_H
#define FACEMOTIONMODEL_H

#include "MotionBaseModel.h"

class FaceMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    explicit FaceMotionModel(QObject *parent = 0);

    void registerKeyFrame(vpvl::Face *bone, int frameIndex);
    vpvl::Face *selectFace(int rowIndex);
    QList<vpvl::Face *> facesFromIndices(const QModelIndexList &indices);
    void setWeight(float value);

public slots:
    void setPMDModel(vpvl::PMDModel *model);
    void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);

private:
    QList<vpvl::Face *> m_faces;
    vpvl::Face *m_selected;
};

#endif // FACEMOTIONMODEL_H

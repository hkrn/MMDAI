#ifndef FACEMOTIONMODEL_H
#define FACEMOTIONMODEL_H

#include "MotionBaseModel.h"

class FaceMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    explicit FaceMotionModel(QObject *parent = 0);

    void saveMotion(vpvl::VMDMotion *motion);
    void registerKeyFrame(vpvl::Face *bone, int frameIndex);
    bool resetAllFaces();
    void selectFaces(QList<vpvl::Face *> faces);
    vpvl::Face *selectFace(int rowIndex);
    vpvl::Face *findFace(const QString &name);
    QList<vpvl::Face *> facesFromIndices(const QModelIndexList &indices);
    void setWeight(float value);
    void setWeight(float value, vpvl::Face *face);

public slots:
    void setPMDModel(vpvl::PMDModel *model);
    bool loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void clearMotion();
    void clearModel();

private:
    QList<vpvl::Face *> m_faces;
    QList<vpvl::Face *> m_selected;
};

#endif // FACEMOTIONMODEL_H

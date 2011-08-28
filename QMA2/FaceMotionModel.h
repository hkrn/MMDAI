#ifndef FACEMOTIONMODEL_H
#define FACEMOTIONMODEL_H

#include "MotionBaseModel.h"

class FaceMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    typedef QPair<int, vpvl::Face *> Frame;

    FaceMotionModel(QUndoGroup *undo, QObject *parent = 0);
    ~FaceMotionModel();

    void saveMotion(vpvl::VMDMotion *motion);
    void copyFrames(int frameIndex);
    void startTransform();
    void commitTransform();
    void setFrames(const QList<Frame> &frames);
    void resetAllFaces();
    void selectFaces(QList<vpvl::Face *> faces);
    vpvl::Face *selectFace(int rowIndex);
    vpvl::Face *findFace(const QString &name);
    QList<vpvl::Face *> facesByIndices(const QModelIndexList &indices);
    QList<vpvl::Face *> facesFromIndices(const QModelIndexList &indices);
    void setWeight(float value);
    void setWeight(float value, vpvl::Face *face);

public slots:
    void setPMDModel(vpvl::PMDModel *model);
    void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void deleteMotion();
    void deleteModel();
    void deleteFrame(const QModelIndex &index);

protected:
    void clearKeys();

private:
    QList<vpvl::Face *> m_faces;
    QList<vpvl::Face *> m_selected;
    vpvl::PMDModel::State *m_state;
};

#endif // FACEMOTIONMODEL_H

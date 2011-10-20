#ifndef FACEMOTIONMODEL_H
#define FACEMOTIONMODEL_H

#include "MotionBaseModel.h"
#include "vpvl/BaseAnimation.h"

namespace vpvl {
class FaceKeyFrame;
}

class FaceMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    typedef QPair<int, vpvl::FaceKeyFrame *> Frame;

    FaceMotionModel(QUndoGroup *undo, QObject *parent = 0);
    ~FaceMotionModel();

    void saveMotion(vpvl::VMDMotion *motion);
    void copyFrames(int frameIndex);
    void pasteFrame(int frameIndex);
    void startTransform();
    void commitTransform();
    void setFrames(const QList<Frame> &frames);
    void resetAllFaces();
    void selectFaces(QList<vpvl::Face *> faces);
    vpvl::Face *findFace(const QString &name);
    void setWeight(float value);
    void setWeight(float value, vpvl::Face *face);
    vpvl::Face *selectedFace() const { return m_selected.isEmpty() ? 0 : m_selected.first(); }
    bool isFaceSelected() const { return m_model != 0 && selectedFace() != 0; }

public slots:
    void setPMDModel(vpvl::PMDModel *model);
    void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void deleteMotion();
    void deleteModel();
    void deleteFrame(const QModelIndex &index);

private:
    QList<vpvl::Face *> m_selected;
    vpvl::BaseKeyFrameList m_frames;
    vpvl::PMDModel::State *m_state;
};

#endif // FACEMOTIONMODEL_H

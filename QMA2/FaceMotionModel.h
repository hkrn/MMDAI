#ifndef FACEMOTIONMODEL_H
#define FACEMOTIONMODEL_H

#include "MotionBaseModel.h"

class FaceMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    typedef QStringList Keys;
    typedef QHash< QPair<int, int>, QVariant > Values;

    typedef QPair<int, vpvl::Face *> Frame;

    FaceMotionModel(QUndoGroup *undo, QObject *parent = 0);
    ~FaceMotionModel();

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

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

    const Keys keys() const { return keys(m_model); }
    const Keys keys(vpvl::PMDModel *model) const { return m_keys[model]; }
    const Values values() const { return values(m_model); }
    const Values values(vpvl::PMDModel *model) const { return m_values[model]; }

public slots:
    void setPMDModel(vpvl::PMDModel *model);
    void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void deleteMotion();
    void deleteModel();
    void deleteFrame(const QModelIndex &index);

protected:
    void clearKeys();
    void clearValues();
    void appendKey(const QString &key, vpvl::PMDModel *model) { m_keys[model].append(key); }

private:
    QHash<vpvl::PMDModel *, Keys> m_keys;
    QHash<vpvl::PMDModel *, Values> m_values;
    QList<vpvl::Face *> m_faces;
    QList<vpvl::Face *> m_selected;
    vpvl::PMDModel::State *m_state;
};

#endif // FACEMOTIONMODEL_H

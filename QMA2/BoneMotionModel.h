#ifndef BONEMOTIONMODEL_H
#define BONEMOTIONMODEL_H

#include "MotionBaseModel.h"

class BoneTreeItem;
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
    typedef QPair<int, vpvl::Bone *> Frame;
    typedef QMap<QString, BoneTreeItem *> Keys;
    typedef QHash<QModelIndex, QVariant> Values;

    BoneMotionModel(QUndoGroup *undo, const SceneWidget *scene, QObject *parent = 0);
    ~BoneMotionModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

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
    void selectBones(const QList<vpvl::Bone *> &bones);
    vpvl::Bone *findBone(const QString &name);

    const QModelIndex frameToIndex(BoneTreeItem *item, int frameIndex) const;
    const Keys keys() const { return m_keys[m_model]; }
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

signals:
    void bonePositionDidChange(vpvl::Bone *bone, const vpvl::Vector3 &pos);
    void boneRotationDidChange(vpvl::Bone *bone, const vpvl::Quaternion &rot);

protected:
    void clearKeys();
    void clearValues();

private:
    const QMatrix4x4 modelviewMatrix() const;

    BoneTreeItem *m_root;
    QHash<vpvl::PMDModel *, Keys> m_keys;
    QHash<vpvl::PMDModel *, Values> m_values;
    QList<vpvl::Bone *> m_selected;
    const SceneWidget *m_sceneWidget;
    vpvl::PMDModel::State *m_state;
    TransformType m_mode;
};

#endif // BONEMOTIONMODEL_H

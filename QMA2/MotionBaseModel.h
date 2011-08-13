#ifndef MOTIONBASEMODEL_H
#define MOTIONBASEMODEL_H

#include <QtGui/QAbstractItemView>
#include <vpvl/PMDModel.h>

namespace vpvl {
class VMDMotion;
class VPDPose;
}

class MotionBaseModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit MotionBaseModel(QObject *parent = 0);
    ~MotionBaseModel();

    virtual void saveMotion(vpvl::VMDMotion *motion) = 0;
    void saveState();
    void restoreState();
    void discardState();

    vpvl::PMDModel *selectedModel() const { return m_model; }
    void setModified(bool value) { m_modified = value; motionDidModify(value); }
    bool isModified() const { return m_modified; }

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public slots:
    virtual void setPMDModel(vpvl::PMDModel *model) = 0;
    virtual bool loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model) = 0;
    virtual void clearMotion() = 0;
    virtual void clearModel() = 0;

signals:
    void modelDidChange(vpvl::PMDModel *model);
    void motionDidModify(bool value);

protected:
    bool updateModel();

    vpvl::PMDModel *m_model;
    vpvl::VMDMotion *m_motion;
    QList<QString> m_keys;
    QHash< QPair<int, int>, QVariant > m_values;

private:
    vpvl::PMDModel::State *m_state;
    bool m_modified;
};

#endif // MOTIONBASEMODEL_H

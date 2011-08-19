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
    typedef QStringList Keys;
    typedef QHash< QPair<int, int>, QVariant > Values;

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
    virtual void deleteMotion() = 0;
    virtual void deleteModel() = 0;
    virtual void deleteFrame(const QModelIndex &index) = 0;

signals:
    void modelDidChange(vpvl::PMDModel *model);
    void motionDidModify(bool value);

protected:
    bool updateModel();
    void setModel(vpvl::PMDModel *model);

    const Keys keys() const { return keys(m_model); }
    const Keys keys(vpvl::PMDModel *model) const { return m_keys[model]; }
    const Values values() const { return m_values[m_model]; }
    void appendKey(const QString &key, vpvl::PMDModel *model) { m_keys[model].append(key); }
    void clearKeys() { m_keys[m_model].clear(); m_keys.remove(m_model); }
    void clearValues() { m_values[m_model].clear(); m_values.remove(m_model); }

    vpvl::PMDModel *m_model;
    vpvl::VMDMotion *m_motion;

private:
    vpvl::PMDModel::State *m_state;
    QHash<vpvl::PMDModel *, Keys> m_keys;
    QHash<vpvl::PMDModel *, Values> m_values;
    bool m_modified;
};

#endif // MOTIONBASEMODEL_H

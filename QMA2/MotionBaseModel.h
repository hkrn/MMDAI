#ifndef MOTIONBASEMODEL_H
#define MOTIONBASEMODEL_H

#include <QtGui/QAbstractItemView>
#include <vpvl/PMDModel.h>

namespace vpvl {
class VMDMotion;
class VPDPose;
}

class QUndoCommand;
class QUndoGroup;
class QUndoStack;

class MotionBaseModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    typedef QStringList Keys;
    typedef QHash< QPair<int, int>, QVariant > Values;
    enum DataRole
    {
        kNameRole = 0x1000,
        kBinaryDataRole
    };

    static const QVariant kInvalidData;

    MotionBaseModel(QUndoGroup *undo, QObject *parent = 0);
    virtual ~MotionBaseModel();

    virtual void saveMotion(vpvl::VMDMotion *motion) = 0;
    virtual void copyFrames(int frameIndex) = 0;
    virtual void startTransform() = 0;
    virtual void commitTransform() = 0;
    void saveState();
    void restoreState();
    void discardState();
    void updateModel();
    void refreshModel();

    vpvl::PMDModel *selectedModel() const { return m_model; }
    vpvl::VMDMotion *currentMotion() const { return m_motion; }
    void setModified(bool value) { m_modified = value; motionDidModify(value); }
    bool isModified() const { return m_modified; }
    const Keys keys() const { return keys(m_model); }
    const Keys keys(vpvl::PMDModel *model) const { return m_keys[model]; }
    const Values values() const { return values(m_model); }
    const Values values(vpvl::PMDModel *model) const { return m_values[model]; }

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public slots:
    virtual void setPMDModel(vpvl::PMDModel *model) = 0;
    virtual void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model) = 0;
    virtual void deleteMotion() = 0;
    virtual void deleteModel() = 0;
    virtual void deleteFrame(const QModelIndex &index) = 0;

signals:
    void modelDidChange(vpvl::PMDModel *model);
    void motionDidModify(bool value);

protected:
    void appendKey(const QString &key, vpvl::PMDModel *model) { m_keys[model].append(key); }
    void clearKeys() { m_keys[m_model].clear(); m_keys.remove(m_model); }
    void clearValues() { m_values[m_model].clear(); m_values.remove(m_model); }
    void addUndoCommand(QUndoCommand *command);

    vpvl::PMDModel *m_model;
    vpvl::VMDMotion *m_motion;

private:
    vpvl::PMDModel::State *m_state;
    QHash<vpvl::PMDModel *, Keys> m_keys;
    QHash<vpvl::PMDModel *, Values> m_values;
    QHash<vpvl::PMDModel *, QUndoStack *> m_stacks;
    QUndoGroup *m_undo;
    bool m_modified;
};

#endif // MOTIONBASEMODEL_H

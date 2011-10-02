#ifndef MOTIONBASEMODEL_H
#define MOTIONBASEMODEL_H

#include <QtCore/QString>
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
    class ITreeItem
    {
    public:
        virtual void addChild(ITreeItem *item) = 0;
        virtual ITreeItem *parent() const = 0;
        virtual ITreeItem *child(int row) const = 0;
        virtual const QString &name() const = 0;
        virtual bool isRoot() const = 0;
        virtual int rowIndex() const = 0;
        virtual int countChildren() const = 0;
    };

    enum DataRole
    {
        kNameRole = 0x1000,
        kBinaryDataRole
    };

    typedef QMap<QString, ITreeItem *> Keys;
    typedef QHash<QModelIndex, QVariant> Values;

    static const QVariant kInvalidData;

    MotionBaseModel(QUndoGroup *undo, QObject *parent = 0);
    virtual ~MotionBaseModel();

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    const QModelIndex frameToIndex(ITreeItem *item, int frameIndex) const;

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
    const Keys keys() const { return m_keys[m_model]; }

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

public slots:
    void markAsNew(vpvl::PMDModel *model);
    virtual void setPMDModel(vpvl::PMDModel *model) = 0;
    virtual void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model) = 0;
    virtual void deleteMotion() = 0;
    virtual void deleteModel() = 0;
    virtual void deleteFrame(const QModelIndex &index) = 0;

signals:
    void modelDidChange(vpvl::PMDModel *model);
    void motionDidModify(bool value);

protected:
    void clearKeys();
    void clearValues();
    void addUndoCommand(QUndoCommand *command);
    const Values values() const { return m_values[m_model]; }

    ITreeItem *m_root;
    vpvl::PMDModel *m_model;
    vpvl::VMDMotion *m_motion;
    QHash<vpvl::PMDModel *, Keys> m_keys;
    QHash<vpvl::PMDModel *, Values> m_values;

private:
    vpvl::PMDModel::State *m_state;
    QHash<vpvl::PMDModel *, QUndoStack *> m_stacks;
    QUndoGroup *m_undo;
    bool m_modified;
};

#endif // MOTIONBASEMODEL_H

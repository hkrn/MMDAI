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

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

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
    virtual void clearKeys() = 0;
    virtual void clearValues() = 0;
    void addUndoCommand(QUndoCommand *command);
    vpvl::PMDModel *m_model;
    vpvl::VMDMotion *m_motion;

private:
    vpvl::PMDModel::State *m_state;
    QHash<vpvl::PMDModel *, QUndoStack *> m_stacks;
    QUndoGroup *m_undo;
    bool m_modified;
};

#endif // MOTIONBASEMODEL_H

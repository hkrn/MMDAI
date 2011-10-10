#include "MotionBaseModel.h"
#include "util.h"
#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

/**
 * Should create undo command
 *
 * - copyFrames (Bone and Face)
 * - loadPose (Bone)
 * - registerKeyFrame (Bone and Face)
 * - resetBone (Bone)
 * - resetAllBone (Bone)
 * - resetAllFace (Face)
 * - setPosition (Bone)
 * - setRotation (Bone)
 * - setWeight (Face)
 */

const QVariant MotionBaseModel::kInvalidData = QVariant();

MotionBaseModel::MotionBaseModel(QUndoGroup *undo, QObject *parent) :
    QAbstractTableModel(parent),
    m_root(0),
    m_model(0),
    m_motion(0),
    m_state(0),
    m_undo(undo),
    m_modified(false)
{
}

MotionBaseModel::~MotionBaseModel()
{
    if (m_model)
        m_model->discardState(m_state);
    if (m_state)
        qWarning("It seems memory leak occured: m_state");
    qDeleteAll(m_stacks);
    delete m_root;
    m_root = 0;
}

QVariant MotionBaseModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole && index.column() == 0) {
        ITreeItem *item = static_cast<ITreeItem *>(index.internalPointer());
        return item->name();
    }
    else if (role == kBinaryDataRole && m_model) {
        QVariant value = m_values[m_model].value(index);
        return value;
    }
    else {
        return QVariant();
    }
}

bool MotionBaseModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (m_model && index.isValid() && role == Qt::EditRole) {
        m_values[m_model].insert(index, value);
        setModified(true);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QVariant MotionBaseModel::headerData(int /* section */, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return kInvalidData;
    }
    return kInvalidData;
}

QModelIndex MotionBaseModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ITreeItem *parentItem = 0;
    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<ITreeItem *>(parent.internalPointer());

    ITreeItem *childItem = parentItem->child(row);
    return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex MotionBaseModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    ITreeItem *childItem = static_cast<ITreeItem *>(child.internalPointer());
    ITreeItem *parentItem = childItem->parent();
    return parentItem == m_root ? QModelIndex() : createIndex(parentItem->rowIndex(), 0, parentItem);
}

int MotionBaseModel::rowCount(const QModelIndex &parent) const
{
    ITreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_root;
    else
        parentItem = static_cast<ITreeItem *>(parent.internalPointer());

    return parentItem->countChildren();
}

const QModelIndex MotionBaseModel::frameToIndex(ITreeItem *item, int frameIndex) const
{
    int rowIndex = item->rowIndex();
    const QModelIndex &parentIndex = index(item->parent()->rowIndex(), 0);
    // column index 0 is row header
    const QModelIndex modelIndex = index(rowIndex, frameIndex + 1, parentIndex);
    if (!modelIndex.isValid())
        createIndex(rowIndex, frameIndex, item);
    return modelIndex;
}

void MotionBaseModel::saveState()
{
    if (m_model) {
        m_model->discardState(m_state);
        m_state = m_model->saveState();
    }
}

void MotionBaseModel::restoreState()
{
    if (m_model) {
        m_model->restoreState(m_state);
        m_model->updateImmediate();
        m_model->discardState(m_state);
    }
}

void MotionBaseModel::discardState()
{
    if (m_model)
        m_model->discardState(m_state);
}

int MotionBaseModel::columnCount(const QModelIndex & /* parent */) const
{
    return maxFrameCount() + 2;
}

void MotionBaseModel::markAsNew(vpvl::PMDModel *model)
{
    if (model == m_model)
        setModified(false);
}

void MotionBaseModel::refreshModel()
{
    updateModel();
    reset();
    emit motionDidUpdate(m_model);
}

int MotionBaseModel::maxFrameCount() const
{
    return 54000;
}

void MotionBaseModel::updateModel()
{
    if (m_model)
        m_model->updateImmediate();
}

void MotionBaseModel::addUndoCommand(QUndoCommand *command)
{
    m_undo->activeStack()->push(command);
}

void MotionBaseModel::setPMDModel(vpvl::PMDModel *model)
{
    if (!m_stacks.contains(model)) {
        QUndoStack *stack = new QUndoStack();
        m_stacks.insert(model, stack);
        m_undo->addStack(stack);
        m_undo->setActiveStack(stack);
    }
    else {
        m_undo->setActiveStack(m_stacks[model]);
    }
    m_model = model;
    emit modelDidChange(model);
}

void MotionBaseModel::clearKeys()
{
    m_keys.clear();
}

void MotionBaseModel::clearValues()
{
    m_values[m_model].clear();
    m_values.remove(m_model);
}

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
}

void MotionBaseModel::saveState()
{
    if (m_model)
        m_state = m_model->saveState();
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
    return 1800;
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

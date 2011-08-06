#include "MotionBaseModel.h"
#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

MotionBaseModel::MotionBaseModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_model(0),
    m_state(0)
{
}

MotionBaseModel::~MotionBaseModel()
{
    if (m_model)
        m_model->discardState(m_state);
    if (m_state)
        qWarning("It seems memory leak occured: m_state");
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

int MotionBaseModel::rowCount(const QModelIndex &parent) const
{
    return m_keys.count();
}

int MotionBaseModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 600;
}

QVariant MotionBaseModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        switch(role) {
        case Qt::UserRole:
            return m_values.value(QPair<int, int>(index.column(), index.row()));
        case Qt::DisplayRole:
            return m_keys[index.row()];
        }
    }
    return QVariant();
}

bool MotionBaseModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        m_values.insert(QPair<int, int>(index.column(), index.row()), value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QVariant MotionBaseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical) {
        return m_keys[section];
    }
    else if (orientation == Qt::Horizontal)
        return " "; //QString("%1").setNum(section + 1);
    return QVariant();
}

bool MotionBaseModel::updateModel()
{
    if (m_model) {
        m_model->updateImmediate();
        return true;
    }
    else {
        return false;
    }
}

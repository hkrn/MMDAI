#include "MotionBaseModel.h"
#include <vpvl/vpvl.h>

MotionBaseModel::MotionBaseModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_model(0)
{
}

int MotionBaseModel::rowCount(const QModelIndex &parent) const
{
    return m_keys.count();
}

int MotionBaseModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1800;
}

QVariant MotionBaseModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();
    return m_values.value(QPair<int, int>(index.column(), index.row()));
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

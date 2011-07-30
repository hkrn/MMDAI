#include "MotionBaseModel.h"
#include <QtGui/QtGui>
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
        m_model->updateRootBone();
        m_model->updateMotion(0);
        m_model->updateSkins();
        return true;
    }
    else {
        return false;
    }
}

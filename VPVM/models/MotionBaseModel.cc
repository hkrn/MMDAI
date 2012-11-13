/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

#include "MotionBaseModel.h"

namespace vpvm
{

int MotionBaseModel::toTimeIndex(const QModelIndex &index)
{
    return toTimeIndex(index.column());
}

int MotionBaseModel::toTimeIndex(int modelColumnIndex)
{
    // column index 0 is row header
    return qMax(modelColumnIndex - 1, 0);
}

int MotionBaseModel::toModelIndex(int frameIndex)
{
    // column index 0 is row header
    return qMax(frameIndex + 1, 0);
}

MotionBaseModel::MotionBaseModel(QUndoGroup *undoRef, QObject *parent)
    : QAbstractTableModel(parent),
      m_motionRef(0),
      m_undoRef(undoRef),
      m_timeIndex(0),
      m_frameIndexColumnMax(kFrameIndexColumnMinimum),
      m_frameIndexColumnOffset(kFrameIndexColumnMinimum),
      m_modified(false)
{
}

MotionBaseModel::~MotionBaseModel()
{
}

QVariant MotionBaseModel::headerData(int /* section */, Qt::Orientation /* orientation */, int /* role */) const
{
    return QVariant();
}

QModelIndex MotionBaseModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    ITreeItem *parentItem = 0;
    if (!parent.isValid())
        parentItem = rootRef();
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
    return parentItem == rootRef() ? QModelIndex() : createIndex(parentItem->rowIndex(), 0, parentItem);
}

int MotionBaseModel::rowCount(const QModelIndex &parent) const
{
    ITreeItem *parentItem;
    if (parent.column() > 0)
        return 0;
    if (!parent.isValid()) {
        parentItem = rootRef();
        if (!parentItem)
            return 0;
    }
    else {
        parentItem = static_cast<ITreeItem *>(parent.internalPointer());
    }
    return parentItem->countChildren();
}

void MotionBaseModel::cutKeyframesByModelIndices(const QModelIndexList &indices, int frameIndex)
{
    copyKeyframesByModelIndices(indices, frameIndex);
    deleteKeyframesByModelIndices(indices);
}

QItemSelection MotionBaseModel::selectKeyframesFromItemSelection(const QItemSelection &selection)
{
    const QModelIndexList &indices = selection.indexes();
    QItemSelection newSelection;
    foreach (const QModelIndex &index, indices) {
        const QVariant &variant = index.data(kBinaryDataRole);
        if (variant.canConvert(QVariant::ByteArray))
            newSelection.select(index, index);
    }
    return newSelection;
}

void MotionBaseModel::setTimeIndex(const vpvl2::IKeyframe::TimeIndex &newIndex)
{
    int oldIndex = m_timeIndex;
    if (oldIndex != newIndex) {
        m_timeIndex = newIndex;
        emit timeIndexDidChange(newIndex, oldIndex);
    }
}

void MotionBaseModel::setModified(bool value)
{
    m_modified = value;
    emit motionDidModify(value);
}

bool MotionBaseModel::canFetchMore(const QModelIndex & /* parent */) const
{
    return m_frameIndexColumnOffset < m_frameIndexColumnMax;
}

void MotionBaseModel::fetchMore(const QModelIndex &parent)
{
    int remain = m_frameIndexColumnMax - m_frameIndexColumnOffset;
    int step = kFrameIndexColumnStep;
    int itemsToFetch = qMin(step, remain);
    if (itemsToFetch > 0) {
        beginInsertColumns(parent, m_frameIndexColumnOffset, m_frameIndexColumnOffset + itemsToFetch - 1);
        m_frameIndexColumnOffset += itemsToFetch;
        endInsertColumns();
    }
}

int MotionBaseModel::frameIndexColumnMax() const
{
    return m_frameIndexColumnMax;
}

void MotionBaseModel::setFrameIndexColumnMax(int newValue)
{
    /* 最小値はモーションの最大フレーム値を優先する (ない場合は0) */
    if (newValue < maxFrameIndex())
        newValue = maxFrameIndex();
    setFrameIndexColumnMax0(newValue);
}

void MotionBaseModel::setFrameIndexColumnMax(const vpvl2::IMotion *motion)
{
    setFrameIndexColumnMax0(motion->maxTimeIndex());
}

void MotionBaseModel::addUndoCommand(QUndoCommand *command)
{
    QUndoStack *activeStack = m_undoRef->activeStack();
    if (activeStack)
        activeStack->push(command);
}

void MotionBaseModel::resetModel()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    beginResetModel();
    endResetModel();
#else
    reset();
#endif
}

void MotionBaseModel::setFrameIndexColumnMax0(int newValue)
{
    int oldValue = m_frameIndexColumnMax;
    /* 下限は kFrameIndexColumnMinimum (=30) を採用する */
    if (newValue < kFrameIndexColumnMinimum)
        newValue = kFrameIndexColumnMinimum;
    /* 元の値より小さい場合は余剰分のカラムの削除を実行 */
    if (m_frameIndexColumnMax > newValue) {
        removeColumns(newValue, m_frameIndexColumnMax - newValue);
        m_frameIndexColumnOffset = newValue;
    }
    m_frameIndexColumnMax = newValue;
    m_frameIndexColumnOffset = newValue;
    resetModel();
    emit frameIndexColumnMaxDidChange(newValue, oldValue);
}

} /* namespace vpvm */

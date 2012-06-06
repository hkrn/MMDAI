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

#include "models/BoneMotionModel.h"
#include "models/MorphMotionModel.h"
#include "models/PMDMotionModel.h"
#include "models/SceneMotionModel.h"
#include "widgets/TimelineTreeView.h"

#include <QtGui/QtGui>

TimelineTreeView::TimelineTreeView(QItemDelegate *delegate, QWidget *parent)
    : QTreeView(parent)
{
    setItemDelegate(delegate);
    setExpandsOnDoubleClick(true);
    setUniformRowHeights(true);
    setSortingEnabled(false);
    setItemsExpandable(false);
    m_frozenTreeView = new QTreeView(this);
    QItemDelegate *itemDelegate = new QItemDelegate();
    m_frozenTreeView->setItemDelegate(itemDelegate);
    connect(m_frozenTreeView, SIGNAL(collapsed(QModelIndex)), SLOT(addCollapsed(QModelIndex)));
    connect(m_frozenTreeView, SIGNAL(expanded(QModelIndex)), SLOT(addExpanded(QModelIndex)));
    connect(m_frozenTreeView->verticalScrollBar(), SIGNAL(valueChanged(int)),
            verticalScrollBar(), SLOT(setValue(int)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            m_frozenTreeView->verticalScrollBar(), SLOT(setValue(int)));
    connect(header(), SIGNAL(sectionResized(int,int,int)), SLOT(updateSectionWidth(int,int,int)));
}

TimelineTreeView::~TimelineTreeView()
{
}

void TimelineTreeView::initializeFrozenView()
{
    QAbstractItemModel *m = model();
    m_frozenTreeView->setModel(m);
    m_frozenTreeView->header()->setResizeMode(QHeaderView::Fixed);
    viewport()->stackUnder(m_frozenTreeView);
    updateFrozenTreeView();
    m_frozenTreeView->setColumnWidth(0, columnWidth(0));
    m_frozenTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_frozenTreeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_frozenTreeView->show();
    updateFrozenTreeViewGeometry();
}

void TimelineTreeView::resizeEvent(QResizeEvent *event)
{
    QTreeView::resizeEvent(event);
    resizeColumnToContents(0);
    updateFrozenTreeViewGeometry();
    m_frozenTreeView->setColumnWidth(0, columnWidth(0));
}

QModelIndex TimelineTreeView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    const QModelIndex &ci = currentIndex();
    int column = ci.column();
    if (column == 0) {
        return QModelIndex();
    }
    else if (column == 1 && cursorAction == MoveLeft) {
        return ci;
    }
    else {
        const QModelIndex &current = QTreeView::moveCursor(cursorAction, modifiers);
        const int x = visualRect(current).topLeft().x();
        if(cursorAction == MoveLeft && current.column() > 0 && x < m_frozenTreeView->columnWidth(0)){
            const int newValue = horizontalScrollBar()->value() + x - m_frozenTreeView->columnWidth(0);
            horizontalScrollBar()->setValue(newValue);
        }
        return current;
    }
}

void TimelineTreeView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    if (index.column() > 0)
        QTreeView::scrollTo(index, hint);
}

void TimelineTreeView::updateFrozenTreeView()
{
    int ncolumns = model()->columnCount();
    for (int i = 1; i < ncolumns; i++)
        m_frozenTreeView->setColumnHidden(i, true);
}

void TimelineTreeView::updateSectionWidth(int logicalIndex, int newSize, int /* oldSize */)
{
    if (logicalIndex == 0) {
        m_frozenTreeView->setColumnWidth(0, newSize);
        updateFrozenTreeViewGeometry();
    }
}

void TimelineTreeView::updateFrozenTreeViewGeometry()
{
    const QRect rect(frameWidth(),
                     frameWidth(),
                     columnWidth(0),
                     viewport()->height() + header()->height());
    m_frozenTreeView->setGeometry(rect);
}

QItemSelectionModel *TimelineTreeView::frozenViewSelectionModel() const
{
    return m_frozenTreeView->selectionModel();
}

void TimelineTreeView::selectFrameIndices(const QList<int> &frameIndices, bool registeredOnly)
{
    QItemSelection selection;
    if (PMDMotionModel *pmm = qobject_cast<PMDMotionModel *>(model())) {
        /* 現在のキーフレームのインデックスから全てのボーンまたは頂点モーフを選択する処理 */
        foreach (PMDMotionModel::ITreeItem *item, pmm->keys().values()) {
            foreach (int frameIndex, frameIndices) {
                const QModelIndex &index = pmm->frameIndexToModelIndex(item, frameIndex);
                if (registeredOnly && !index.data(MotionBaseModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
                    continue;
                selection.append(QItemSelectionRange(index));
            }
        }
        selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    }
    else if (SceneMotionModel *smm = qobject_cast<SceneMotionModel *>(model())) {
        /* 現在のキーフレームのインデックスから現時点ではカメラフレームのみを選択する処理 */
        foreach (int frameIndex, frameIndices) {
            const QModelIndex &index = smm->index(0, MotionBaseModel::toModelIndex(frameIndex));
            if (registeredOnly && !index.data(MotionBaseModel::kBinaryDataRole).canConvert(QVariant::ByteArray))
                continue;
            selection.append(QItemSelectionRange(index));
        }
        selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    }
}

void TimelineTreeView::addKeyframesBySelectedIndices()
{
    MotionBaseModel *m = static_cast<MotionBaseModel *>(model());
    const QModelIndexList &indices = selectionModel()->selectedIndexes();
    m->addKeyframesByModelIndices(indices);
}

void TimelineTreeView::deleteKeyframesBySelectedIndices()
{
    MotionBaseModel *m = static_cast<MotionBaseModel *>(model());
    const QModelIndexList &indices = selectionModel()->selectedIndexes();
    m->deleteKeyframesByModelIndices(indices);
}

void TimelineTreeView::setBoneKeyframesWeightBySelectedIndices(const vpvl2::Vector3 &position,
                                                               const vpvl2::Vector3 &rotation)
{
    if (BoneMotionModel *m = qobject_cast<BoneMotionModel *>(model())) {
        const QModelIndexList &indices = selectionModel()->selectedIndexes();
        m->applyKeyframeWeightByModelIndices(indices, position, rotation);
    }
}

void TimelineTreeView::setMorphKeyframesWeightBySelectedIndices(float value)
{
    if (MorphMotionModel *m = qobject_cast<MorphMotionModel *>(model())) {
        const QModelIndexList &indices = selectionModel()->selectedIndexes();
        m->applyKeyframeWeightByModelIndices(indices, value);
    }
}

void TimelineTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    const QModelIndex &index = indexAt(event->pos());
    if (index.isValid()) {
        QModelIndexList indices;
        indices.append(index);
        emit modelIndexDidSelect(indices);
    }
    QTreeView::mouseDoubleClickEvent(event);
}

void TimelineTreeView::addCollapsed(const QModelIndex &index)
{
    collapse(index);
    m_expanded.removeOne(index);
    updateFrozenTreeViewGeometry();
}

void TimelineTreeView::addExpanded(const QModelIndex &index)
{
    expand(index);
    if (!m_expanded.contains(index))
        m_expanded.append(index);
    updateFrozenTreeViewGeometry();
}

void TimelineTreeView::selectModelIndices(const QItemSelection &selected, const QItemSelection & /* deselected */)
{
    QAbstractItemModel *m = model();
    QModelIndexList names, indices;
    /* ボーンまた頂点モーフのモデルである PMDMotionModel のクラスである */
    if (PMDMotionModel *pmm = qobject_cast<PMDMotionModel *>(m)) {
        QItemSelectionModel *sm = selectionModel();
        foreach (const QModelIndex &index, selected.indexes()) {
            PMDMotionModel::ITreeItem *item = static_cast<PMDMotionModel::ITreeItem *>(index.internalPointer());
            /* ボーンまたは頂点フレームのカテゴリ名またはキーフレームが選択されていることを確認する */
            int column = index.column();
            if (item && !item->isRoot()) {
                if (item->isCategory()) {
                    int nchildren = item->countChildren();
                    for (int i = 0; i < nchildren; i++) {
                        if (column == 0) {
                            const QModelIndex &child = pmm->index(i, 0, index);
                            names.append(child);
                            sm->select(child, QItemSelectionModel::Select);
                        }
                        else {
                            int frameIndex = PMDMotionModel::toFrameIndex(column);
                            const QModelIndex &child = pmm->frameIndexToModelIndex(item->child(i), frameIndex);
                            indices.append(child);
                            sm->select(child, QItemSelectionModel::Select);
                        }
                    }
                }
                else {
                    column == 0 ? names.append(index) : indices.append(index);
                }
            }
        }
        /* テーブルモデルがボーンの場合は該当ボーンを選択状態にする処理を入れる */
        BoneMotionModel *bmm = 0;
        if (!names.empty() && (bmm = qobject_cast<BoneMotionModel *>(pmm)))
            bmm->selectBonesByModelIndices(names);
    }
}

void TimelineTreeView::restoreExpandState()
{
    foreach (const QModelIndex &index, m_expanded)
        m_frozenTreeView->expand(index);
}

TimelineHeaderView::TimelineHeaderView(Qt::Orientation orientation,
                                       bool stretchLastSection,
                                       QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    setResizeMode(QHeaderView::Fixed);
    setStretchLastSection(stretchLastSection);
    setClickable(true);
    setMovable(false);
    setSortIndicatorShown(false);
    setDefaultSectionSize(16);
}

TimelineHeaderView::~TimelineHeaderView()
{
}

void TimelineHeaderView::mousePressEvent(QMouseEvent *e)
{
    /* setMovable(false) にすると何故か sectionPressed が効かなくなるので mousePressEvent でシグナルを発行する */
    int modelIndex = logicalIndexAt(e->pos());
    emit frameIndexDidSelect(MotionBaseModel::toFrameIndex(modelIndex));
    QHeaderView::mousePressEvent(e);
}

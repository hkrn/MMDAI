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

#include "models/PMDMotionModel.h"
#include "models/SceneMotionModel.h"
#include "widgets/TimelineTreeView.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

TimelineTreeView::TimelineTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setExpandsOnDoubleClick(true);
    setUniformRowHeights(true);
    setSortingEnabled(false);
    connect(this, SIGNAL(collapsed(QModelIndex)), this, SLOT(addCollapsed(QModelIndex)));
    connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(addExpanded(QModelIndex)));
}

TimelineTreeView::~TimelineTreeView()
{
}

void TimelineTreeView::selectFrameIndex(int frameIndex)
{
    QList<int> frameIndices;
    frameIndices.append(frameIndex);
    selectFrameIndices(frameIndices, false);
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
    m->addKeyframesByModelIndices(selectionModel()->selectedIndexes());
}

void TimelineTreeView::deleteKeyframesBySelectedIndices()
{
    MotionBaseModel *m = qobject_cast<MotionBaseModel *>(model());
    m->deleteKeyframesByModelIndices(selectionModel()->selectedIndexes());
}

void TimelineTreeView::copyKeyframes(int frameIndex)
{
    static_cast<MotionBaseModel *>(model())->copyKeyframes(frameIndex);
}

void TimelineTreeView::pasteKeyframes(int frameIndex)
{
    static_cast<MotionBaseModel *>(model())->pasteKeyframes(frameIndex);
}

void TimelineTreeView::setKeyframeWeightBySelectedIndices(float /* value */)
{
}

void TimelineTreeView::mousePressEvent(QMouseEvent *event)
{
    /* ボーンまたは頂点モーフの名前から対象を選択する処理 */
    const QModelIndex &index = indexAt(event->pos());
    QAbstractItemModel *m = model();
    /* ボーンまた頂点モーフのモデルである PMDMotionModel のクラスである */
    if (PMDMotionModel *pmm = qobject_cast<PMDMotionModel *>(m)) {
        PMDMotionModel::ITreeItem *item = static_cast<PMDMotionModel::ITreeItem *>(index.internalPointer());
        /* ルートでもカテゴリでもなく、ボーンまたは頂点フレームのキーフレームが選択されていることを確認する */
        if (item && !item->isRoot() && !item->isCategory()) {
            selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            pmm->selectKeyframesByModelIndices(selectionModel()->selection().indexes());
        }
    }
    /* 場面のモデルである SceneMotionModel のクラスである */
    else if (SceneMotionModel *smm = qobject_cast<SceneMotionModel *>(m)) {
        SceneMotionModel::ITreeItem *item = static_cast<SceneMotionModel::ITreeItem *>(index.internalPointer());
        /* ルートでもカテゴリでもなく、カメラまたは照明のキーフレームが選択されていることを確認する */
        if (item && !item->isRoot() && !item->isCategory())
            smm->selectKeyframesByModelIndex(index);
    }
    QTreeView::mousePressEvent(event);
}

const QModelIndexList &TimelineTreeView::expandedModelIndices() const
{
    return m_expanded;
}

void TimelineTreeView::addCollapsed(const QModelIndex &index)
{
    m_expanded.removeOne(index);
}

void TimelineTreeView::addExpanded(const QModelIndex &index)
{
    if (!m_expanded.contains(index))
        m_expanded.append(index);
}

TimelineHeaderView::TimelineHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setDefaultAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    setResizeMode(QHeaderView::Fixed);
    setClickable(true);
    setMovable(false);
    setStretchLastSection(true);
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

/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#include "TimelineWidget.h"
#include "BoneMotionModel.h"
#include "FaceMotionModel.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

namespace {

class TimelineItemDelegate : public QItemDelegate
{
public:
    TimelineItemDelegate(QObject *parent = 0) : QItemDelegate(parent) {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        if (index.column() == 0) {
            QItemDelegate::paint(painter, option, index);
            return;
        }
        // column index 0 is row header
        if ((index.column() - 1) % 5 == 0)
            painter->fillRect(option.rect, qApp->palette().alternateBase());
        if (index.data(MotionBaseModel::kBinaryDataRole).canConvert(QVariant::ByteArray)) {
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing);
            if (option.state & QStyle::State_Selected)
                painter->setBrush(option.palette.highlight());
            else
                painter->setBrush(option.palette.foreground());
            const QRect &rect = option.rect;
            int width = rect.height();
            int height = width;
            int xoffset = rect.x() + ((rect.width() - width) / 2);
            int yoffset = rect.y() + ((rect.height() - height) / 2);
            QPolygon polygon;
            polygon.append(QPoint(xoffset, yoffset + height / 2));
            polygon.append(QPoint(xoffset + width / 2, yoffset + height));
            polygon.append(QPoint(xoffset + width, yoffset + height / 2));
            polygon.append(QPoint(xoffset + width / 2, yoffset ));
            painter->drawPolygon(polygon);
        }
    }
};

}

TimelineTreeView::TimelineTreeView(QWidget *parent)
    : QTreeView(parent) {
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
    QItemSelection selection;
    MotionBaseModel *m = static_cast<MotionBaseModel *>(model());
    foreach (MotionBaseModel::ITreeItem *item, m->keys().values()) {
        const QModelIndex &index = m->frameToIndex(item, frameIndex);
        selection.append(QItemSelectionRange(index));
    }
    QItemSelectionModel *sm = selectionModel();
    sm->select(selection, QItemSelectionModel::ClearAndSelect);
}

const QModelIndexList &TimelineTreeView::expandedIndices() const
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

TimelineWidget::TimelineWidget(MotionBaseModel *base,
                               QWidget *parent) :
    QWidget(parent),
    m_treeView(0)
{
    TimelineTreeView *treeView = new TimelineTreeView();
    treeView->setModel(base);
    QHeaderView *header = treeView->header();
    header->setSortIndicatorShown(false);
    header->setResizeMode(QHeaderView::Fixed);
    header->setResizeMode(0, QHeaderView::ResizeToContents);
    header->setDefaultSectionSize(15);
    TimelineItemDelegate *delegate = new TimelineItemDelegate(this);
    treeView->setItemDelegate(delegate);
    m_spinBox = new QSpinBox();
    m_spinBox->setMaximum(base->maxFrameCount());
    connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(setFrameIndex(int)));
    m_label = new QLabel();
    QHBoxLayout *spinboxLayout = new QHBoxLayout();
    spinboxLayout->addWidget(m_label);
    spinboxLayout->addWidget(m_spinBox);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(spinboxLayout);
    mainLayout->addWidget(treeView);
    mainLayout->setAlignment(spinboxLayout, Qt::AlignCenter);
    mainLayout->setContentsMargins(QMargins());
    QItemSelectionModel *sm = treeView->selectionModel();
    connect(sm, SIGNAL(currentColumnChanged(QModelIndex,QModelIndex)), this, SLOT(setCurrentColumnIndex(QModelIndex)));
    connect(sm, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(setCurrentRowIndex(QModelIndex)));
    connect(base, SIGNAL(motionDidUpdate(vpvl::PMDModel*)), this, SLOT(reexpand()));
    connect(base, SIGNAL(motionDidUpdate(vpvl::PMDModel*)), this, SLOT(setCurrentFrameIndexBySpinBox()));
    retranslate();
    setLayout(mainLayout);
    m_treeView = treeView;
}

TimelineWidget::~TimelineWidget()
{
    delete m_treeView;
}

void TimelineWidget::retranslate()
{
    m_label->setText(tr("Frame Index"));
}

int TimelineWidget::frameIndex() const
{
    const QModelIndexList &indices = m_treeView->selectionModel()->selectedIndexes();
    if (!indices.isEmpty()) {
        const QModelIndex &index = indices.first();
        if (index.isValid())
            return index.column() - 1;
    }
    return 0;
}

void TimelineWidget::setCurrentColumnIndex(const QModelIndex &index)
{
    int frameIndex = qMax(index.column() - 1, 0);
    setFrameIndex(frameIndex);
}

void TimelineWidget::setCurrentRowIndex(const QModelIndex & /* index */)
{
}

void TimelineWidget::setFrameIndex(int frameIndex)
{
    MotionBaseModel *model = static_cast<MotionBaseModel *>(m_treeView->model());
    model->setFrameIndex(frameIndex);
    m_treeView->selectFrameIndex(frameIndex);
    m_treeView->scrollTo(model->index(0, frameIndex));
    m_spinBox->setValue(frameIndex);
    emit motionDidSeek(static_cast<float>(frameIndex));
}

void TimelineWidget::setCurrentFrameIndexBySpinBox()
{
    setFrameIndex(m_spinBox->value());
}

void TimelineWidget::reexpand()
{
    TimelineTreeView *view = static_cast<TimelineTreeView *>(m_treeView);
    foreach (const QModelIndex &index, view->expandedIndices())
        m_treeView->expand(index);
}

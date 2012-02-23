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
#include "widgets/TimelineWidget.h"
#include "widgets/TimelineTreeView.h"

#include <QtGui/QtGui>

namespace {

class TimelineItemDelegate : public QItemDelegate
{
public:
    TimelineItemDelegate(QObject *parent = 0) : QItemDelegate(parent) {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        /* ボーンまたは頂点モーフの名前を表示する部分は継承元のクラスに描画を任せる */
        if (index.column() == 0) {
            QItemDelegate::paint(painter, option, index);
            return;
        }
        painter->save();
        /* キーフレームのインデックスが5で割り切れる場合は背景を白ではない色にする */
        if (MotionBaseModel::toFrameIndex(index) % 5 == 0)
            painter->fillRect(option.rect, qApp->palette().alternateBase());
        painter->setRenderHint(QPainter::Antialiasing);
        MotionBaseModel::ITreeItem *item = static_cast<MotionBaseModel::ITreeItem *>(index.internalPointer());
        /* カテゴリ内のボーンリストにひとつでもキーフレームが含まれていればダイアモンドマークを表示する */
        const PMDMotionModel *m = qobject_cast<const PMDMotionModel *>(index.model());
        if (m && item->isCategory()) {
            int nchildren = item->countChildren(), frameIndex = MotionBaseModel::toFrameIndex(index.column());
            bool dataFound = false;
            for (int i = 0; i < nchildren; i++) {
                const QModelIndex &mi = m->frameIndexToModelIndex(item->child(i), frameIndex);
                if (mi.data(MotionBaseModel::kBinaryDataRole).canConvert(QVariant::ByteArray)) {
                    dataFound = true;
                    break;
                }
            }
            if (dataFound) {
                painter->setBrush(Qt::NoBrush);
                if (option.state & QStyle::State_Selected)
                    painter->setPen(Qt::NoPen);
                else
                    painter->setPen(Qt::black);
                drawDiamond(painter, option);
            }
        }
        /* モデルのデータにキーフレームのバイナリが含まれていればダイアモンドマークを表示する */
        painter->setPen(Qt::NoPen);
        if (index.data(MotionBaseModel::kBinaryDataRole).canConvert(QVariant::ByteArray)) {
            painter->setBrush(option.state & QStyle::State_Selected ? Qt::red : option.palette.foreground());
            drawDiamond(painter, option);
        }
        else if (option.state & QStyle::State_Selected) {
            painter->setBrush(option.palette.highlight());
            drawDiamond(painter, option);
        }
        painter->restore();
    }

private:
    void drawDiamond(QPainter *painter, const QStyleOptionViewItem &option) const {
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
};

}

TimelineWidget::TimelineWidget(MotionBaseModel *base,
                               QWidget *parent) :
    QWidget(parent)
{
    TimelineTreeView *treeView = new TimelineTreeView();
    treeView->setModel(base);
    treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    /* 専用の選択処理を行うようにスロットを追加する */
    connect(treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            treeView, SLOT(selectModelIndices(QItemSelection,QItemSelection)));
    TimelineHeaderView *header = new TimelineHeaderView(Qt::Horizontal);
    connect(header, SIGNAL(frameIndexDidSelect(int)), this, SLOT(setCurrentFrameIndex(int)));
    treeView->setHeader(header);
    header->setResizeMode(0, QHeaderView::ResizeToContents);
    TimelineItemDelegate *delegate = new TimelineItemDelegate(this);
    treeView->setItemDelegate(delegate);
    m_spinBox = new QSpinBox();
    m_spinBox->setMaximum(base->maxFrameCount());
    /* フレームインデックスの移動と共に SceneWidget にシークを実行する(例外あり) */
    connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(setCurrentFrameIndex(int)));
    m_label = new QLabel();
    m_button = new QPushButton();
    /* キーフレームの登録処理 */
    connect(m_button, SIGNAL(clicked()), treeView, SLOT(addKeyframesBySelectedIndices()));
    QHBoxLayout *spinboxLayout = new QHBoxLayout();
    spinboxLayout->addWidget(m_label);
    spinboxLayout->addWidget(m_spinBox);
    spinboxLayout->addWidget(m_button);
    spinboxLayout->setAlignment(Qt::AlignCenter);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(spinboxLayout);
    mainLayout->addWidget(treeView);
    mainLayout->setContentsMargins(QMargins());
    QItemSelectionModel *sm = treeView->selectionModel();
    connect(sm, SIGNAL(currentColumnChanged(QModelIndex,QModelIndex)), SLOT(setCurrentColumnIndex(QModelIndex)));
    /* 開閉状態を保持するためのスロットを追加。フレーム移動時に保持した開閉状態を適用する仕組み */
    connect(base, SIGNAL(motionDidUpdate(vpvl::PMDModel*)), SLOT(reexpand()));
    connect(base, SIGNAL(motionDidUpdate(vpvl::PMDModel*)), SLOT(setCurrentFrameIndexBySpinBox()));
    retranslate();
    setLayout(mainLayout);
    m_treeView = treeView;
    m_headerView = header;
}

TimelineWidget::~TimelineWidget()
{
}

void TimelineWidget::retranslate()
{
    m_label->setText(tr("Frame Index"));
    m_button->setText(tr("Register"));
}

int TimelineWidget::frameIndex() const
{
    /* 選択状態のモデルインデックスの最初のインデックスからキーフレームのインデックスを求める */
    const QModelIndexList &indices = m_treeView->selectionModel()->selectedIndexes();
    if (!indices.isEmpty()) {
        const QModelIndex &index = indices.first();
        if (index.isValid())
            return MotionBaseModel::toFrameIndex(index);
    }
    return 0;
}

void TimelineWidget::setEnableFrameIndexSpinBox(bool value)
{
    m_spinBox->setEnabled(value);
    m_button->setEnabled(value);
}

void TimelineWidget::setCurrentColumnIndex(const QModelIndex &index)
{
    int frameIndex = MotionBaseModel::toFrameIndex(index);
    setCurrentFrameIndex(frameIndex);
}

void TimelineWidget::setCurrentFrameIndex(int frameIndex)
{
    /* キーフレームのインデックスを現在の位置として設定し、フレームの列を全て選択状態にした上でスクロールを行う */
    MotionBaseModel *model = qobject_cast<MotionBaseModel *>(m_treeView->model());
    model->setFrameIndex(frameIndex);
    m_treeView->selectFrameIndex(frameIndex);
    m_treeView->scrollTo(model->index(0, MotionBaseModel::toModelIndex(frameIndex)));
    m_spinBox->setValue(frameIndex);
    /* モーション移動を行わせるようにシグナルを発行する */
    emit motionDidSeek(static_cast<float>(frameIndex));
}

void TimelineWidget::setCurrentFrameIndexBySpinBox()
{
    setCurrentFrameIndex(m_spinBox->value());
}

void TimelineWidget::reexpand()
{
    /* QAbstractTableModel#reset が行われると QTreeView では閉じてしまうので、開閉状態を reset 前に戻す */
    foreach (const QModelIndex &index, m_treeView->expandedModelIndices())
        m_treeView->expand(index);
}

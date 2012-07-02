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
        /* カテゴリ内のボーンリストにひとつでもキーフレームが含まれていれば中空きのダイアモンドマークを表示する */
        const PMDMotionModel *m = qobject_cast<const PMDMotionModel *>(index.model());
        bool isCategory = item->isCategory(), hasCategoryData = false;
        if (m && isCategory) {
            int nchildren = item->countChildren(), frameIndex = MotionBaseModel::toFrameIndex(index.column());
            bool dataFound = false;
            /* カテゴリ内の登録済みのキーフレームを探す */
            for (int i = 0; i < nchildren; i++) {
                const QModelIndex &mi = m->frameIndexToModelIndex(item->child(i), frameIndex);
                if (mi.data(MotionBaseModel::kBinaryDataRole).canConvert(QVariant::ByteArray)) {
                    dataFound = true;
                    break;
                }
            }
            if (dataFound) {
                painter->setBrush(Qt::NoBrush);
                /* 選択状態にある場合は赤線枠で表示し、登録済みの場合は黒線枠で表示 */
                if (option.state & QStyle::State_Selected) {
                    painter->setPen(Qt::red);
                    hasCategoryData = true;
                }
                else {
                    painter->setPen(Qt::black);
                }
                drawDiamond(painter, option);
            }
        }
        /* モデルのデータにキーフレームのバイナリが含まれていれば塗りつぶしのダイアモンドマークを表示する。登録済みの場合は赤色で表示 */
        if (index.data(MotionBaseModel::kBinaryDataRole).canConvert(QVariant::ByteArray)) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(option.state & QStyle::State_Selected ? Qt::red : option.palette.foreground());
            drawDiamond(painter, option);
        }
        /* 選択中の場合は選択状態の色(通常は青)で表示。カテゴリの場合は線枠、フレームの場合は塗りつぶし */
        else if (!hasCategoryData && option.state & QStyle::State_Selected) {
            if (isCategory) {
                QPen pen; pen.setBrush(option.palette.highlight());
                painter->setPen(pen);
                painter->setBrush(Qt::NoBrush);
            }
            else {
                painter->setPen(Qt::NoPen);
                painter->setBrush(option.palette.highlight());
            }
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
                               bool stretchLastSection,
                               QWidget *parent)
    : QWidget(parent)
{
    TimelineItemDelegate *delegate = new TimelineItemDelegate(this);
    m_treeView = new TimelineTreeView(base, delegate);
    /* 専用の選択処理を行うようにスロットを追加する */
    connect(m_treeView->horizontalScrollBar(), SIGNAL(actionTriggered(int)), SLOT(adjustFrameColumnSize(int)));
    m_headerView = new TimelineHeaderView(Qt::Horizontal, stretchLastSection);
    connect(m_headerView, SIGNAL(frameIndexDidSelect(int)), SLOT(setCurrentFrameIndexAndSelect(int)));
    m_treeView->setHeader(m_headerView);
    m_headerView->setResizeMode(0, QHeaderView::ResizeToContents);
    m_treeView->initializeFrozenView();
    m_spinBox = new QSpinBox();
    m_spinBox->setAlignment(Qt::AlignRight);
    connect(m_spinBox, SIGNAL(valueChanged(int)), SLOT(setCurrentFrameIndex(int)));
    connect(m_spinBox, SIGNAL(editingFinished()), SLOT(setCurrentFrameIndexAndExpandBySpinBox()));
    m_spinBox->setRange(0, kFrameIndexColumnMax);
    m_spinBox->setWrapping(false);
    /* フレームインデックスの移動と共に SceneWidget にシークを実行する(例外あり) */
    m_label = new QLabel();
    m_button = new QPushButton();
    /* キーフレームの登録処理 */
    connect(m_button, SIGNAL(clicked()), m_treeView, SLOT(addKeyframesBySelectedIndices()));
    QHBoxLayout *spinboxLayout = new QHBoxLayout();
    spinboxLayout->addWidget(m_label);
    spinboxLayout->addWidget(m_spinBox);
    spinboxLayout->addWidget(m_button);
    spinboxLayout->setAlignment(Qt::AlignCenter);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(spinboxLayout);
    mainLayout->addWidget(m_treeView);
    mainLayout->setContentsMargins(QMargins());
    QItemSelectionModel *sm = m_treeView->selectionModel();
    connect(sm, SIGNAL(currentColumnChanged(QModelIndex,QModelIndex)), SLOT(setCurrentFrameIndex(QModelIndex)));
    /* 開閉状態を保持するためのスロットを追加。フレーム移動時に保持した開閉状態を適用する仕組み */
    connect(base, SIGNAL(motionDidUpdate(vpvl2::IModel*)), m_treeView, SLOT(restoreExpandState()));
    connect(base, SIGNAL(motionDidUpdate(vpvl2::IModel*)), SLOT(setCurrentFrameIndexBySpinBox()));
    retranslate();
    setLayout(mainLayout);
}

TimelineWidget::~TimelineWidget()
{
}

void TimelineWidget::retranslate()
{
    m_label->setText(tr("Frame Index"));
    m_button->setText(tr("Register"));
}

int TimelineWidget::currentFrameIndex() const
{
    return m_spinBox->value();
}

int TimelineWidget::selectedFrameIndex() const
{
    /* 選択状態のモデルインデックスの最初のインデックスからキーフレームのインデックスを求める */
    const QModelIndexList &indices = m_treeView->selectionModel()->selectedIndexes();
    if (!indices.isEmpty()) {
        const QModelIndex &index = indices.first();
        if (index.isValid())
            return MotionBaseModel::toFrameIndex(index);
    }
    /* 選択状態のモデルインデックスがない場合はスピンボックス上の現在のフレーム位置を返すようにする */
    return currentFrameIndex();
}

void TimelineWidget::setFrameIndexSpinBoxEnable(bool value)
{
    m_spinBox->setEnabled(value);
    m_button->setEnabled(value);
}

void TimelineWidget::setCurrentFrameIndex(const vpvl2::IKeyframe::Index &frameIndex)
{
    setCurrentFrameIndex(int(frameIndex));
}

void TimelineWidget::setCurrentFrameIndex(int frameIndex)
{
    MotionBaseModel *model = qobject_cast<MotionBaseModel *>(m_treeView->model());
    setCurrentFrameIndex(model->index(0, MotionBaseModel::toModelIndex(frameIndex)));
}

void TimelineWidget::setCurrentFrameIndexBySpinBox()
{
    int frameIndex = m_spinBox->value();
    setCurrentFrameIndex(frameIndex);
}

void TimelineWidget::setCurrentFrameIndexAndExpandBySpinBox()
{
    /* タイムラインを伸縮した上で現在のフレーム位置を選択指定 */
    MotionBaseModel *m = static_cast<MotionBaseModel *>(m_treeView->model());
    int frameIndex = m_spinBox->value();
    m->setFrameIndexColumnMax(frameIndex);
    m_treeView->header()->reset();
    m_treeView->restoreExpandState();
    setCurrentFrameIndexAndSelect(frameIndex);
}

void TimelineWidget::setCurrentFrameIndexAndSelect(int frameIndex)
{
    setCurrentFrameIndex(frameIndex);
    QList<int> frameIndices; frameIndices.append(frameIndex);
    m_treeView->selectFrameIndices(frameIndices, false);
}

void TimelineWidget::setCurrentFrameIndex(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    /* キーフレームのインデックスを現在の位置として設定し、フレームの列を全て選択状態にした上でスクロールを行う */
    MotionBaseModel *model = qobject_cast<MotionBaseModel *>(m_treeView->model());
    int frameIndex = MotionBaseModel::toFrameIndex(index);
    model->setFrameIndex(frameIndex);
    m_treeView->scrollTo(index);
    m_spinBox->setValue(frameIndex);
    /* モーション移動を行わせるようにシグナルを発行する */
    emit motionDidSeek(frameIndex, model->forceCameraUpdate());
}

void TimelineWidget::adjustFrameColumnSize(int value)
{
    QAbstractSlider *slider = qobject_cast<QAbstractSlider *>(sender());
    MotionBaseModel *m = static_cast<MotionBaseModel *>(m_treeView->model());
    int sliderPosition = slider->sliderPosition();
    int frameIndexColumnMax = m->frameIndexColumnMax();
    int frameIndexColumnMin = MotionBaseModel::kFrameIndexColumnMinimum;
    int maxFrameIndex = qMax(m->maxFrameIndex(), frameIndexColumnMin);
    switch (value) {
    case QAbstractSlider::SliderMove:
        if (sliderPosition >= slider->maximum()) {
            /* 列とツリーテーブルの拡張を行う */
            m->setFrameIndexColumnMax(frameIndexColumnMax + MotionBaseModel::kFrameIndexColumnStep);
            slider->setSliderPosition(sliderPosition);
            /* リセットを行うと開閉状態もリセットされるのでリセット前の状態に戻す */
            m_treeView->restoreExpandState();
        }
        /* 列数がモーションの最大値より大きい場合のみ切り詰めを行うようにする */
        else if (sliderPosition <= slider->minimum() && frameIndexColumnMax > maxFrameIndex) {
            /* setFrameIndexColumnMax の値が最大値未満の場合自動的に列が切り詰められるように処理される */
            m->setFrameIndexColumnMax(frameIndexColumnMax - MotionBaseModel::kFrameIndexColumnMinimum);
            /* リセットを行わないと空白部分がヘッダーの方で残ったままになる */
            m_treeView->header()->reset();
            /* リセットを行うと開閉状態もリセットされるのでリセット前の状態に戻す */
            m_treeView->restoreExpandState();
        }
        break;
    case QAbstractSlider::SliderToMaximum:
    case QAbstractSlider::SliderToMinimum:
    case QAbstractSlider::SliderNoAction:
    case QAbstractSlider::SliderSingleStepAdd:
    case QAbstractSlider::SliderSingleStepSub:
    case QAbstractSlider::SliderPageStepAdd:
    case QAbstractSlider::SliderPageStepSub:
    default:
        break;
    }
}

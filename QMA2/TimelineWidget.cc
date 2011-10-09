#include "TimelineWidget.h"
#include "BoneMotionModel.h"
#include "FaceMotionModel.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

namespace {

class TimelineItemDelegate : public QItemDelegate {
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
    : QTreeView(parent)
{
    setExpandsOnDoubleClick(true);
    setUniformRowHeights(true);
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
    sm->select(sm->selection(), QItemSelectionModel::Deselect);
    sm->select(selection, QItemSelectionModel::Select);
}

TimelineWidget::TimelineWidget(MotionBaseModel *base,
                               QWidget *parent) :
    QWidget(parent),
    m_treeView(0)
{
    TimelineTreeView *treeView = new TimelineTreeView();
    treeView->setModel(base);
    QHeaderView *header = treeView->header();
    header->setResizeMode(QHeaderView::Fixed);
    header->setResizeMode(0, QHeaderView::ResizeToContents);
    header->setDefaultSectionSize(15);
    TimelineItemDelegate *delegate = new TimelineItemDelegate(this);
    treeView->setItemDelegate(delegate);
    connect(treeView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(setCurrentIndex(QModelIndex)));
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *spinboxLayout = new QHBoxLayout();
    m_spinBox = new QSpinBox();
    m_spinBox->setMaximum(base->columnCount());
    connect(m_spinBox, SIGNAL(valueChanged(int)), treeView, SLOT(selectFrameIndex(int)));
    connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(setFrameIndex(int)));
    m_label = new QLabel();
    spinboxLayout->addSpacing(250);
    spinboxLayout->addWidget(m_label);
    spinboxLayout->addWidget(m_spinBox);
    spinboxLayout->addSpacing(250);
    layout->addLayout(spinboxLayout);
    layout->addWidget(treeView);
    layout->setContentsMargins(QMargins());
    retranslate();
    setLayout(layout);
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

const QModelIndex TimelineWidget::selectedIndex() const
{
    QModelIndexList indices = m_treeView->selectionModel()->selectedIndexes();
    if (!indices.isEmpty()) {
        QModelIndex index = indices.first();
        if (index.isValid())
            return index;
    }
    return m_treeView->model()->index(0, 0);
}

void TimelineWidget::setCurrentIndex(const QModelIndex index)
{
    int frameIndex = qMax(index.column() - 1, 0);
    setFrameIndex(frameIndex);
}

void TimelineWidget::setFrameIndex(int frameIndex)
{
    m_spinBox->setValue(frameIndex);
    emit motionDidSeek(static_cast<float>(frameIndex));
}

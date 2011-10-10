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

class TimelineTreeView : public QTreeView
{
public:
    TimelineTreeView(QWidget *parent = 0)
        : QTreeView(parent) {
        setExpandsOnDoubleClick(true);
        setUniformRowHeights(true);
    }
    void selectFrameIndex(int frameIndex) {
        QItemSelection selection;
        MotionBaseModel *m = static_cast<MotionBaseModel *>(model());
        foreach (MotionBaseModel::ITreeItem *item, m->keys().values()) {
            const QModelIndex &index = m->frameToIndex(item, frameIndex);
            selection.append(QItemSelectionRange(index));
        }
        QItemSelectionModel *sm = selectionModel();
        sm->select(selection, QItemSelectionModel::ClearAndSelect);
    }
};

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
    QItemSelectionModel *sm = treeView->selectionModel();
    connect(sm, SIGNAL(currentColumnChanged(QModelIndex,QModelIndex)), this, SLOT(setCurrentColumnIndex(QModelIndex)));
    connect(sm, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(setCurrentRowIndex(QModelIndex)));
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *spinboxLayout = new QHBoxLayout();
    m_spinBox = new QSpinBox();
    m_spinBox->setMaximum(base->maxFrameCount());
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
    static_cast<TimelineTreeView*>(m_treeView)->selectFrameIndex(frameIndex);
    m_spinBox->setValue(frameIndex);
    emit motionDidSeek(static_cast<float>(frameIndex));
}

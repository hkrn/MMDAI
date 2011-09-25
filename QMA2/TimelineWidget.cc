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

class TimelineTreeView : public QTreeView {
public:
    TimelineTreeView(QWidget *parent = 0) : QTreeView(parent) {
        setExpandsOnDoubleClick(true);
        setUniformRowHeights(true);
    }
};

}

TimelineWidget::TimelineWidget(MotionBaseModel *base,
                               QWidget *parent) :
    QWidget(parent),
    m_treeView(0)
{
    m_treeView = new TimelineTreeView();
    m_treeView->setModel(base);
    QHeaderView *header = m_treeView->header();
    header->setResizeMode(QHeaderView::Fixed);
    header->setResizeMode(0, QHeaderView::ResizeToContents);
    header->setDefaultSectionSize(15);
    TimelineItemDelegate *delegate = new TimelineItemDelegate(this);
    m_treeView->setItemDelegate(delegate);
    connect(m_treeView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(setCurrentIndex(QModelIndex)));
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *spinboxLayout = new QHBoxLayout();
    m_spinBox = new QSpinBox();
    m_spinBox->setMaximum(base->columnCount());
    connect(m_spinBox, SIGNAL(valueChanged(int)), m_treeView, SLOT(selectColumn(int)));
    spinboxLayout->addSpacing(250);
    spinboxLayout->addWidget(new QLabel(tr("Frame index")));
    spinboxLayout->addWidget(m_spinBox);
    spinboxLayout->addSpacing(250);
    layout->addLayout(spinboxLayout);
    layout->addWidget(m_treeView);
    layout->setContentsMargins(QMargins());
    setLayout(layout);
}

TimelineWidget::~TimelineWidget()
{
    delete m_treeView;
}

const QModelIndex TimelineWidget::selectedIndex() const
{
    QModelIndexList indices = m_treeView->selectionModel()->selectedIndexes();
    if (!indices.isEmpty()) {
        QModelIndex index = indices.first();
        if (index.isValid())
            return index;
    }
    return QModelIndex();
}

void TimelineWidget::setCurrentIndex(const QModelIndex index)
{
    int frameIndex = qMax(index.column() - 1, 0);
    m_spinBox->setValue(frameIndex);
    emit motionDidSeek(static_cast<float>(frameIndex));
}

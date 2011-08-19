#include "TimelineWidget.h"
#include "BoneMotionModel.h"
#include "FaceMotionModel.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

namespace internal {

class TimelineItemDelegate : public QItemDelegate {
public:
    TimelineItemDelegate(QObject *parent = 0) : QItemDelegate(parent) {
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        if (index.column() % 5 == 0 && !(option.state & QStyle::State_Selected))
            painter->fillRect(option.rect, qApp->palette().alternateBase());
        if (index.data(Qt::UserRole).canConvert(QVariant::ByteArray)) {
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setBrush(option.palette.foreground());
            const QRect &rect = option.rect;
            int width = rect.width();
            int height = width;
            int xoffset = rect.x();
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

class TableView : public QTableView {
public:
    TableView(QWidget *parent = 0) : QTableView(parent) {
    }
protected:
    int sizeHintForColumn(int /* column */) const {
        return 16;
    }
};

}

TimelineWidget::TimelineWidget(MotionBaseModel *base,
                               QWidget *parent) :
    QWidget(parent),
    m_tableView(0)
{
    m_tableView = new internal::TableView();
    m_tableView->setShowGrid(true);
    m_tableView->setModel(base);
    internal::TimelineItemDelegate *delegate = new internal::TimelineItemDelegate(this);
    m_tableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    m_tableView->setItemDelegate(delegate);
    m_tableView->resizeColumnsToContents();
    connect(m_tableView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(setCurrentIndex(QModelIndex)));
    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *spinboxLayout = new QHBoxLayout();
    m_spinBox = new QSpinBox();
    m_spinBox->setMaximum(base->columnCount());
    connect(m_spinBox, SIGNAL(valueChanged(int)), m_tableView, SLOT(selectColumn(int)));
    spinboxLayout->addSpacing(250);
    spinboxLayout->addWidget(new QLabel(tr("Frame index")));
    spinboxLayout->addWidget(m_spinBox);
    spinboxLayout->addSpacing(250);
    layout->addLayout(spinboxLayout);
    layout->addWidget(m_tableView);
    layout->setContentsMargins(QMargins());
    setLayout(layout);
}

TimelineWidget::~TimelineWidget()
{
    delete m_tableView;
}

const QModelIndex TimelineWidget::selectedIndex() const
{
    QModelIndexList indices = m_tableView->selectionModel()->selectedIndexes();
    if (!indices.isEmpty()) {
        QModelIndex index = indices.first();
        if (index.isValid())
            return index;
    }
    return QModelIndex();
}

void TimelineWidget::setCurrentIndex(const QModelIndex index)
{
    int frameIndex = index.column();
    m_spinBox->setValue(frameIndex);
    emit motionDidSeek(static_cast<float>(frameIndex));
}

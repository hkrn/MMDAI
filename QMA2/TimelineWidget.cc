#include "TimelineWidget.h"
#include "BoneMotionModel.h"
#include "FaceMotionModel.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

namespace internal {

class TimelineItemDelegate : public QStyledItemDelegate {
public:
    TimelineItemDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        const QRect &rect = option.rect;
        //painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        if (index.column() % 5 == 0)
            painter->setBackground(option.palette.background());
        if (index.data(Qt::UserRole) != QVariant()) {
            painter->setPen(Qt::NoPen);
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setBrush(option.palette.foreground());
            QPolygon polygon;
            int width = rect.width();
            int height = width;
            int xoffset = rect.x();
            int yoffset = rect.y() + ((rect.height() - height) / 2);
            polygon.append(QPoint(xoffset, yoffset + height / 2));
            polygon.append(QPoint(xoffset + width / 2, yoffset + height));
            polygon.append(QPoint(xoffset + width, yoffset + height / 2));
            polygon.append(QPoint(xoffset + width / 2, yoffset ));
            painter->drawPolygon(polygon);
        }
        //painter->restore();
    }
};

}

TimelineWidget::TimelineWidget(MotionBaseModel *base,
                               QWidget *parent) :
    QWidget(parent),
    m_tableView(0)
{
    m_tableView = new QTableView();
    m_tableView->setShowGrid(true);
    m_tableView->setModel(base);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    internal::TimelineItemDelegate *delegate = new internal::TimelineItemDelegate(this);
    m_tableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    m_tableView->setItemDelegate(delegate);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->resizeColumnsToContents();
    connect(m_tableView->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(setCurrentIndex(QModelIndex)));
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_tableView);
    layout->setContentsMargins(QMargins());
    setLayout(layout);
}

TimelineWidget::~TimelineWidget()
{
    delete m_tableView;
}

void TimelineWidget::setCurrentIndex(const QModelIndex index)
{
    emit motionDidSeek(static_cast<float>(index.column()));
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

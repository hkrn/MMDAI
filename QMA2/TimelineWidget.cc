#include "TimelineWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

class ItemDelegate : public QStyledItemDelegate {
public:
    ItemDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & /* index */) const {
        const QRect &rect = option.rect;
        int offset = rect.y() + ((rect.height() - 10) / 2);
        painter->restore();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(option.palette.foreground());
        painter->drawEllipse(rect.x() + 5, offset, 10, 10);
        painter->save();
    }
};

class TableModel : public QAbstractTableModel {
public:
    TableModel(QObject *parent = 0)
        : QAbstractTableModel(parent), m_model(0) {
        setModel(0);
    }

    void setModel(vpvl::PMDModel *model) {
        m_bones.clear();
        if (model) {
            vpvl::BoneList bones = model->bones();
            uint32_t nBones = bones.size();
            QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
            for (uint32_t i = 0; i < nBones; i++) {
                vpvl::Bone *bone = bones.at(i);
                m_bones.append(codec->toUnicode(reinterpret_cast<const char *>(bone->name())));
            }
        }
        else {
            m_bones.append(tr("Camera"));
        }
        reset();
    }

    int rowCount(const QModelIndex & /* parent */) const {
        return m_bones.size();
    }
    int columnCount(const QModelIndex & /* parent */) const {
        return 18000;
    }
    QVariant data(const QModelIndex &index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();
        return QVariant();
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const {
        if (role != Qt::DisplayRole)
            return QVariant();
        if (orientation == Qt::Vertical)
            return m_bones.at(section);
        else if (orientation == Qt::Horizontal)
            return section + 1;
        return QVariant();
    }

private:
    vpvl::PMDModel *m_model;
    QList<QString> m_bones;
};

TimelineWidget::TimelineWidget(QWidget *parent) :
    QWidget(parent),
    m_tableView(0),
    m_tableModel(0),
    m_selectedModel(0)
{
    m_tableModel = new TableModel();
    m_tableView = new QTableView();
    m_tableView->setShowGrid(true);
    m_tableView->setModel(m_tableModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    //ItemDelegate *delegate = new ItemDelegate(this);
    //m_tableView->setItemDelegate(delegate);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_tableView);
    layout->setContentsMargins(0, 0, 0, 0);
    setMinimumSize(260, 480);
    setLayout(layout);
}

TimelineWidget::~TimelineWidget()
{
    delete m_tableModel;
    delete m_tableView;
}

void TimelineWidget::setModel(vpvl::PMDModel *model)
{
    m_selectedModel = model;
    m_tableModel->setModel(model);
}

#include "TimelineWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

class ItemDelegate : public QStyledItemDelegate {
public:
    ItemDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {
    }
    void paint(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QStyledItemDelegate::paint(painter, option, index);
    }
    QSize sizeHint(const QStyleOptionViewItem & /* option */, const QModelIndex & /* index */) const {
        return QSize(5, 5);
    }
};

class TableModel : public QAbstractTableModel {
public:
    TableModel(vpvl::PMDModel *model, QObject *parent = 0)
        : QAbstractTableModel(parent), m_model(model) {
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
    }

    int rowCount(const QModelIndex & /* parent */) const {
        return m_bones.size();
    }
    int columnCount(const QModelIndex & /* parent */) const {
        return 100;
    }
    QVariant data(const QModelIndex &index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();
        return QVariant();
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const {
        if (orientation == Qt::Vertical && role != Qt::SizeHintRole)
            return m_bones.at(section);
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
    m_tableModel = new TableModel(0, this);
    m_tableView = new QTableView();
    m_tableView->setShowGrid(true);
    m_tableView->horizontalHeader()->hide();
    m_tableView->setModel(m_tableModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
    ItemDelegate *delegate = new ItemDelegate(this);
    m_tableView->setItemDelegate(delegate);
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_tableView);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    setMinimumSize(640, 480);
}

TimelineWidget::~TimelineWidget()
{
    delete m_tableModel;
}

void TimelineWidget::setModel(vpvl::PMDModel *model)
{
    m_selectedModel = model;
    delete m_tableModel;
    m_tableModel = new TableModel(model);
    m_tableView->setModel(m_tableModel);
}

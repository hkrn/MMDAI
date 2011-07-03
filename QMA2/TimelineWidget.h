#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QtCore/QModelIndex>
#include <QtGui/QWidget>

namespace vpvl {
class Bone;
class PMDModel;
}

class QTableView;
class TableModel;

class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineWidget(QWidget *parent = 0);
    ~TimelineWidget();

public slots:
    void setModel(vpvl::PMDModel *value);
    void selectCell(QModelIndex modelIndex);

signals:
    void boneDidSelect(vpvl::Bone *bone);

private:
    QTableView *m_tableView;
    TableModel *m_tableModel;
    vpvl::PMDModel *m_selectedModel;
};

#endif // TIMLINEWIDGET_H

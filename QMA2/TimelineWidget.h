#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QtGui/QWidget>

namespace vpvl { class PMDModel; }

class QTableView;
class TableModel;

class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineWidget(QWidget *parent = 0);
    ~TimelineWidget();

public slots:
    void setModel(vpvl::PMDModel *model);

private:
    QTableView *m_tableView;
    TableModel *m_tableModel;
    vpvl::PMDModel *m_selectedModel;
};

#endif // TIMLINEWIDGET_H

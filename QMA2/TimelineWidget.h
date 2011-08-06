#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QtCore/QModelIndex>
#include <QtGui/QWidget>

namespace vpvl {
class Bone;
class Face;
class PMDModel;
class VMDMotion;
class VPDPose;
}

namespace internal {
class TimelineTableModel;
}

class MotionBaseModel;

class QTableView;
class QSettings;

class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineWidget(MotionBaseModel *base,
                            QWidget *parent = 0);
    ~TimelineWidget();

public slots:
    void setCurrentIndex(const QModelIndex index);

signals:
    void motionDidSeek(float column);

private:
    const QModelIndex selectedIndex() const;

    QSettings *m_settings;
    QTableView *m_tableView;
};

#endif // TIMLINEWIDGET_H

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

class MotionBaseModel;

class QTableView;
class QSettings;
class QSpinBox;

class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineWidget(MotionBaseModel *base,
                            QWidget *parent = 0);
    ~TimelineWidget();

    const QModelIndex selectedIndex() const;
    QTableView *tableView() const { return m_tableView; }

public slots:
    void setCurrentIndex(const QModelIndex index);

signals:
    void motionDidSeek(float column);

private:
    QSettings *m_settings;
    QSpinBox *m_spinBox;
    QTableView *m_tableView;
};

#endif // TIMLINEWIDGET_H

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QtCore/QModelIndex>
#include <QtGui/QTreeView>
#include <QtGui/QWidget>

namespace vpvl {
class Bone;
class Face;
class PMDModel;
class VMDMotion;
class VPDPose;
}

class MotionBaseModel;

class QLabel;
class QTreeView;
class QSettings;
class QSpinBox;

class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineWidget(MotionBaseModel *base,
                            QWidget *parent = 0);
    ~TimelineWidget();

    int frameIndex() const;
    QTreeView *tableView() const { return m_treeView; }

public slots:
    void retranslate();
    void setCurrentColumnIndex(const QModelIndex &index);
    void setCurrentRowIndex(const QModelIndex &index);
    void setFrameIndex(int frameIndex);

signals:
    void motionDidSeek(float column);

private:
    QLabel *m_label;
    QSettings *m_settings;
    QSpinBox *m_spinBox;
    QTreeView *m_treeView;
    QModelIndex m_index;
};

#endif // TIMLINEWIDGET_H

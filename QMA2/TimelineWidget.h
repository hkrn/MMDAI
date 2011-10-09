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

class TimelineTreeView : public QTreeView
{
    Q_OBJECT

public:
    TimelineTreeView(QWidget *parent = 0);

public slots:
    void selectFrameIndex(int frameIndex);
};

class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineWidget(MotionBaseModel *base,
                            QWidget *parent = 0);
    ~TimelineWidget();

    const QModelIndex selectedIndex() const;
    QTreeView *tableView() const { return m_treeView; }

public slots:
    void retranslate();
    void setCurrentIndex(const QModelIndex index);

signals:
    void motionDidSeek(float column);

private slots:
    void setFrameIndex(int frameIndex);

private:
    QLabel *m_label;
    QSettings *m_settings;
    QSpinBox *m_spinBox;
    QTreeView *m_treeView;
};

#endif // TIMLINEWIDGET_H

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

class QTableView;
class QSettings;

class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineWidget(QSettings *settings, QWidget *parent = 0);
    ~TimelineWidget();

public slots:
    void registerBone(vpvl::Bone *bone);
    void registerFace(vpvl::Face *face);
    void setModel(vpvl::PMDModel *value);
    void selectCell(QModelIndex modelIndex);
    void setMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void setPose(vpvl::VPDPose *pose, vpvl::PMDModel *model);

signals:
    void boneDidSelect(vpvl::Bone *bone);
    void faceDidSelect(vpvl::Face *face);
    void frameIndexSeeked(float frameIndex);

protected:
    void closeEvent(QCloseEvent *event);

private:
    QSettings *m_settings;
    QTableView *m_tableView;
    internal::TimelineTableModel *m_tableModel;
    vpvl::PMDModel *m_selectedModel;
};

#endif // TIMLINEWIDGET_H

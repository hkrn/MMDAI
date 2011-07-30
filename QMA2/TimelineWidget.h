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

class BoneMotionModel;
class FaceMotionModel;

class QTableView;
class QSettings;

class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineWidget(QSettings *settings,
                            BoneMotionModel *bmm,
                            FaceMotionModel *fmm,
                            QWidget *parent = 0);
    ~TimelineWidget();

    BoneMotionModel *boneMotionModel() const { return m_boneMotionModel; }
    FaceMotionModel *faceMotionModel() const { return m_faceMotionModel; }

public slots:
    void loadPose(vpvl::VPDPose *pose, vpvl::PMDModel *model);
    void registerKeyFrame(vpvl::Bone *bone);
    void registerKeyFrame(vpvl::Face *face);

protected:
    void closeEvent(QCloseEvent *event);

private:
    const QModelIndex selectedIndex() const;

    QSettings *m_settings;
    QTableView *m_tableView;
    BoneMotionModel *m_boneMotionModel;
    FaceMotionModel *m_faceMotionModel;
};

#endif // TIMLINEWIDGET_H

#ifndef TIMELINETABWIDGET_H
#define TIMELINETABWIDGET_H

#include <QWidget>

namespace vpvl {
class Bone;
class Face;
class PMDModel;
class VPDPose;
}

class QSettings;
class TimelineWidget;
class BoneMotionModel;
class FaceMotionModel;
class VPDFile;

class TimelineTabWidget : public QWidget
{
    Q_OBJECT

public:
    static const QString kBone;
    static const QString kCamera;
    static const QString kFace;

    explicit TimelineTabWidget(QSettings *settings,
                               BoneMotionModel *bmm,
                               FaceMotionModel *fmm,
                               QWidget *parent = 0);
    ~TimelineTabWidget();

public slots:
    void loadPose(VPDFile *pose, vpvl::PMDModel *model);
    void registerKeyFrame(vpvl::Bone *bone);
    void registerKeyFrame(vpvl::Face *face);

signals:
    void motionDidSeek(float frameIndex);
    void currentTabDidChange(const QString &name);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void setCurrentTabIndex(int index);

private:
    QSettings *m_settings;
    TimelineWidget *m_boneTimeline;
    TimelineWidget *m_faceTimeline;
};

#endif // TIMELINETABWIDGET_H

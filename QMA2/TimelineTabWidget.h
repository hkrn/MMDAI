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
class QTabWidget;
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
    void savePose(VPDFile *pose, vpvl::PMDModel *model);
    void setFrameAtCurrentIndex(vpvl::Bone *bone);
    void setFrameAtCurrentIndex(vpvl::Face *face);
    void insertFrame();
    void deleteFrame();

signals:
    void motionDidSeek(float frameIndex);
    void currentTabDidChange(const QString &name);

private slots:
    void setCurrentTabIndex(int index);

private:
    QSettings *m_settings;
    QTabWidget *m_tabWidget;
    TimelineWidget *m_boneTimeline;
    TimelineWidget *m_faceTimeline;
};

#endif // TIMELINETABWIDGET_H

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
    enum Type {
        kBone,
        kFace,
        kCamera
    };

    explicit TimelineTabWidget(QSettings *settings,
                               BoneMotionModel *bmm,
                               FaceMotionModel *fmm,
                               QWidget *parent = 0);
    ~TimelineTabWidget();

public slots:
    void loadPose(VPDFile *pose, vpvl::PMDModel *model);
    void savePose(VPDFile *pose, vpvl::PMDModel *model);

signals:
    void motionDidSeek(float frameIndex);
    void currentTabDidChange(Type type);

private slots:
    void retranslate();
    void setCurrentFrameIndexZero();
    void insertFrame();
    void deleteFrame();
    void copyFrame();
    void pasteFrame();
    void setCurrentTabIndex(int index);

private:
    QSettings *m_settings;
    QTabWidget *m_tabWidget;
    TimelineWidget *m_boneTimeline;
    TimelineWidget *m_faceTimeline;
};

#endif // TIMELINETABWIDGET_H

#ifndef TIMELINETABWIDGET_H
#define TIMELINETABWIDGET_H

#include <QWidget>

namespace vpvl {
class PMDModel;
}

class QSettings;
class BoneMotionModel;
class FaceMotionModel;

class TimelineTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineTabWidget(QSettings *settings,
                               BoneMotionModel *bmm,
                               FaceMotionModel *fmm,
                               QWidget *parent = 0);
    ~TimelineTabWidget();

#if 0
public slots:
    void loadPose(vpvl::VPDPose *pose, vpvl::PMDModel *model);
    void registerKeyFrame(vpvl::Bone *bone);
    void registerKeyFrame(vpvl::Face *face);
    void selectColumn(QModelIndex current, QModelIndex previous);
#endif

signals:
    void motionDidSeek(float frameIndex);

protected:
    void closeEvent(QCloseEvent *event);

private:
    QSettings *m_settings;
    BoneMotionModel *m_boneMotionModel;
    FaceMotionModel *m_faceMotionModel;
};

#endif // TIMELINETABWIDGET_H

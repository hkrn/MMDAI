#include "TimelineTabWidget.h"
#include "BoneMotionModel.h"
#include "FaceMotionModel.h"
#include "TimelineWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

TimelineTabWidget::TimelineTabWidget(QSettings *settings,
                                     BoneMotionModel *bmm,
                                     FaceMotionModel *fmm,
                                     QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_boneMotionModel(bmm),
    m_faceMotionModel(fmm)
{
    QTabWidget *tabWidget = new QTabWidget();
    TimelineWidget *boneTimeline = new TimelineWidget(m_boneMotionModel, this);
    tabWidget->addTab(boneTimeline, tr("Bone"));
    TimelineWidget *faceTimeline = new TimelineWidget(m_faceMotionModel, this);
    tabWidget->addTab(faceTimeline, tr("Face"));
    connect(boneTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    connect(faceTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(tabWidget);
    setLayout(layout);
    restoreGeometry(m_settings->value("timelineTabWidget/geometry").toByteArray());
}

TimelineTabWidget::~TimelineTabWidget()
{
}

#if 0
void TimelineWidget::loadPose(vpvl::VPDPose *pose, vpvl::PMDModel *model)
{
    QModelIndex index = selectedIndex();
    if (index.isValid())
        m_boneMotionModel->loadPose(pose, model, index.column());
}

void TimelineWidget::registerKeyFrame(vpvl::Bone *bone)
{
    QModelIndex index = selectedIndex();
    if (index.isValid())
        m_boneMotionModel->registerKeyFrame(bone, index.column());
}

void TimelineWidget::registerKeyFrame(vpvl::Face *face)
{
    QModelIndex index = selectedIndex();
    if (index.isValid())
        m_faceMotionModel->registerKeyFrame(face, index.column());
}

void TimelineWidget::selectColumn(QModelIndex current, QModelIndex /* previous */)
{
    emit motionDidSeek(static_cast<float>(current.column()));
}
#endif

void TimelineTabWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("timelineTabWidget/geometry", saveGeometry());
    event->accept();
}

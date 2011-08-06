#include "TimelineTabWidget.h"
#include "BoneMotionModel.h"
#include "FaceMotionModel.h"
#include "TimelineWidget.h"
#include "ui_TimelineTabWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

TimelineTabWidget::TimelineTabWidget(QSettings *settings,
                                     BoneMotionModel *bmm,
                                     FaceMotionModel *fmm,
                                     QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimelineTabWidget),
    m_settings(settings),
    m_boneMotionModel(bmm),
    m_faceMotionModel(fmm)
{
    ui->setupUi(this);
    TimelineWidget *boneTimeline = new TimelineWidget(m_boneMotionModel, this);
    ui->tabWidget->addTab(boneTimeline, tr("Bone"));
    TimelineWidget *faceTimeline = new TimelineWidget(m_faceMotionModel, this);
    ui->tabWidget->addTab(faceTimeline, tr("Face"));
    connect(boneTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    connect(faceTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    restoreGeometry(m_settings->value("timelineTabWidget/geometry").toByteArray());
}

TimelineTabWidget::~TimelineTabWidget()
{
    delete ui;
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

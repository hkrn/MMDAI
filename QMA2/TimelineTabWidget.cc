#include "TimelineTabWidget.h"
#include "BoneMotionModel.h"
#include "FaceMotionModel.h"
#include "TimelineWidget.h"
#include "VPDFile.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

const QString TimelineTabWidget::kBone = QT_TR_NOOP_UTF8("Bone");
const QString TimelineTabWidget::kCamera = QT_TR_NOOP_UTF8("Camera");
const QString TimelineTabWidget::kFace = QT_TR_NOOP_UTF8("Face");

TimelineTabWidget::TimelineTabWidget(QSettings *settings,
                                     BoneMotionModel *bmm,
                                     FaceMotionModel *fmm,
                                     QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_boneTimeline(0),
    m_faceTimeline(0)
{
    QTabWidget *tabWidget = new QTabWidget();
    m_boneTimeline = new TimelineWidget(bmm, this);
    tabWidget->addTab(m_boneTimeline, kBone);
    m_faceTimeline = new TimelineWidget(fmm, this);
    tabWidget->addTab(m_faceTimeline, kFace);
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setCurrentTabIndex(int)));
    connect(m_boneTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    connect(m_faceTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(tabWidget);
    setLayout(layout);
    restoreGeometry(m_settings->value("timelineTabWidget/geometry").toByteArray());
    setWindowTitle(tr("Motion Timeline"));
}

TimelineTabWidget::~TimelineTabWidget()
{
}

void TimelineTabWidget::loadPose(VPDFile *pose, vpvl::PMDModel *model)
{
    QModelIndex index = m_boneTimeline->selectedIndex();
    if (index.isValid())
        reinterpret_cast<BoneMotionModel *>(m_boneTimeline->tableView()->model())->loadPose(pose, model, index.column());
}

void TimelineTabWidget::savePose(VPDFile *pose, vpvl::PMDModel *model)
{
    QModelIndex index = m_boneTimeline->selectedIndex();
    if (index.isValid())
        reinterpret_cast<BoneMotionModel *>(m_boneTimeline->tableView()->model())->savePose(pose, model, index.column());
}


void TimelineTabWidget::registerKeyFrame(vpvl::Bone *bone)
{
    QModelIndex index = m_boneTimeline->selectedIndex();
    if (index.isValid())
        reinterpret_cast<BoneMotionModel *>(m_boneTimeline->tableView()->model())->registerKeyFrame(bone, index.column());
}

void TimelineTabWidget::registerKeyFrame(vpvl::Face *face)
{
    QModelIndex index = m_faceTimeline->selectedIndex();
    if (index.isValid())
        reinterpret_cast<FaceMotionModel *>(m_faceTimeline->tableView()->model())->registerKeyFrame(face, index.column());
}

void TimelineTabWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("timelineTabWidget/geometry", saveGeometry());
    event->accept();
}

void TimelineTabWidget::setCurrentTabIndex(int index)
{
    switch (index) {
    case 0:
        emit currentTabDidChange(kBone);
        break;
    case 1:
        emit currentTabDidChange(kFace);
        break;
    }
}

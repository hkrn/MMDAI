#include "TimelineTabWidget.h"
#include "BoneMotionModel.h"
#include "FaceMotionModel.h"
#include "TimelineWidget.h"
#include "VPDFile.h"
#include "util.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

namespace {

static const int kBoneTabIndex = 0;
static const int kFaceTabIndex = 1;

static BoneMotionModel *UIGetBoneModel(TimelineWidget *timeline)
{
    return static_cast<BoneMotionModel *>(timeline->tableView()->model());
}

static FaceMotionModel *UIGetFaceModel(TimelineWidget *timeline)
{
    return static_cast<FaceMotionModel *>(timeline->tableView()->model());
}

static void UIModelDeleteFrame(TimelineWidget *timeline)
{
    MotionBaseModel *model = static_cast<MotionBaseModel *>(timeline->tableView()->model());
    QModelIndexList indices = timeline->tableView()->selectionModel()->selectedIndexes();
    foreach (QModelIndex index, indices)
        model->deleteFrame(index);
}

static void UIModelInsertBoneFrame(TimelineWidget *timeline)
{
    BoneMotionModel *model = UIGetBoneModel(timeline);
    QModelIndexList indices = timeline->tableView()->selectionModel()->selectedIndexes();
    QTextCodec *codec = internal::getTextCodec();
    foreach (QModelIndex index, indices) {
        vpvl::Bone bone;
        QString name = model->data(index, Qt::DisplayRole).toString();
        bone.setName(reinterpret_cast<const uint8_t *>(codec->fromUnicode(name).constData()));
        // FIXME: should use constant value
        bone.setPosition(btVector3(0, 0, 0));
        bone.setRotation(btQuaternion(0, 0, 0, 1));
        model->registerKeyFrame(&bone, index.column());
    }
}

static void UIModelInsertFaceFrame(TimelineWidget *timeline)
{
    FaceMotionModel *model = UIGetFaceModel(timeline);
    QModelIndexList indices = timeline->tableView()->selectionModel()->selectedIndexes();
    QTextCodec *codec = internal::getTextCodec();
    foreach (QModelIndex index, indices) {
        vpvl::Face face;
        QString name = model->data(index, Qt::DisplayRole).toString();
        face.setName(reinterpret_cast<const uint8_t *>(codec->fromUnicode(name).constData()));
        face.setWeight(0);
        model->registerKeyFrame(&face, index.column());
    }
}

}

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
    m_tabWidget = new QTabWidget();
    m_boneTimeline = new TimelineWidget(bmm, this);
    m_tabWidget->insertTab(kBoneTabIndex, m_boneTimeline, kBone);
    m_faceTimeline = new TimelineWidget(fmm, this);
    m_tabWidget->insertTab(kFaceTabIndex, m_faceTimeline, kFace);
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setCurrentTabIndex(int)));
    connect(m_boneTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    connect(m_faceTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(m_tabWidget);
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
        UIGetBoneModel(m_boneTimeline)->loadPose(pose, model, index.column());
}

void TimelineTabWidget::savePose(VPDFile *pose, vpvl::PMDModel *model)
{
    QModelIndex index = m_boneTimeline->selectedIndex();
    if (index.isValid())
        UIGetBoneModel(m_boneTimeline)->savePose(pose, model, index.column());
}


void TimelineTabWidget::registerKeyFrame(vpvl::Bone *bone)
{
    QModelIndex index = m_boneTimeline->selectedIndex();
    if (index.isValid())
        UIGetBoneModel(m_boneTimeline)->registerKeyFrame(bone, index.column());
}

void TimelineTabWidget::registerKeyFrame(vpvl::Face *face)
{
    QModelIndex index = m_faceTimeline->selectedIndex();
    if (index.isValid())
        UIGetFaceModel(m_faceTimeline)->registerKeyFrame(face, index.column());
}

void TimelineTabWidget::insertFrame()
{
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
        UIModelInsertBoneFrame(m_boneTimeline);
        break;
    case kFaceTabIndex:
        UIModelInsertFaceFrame(m_faceTimeline);
        break;
    }
}

void TimelineTabWidget::deleteFrame()
{
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
        UIModelDeleteFrame(m_boneTimeline);
        break;
    case kFaceTabIndex:
        UIModelDeleteFrame(m_faceTimeline);
        break;
    }
}

void TimelineTabWidget::setCurrentTabIndex(int index)
{
    switch (index) {
    case kBoneTabIndex:
        emit currentTabDidChange(kBone);
        break;
    case kFaceTabIndex:
        emit currentTabDidChange(kFace);
        break;
    }
}

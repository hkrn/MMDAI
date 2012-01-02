/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "common/VPDFile.h"
#include "common/util.h"
#include "models/BoneMotionModel.h"
#include "models/FaceMotionModel.h"
#include "models/SceneMotionModel.h"
#include "widgets/TimelineTabWidget.h"
#include "widgets/TimelineWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

namespace
{

static const int kSceneTabIndex = 0;
static const int kBoneTabIndex = 1;
static const int kFaceTabIndex = 2;

static BoneMotionModel *UIGetBoneModel(TimelineWidget *timeline)
{
    return qobject_cast<BoneMotionModel *>(timeline->treeView()->model());
}

static FaceMotionModel *UIGetFaceModel(TimelineWidget *timeline)
{
    return qobject_cast<FaceMotionModel *>(timeline->treeView()->model());
}

static SceneMotionModel *UIGetSceneModel(TimelineWidget *timeline)
{
    return qobject_cast<SceneMotionModel *>(timeline->treeView()->model());
}

static QItemSelectionModel *UIGetSelectionModel(TimelineWidget *timeline)
{
    return qobject_cast<QItemSelectionModel *>(timeline->treeView()->selectionModel());
}

static void UIModelInsertBoneFrame(TimelineWidget *timeline)
{
    BoneMotionModel *model = UIGetBoneModel(timeline);
    const QModelIndexList &indices = timeline->treeView()->selectionModel()->selectedIndexes();
    BoneMotionModel::KeyFramePairList boneFrames;
    foreach (const QModelIndex &index, indices) {
        vpvl::BoneKeyframe *frame = new vpvl::BoneKeyframe();
        QByteArray name = model->nameFromModelIndex(index);
        int frameIndex = MotionBaseModel::toFrameIndex(index);
        frame->setName(reinterpret_cast<const uint8_t *>(name.constData()));
        frame->setDefaultInterpolationParameter();
        boneFrames.append(BoneMotionModel::KeyFramePair(frameIndex, BoneMotionModel::KeyFramePtr(frame)));
    }
    model->setFrames(boneFrames);
}

static void UIModelInsertFaceFrame(TimelineWidget *timeline)
{
    FaceMotionModel *model = UIGetFaceModel(timeline);
    const QModelIndexList &indices = timeline->treeView()->selectionModel()->selectedIndexes();
    FaceMotionModel::KeyFramePairList faceFrames;
    foreach (const QModelIndex &index, indices) {
        vpvl::FaceKeyframe *frame = new vpvl::FaceKeyframe();
        QByteArray name = model->nameFromModelIndex(index);
        int frameIndex = MotionBaseModel::toFrameIndex(index);
        frame->setName(reinterpret_cast<const uint8_t *>(name.constData()));
        frame->setWeight(0);
        faceFrames.append(FaceMotionModel::KeyFramePair(frameIndex, FaceMotionModel::KeyFramePtr(frame)));
    }
    model->setFrames(faceFrames);
}

}

TimelineTabWidget::TimelineTabWidget(QSettings *settings,
                                     BoneMotionModel *bmm,
                                     FaceMotionModel *fmm,
                                     SceneMotionModel *smm,
                                     QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_boneTimeline(0),
    m_faceTimeline(0),
    m_sceneTimeline(0)
{
    m_tabWidget = new QTabWidget();
    m_boneTimeline = new TimelineWidget(bmm, this);
    m_tabWidget->insertTab(kBoneTabIndex, m_boneTimeline, "");
    m_faceTimeline = new TimelineWidget(fmm, this);
    m_tabWidget->insertTab(kFaceTabIndex, m_faceTimeline, "");
    m_sceneTimeline = new TimelineWidget(smm, this);
    m_tabWidget->insertTab(kSceneTabIndex, m_sceneTimeline, "");
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setCurrentTabIndex(int)));
    connect(m_boneTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    connect(m_faceTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    connect(m_sceneTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    connect(m_boneTimeline->button(), SIGNAL(clicked()), this, SLOT(addBoneKeyFramesFromSelectedIndices()));
    connect(m_faceTimeline->button(), SIGNAL(clicked()), this, SLOT(addFaceKeyFramesFromSelectedIndices()));
    connect(m_sceneTimeline->button(), SIGNAL(clicked()), this, SLOT(addSceneKeyFramesFromSelectedIndices()));
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(m_tabWidget);
    retranslate();
    setLayout(layout);
    restoreGeometry(m_settings->value("timelineTabWidget/geometry").toByteArray());
}

TimelineTabWidget::~TimelineTabWidget()
{
}

void TimelineTabWidget::addKeyFramesFromSelectedIndices()
{
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
        addBoneKeyFramesFromSelectedIndices();
        break;
    case kFaceTabIndex:
        addFaceKeyFramesFromSelectedIndices();
        break;
    }
}

void TimelineTabWidget::loadPose(VPDFile *pose, vpvl::PMDModel *model)
{
    UIGetBoneModel(m_boneTimeline)->loadPose(pose, model, m_boneTimeline->frameIndex());
}

void TimelineTabWidget::retranslate()
{
    m_tabWidget->setTabText(kBoneTabIndex, tr("Bone"));
    m_tabWidget->setTabText(kFaceTabIndex, tr("Face"));
    m_tabWidget->setTabText(kSceneTabIndex, tr("Scene"));
    setWindowTitle(tr("Motion Timeline"));
}

void TimelineTabWidget::addBoneKeyFramesFromSelectedIndices()
{
    BoneMotionModel *model = UIGetBoneModel(m_boneTimeline);
    QItemSelectionModel *selection = UIGetSelectionModel(m_boneTimeline);
    model->addKeyFramesByModelIndices(selection->selectedIndexes());
}

void TimelineTabWidget::addFaceKeyFramesFromSelectedIndices()
{
    FaceMotionModel *model = UIGetFaceModel(m_faceTimeline);
    QItemSelectionModel *selection = UIGetSelectionModel(m_faceTimeline);
    model->addKeyFramesByModelIndices(selection->selectedIndexes());
}

void TimelineTabWidget::addSceneKeyFramesFromSelectedIndices()
{
    SceneMotionModel *model = UIGetSceneModel(m_sceneTimeline);
    QItemSelectionModel *selection = UIGetSelectionModel(m_sceneTimeline);
    model->addKeyFramesByModelIndices(selection->selectedIndexes());
}

void TimelineTabWidget::savePose(VPDFile *pose, vpvl::PMDModel *model)
{
    UIGetBoneModel(m_boneTimeline)->savePose(pose, model, m_boneTimeline->frameIndex());
}

void TimelineTabWidget::addBoneKeyFrameAtCurrentFrameIndex(vpvl::Bone *bone)
{
    if (bone) {
        BoneMotionModel::KeyFramePairList boneFrames;
        vpvl::BoneKeyframe *frame = new vpvl::BoneKeyframe();
        frame->setDefaultInterpolationParameter();
        frame->setName(bone->name());
        frame->setPosition(bone->position());
        frame->setRotation(bone->rotation());
        boneFrames.append(BoneMotionModel::KeyFramePair(m_boneTimeline->frameIndex(), BoneMotionModel::KeyFramePtr(frame)));
        UIGetBoneModel(m_boneTimeline)->setFrames(boneFrames);
    }
}

void TimelineTabWidget::addFaceKeyFrameAtCurrentFrameIndex(vpvl::Face *face)
{
    if (face) {
        FaceMotionModel::KeyFramePairList faceFrames;
        vpvl::FaceKeyframe *frame = new vpvl::FaceKeyframe();
        frame->setName(face->name());
        frame->setWeight(face->weight());
        faceFrames.append(FaceMotionModel::KeyFramePair(m_faceTimeline->frameIndex(), FaceMotionModel::KeyFramePtr(frame)));
        UIGetFaceModel(m_faceTimeline)->setFrames(faceFrames);
    }
}

void TimelineTabWidget::setCurrentFrameIndexZero()
{
    m_boneTimeline->setCurrentFrameIndex(0);
    m_faceTimeline->setCurrentFrameIndex(0);
    m_sceneTimeline->setCurrentFrameIndex(0);
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
    if (TimelineWidget *timeline = currentSelectedTimelineWidget()) {
        TimelineTreeView *view = timeline->treeView();
        MotionBaseModel *model = qobject_cast<MotionBaseModel *>(view->model());
        const QModelIndexList &indices = view->selectionModel()->selectedIndexes();
        foreach (const QModelIndex &index, indices) {
            if (index.column() > 1)
                model->deleteFrameByModelIndex(index);
        }
    }
}


void TimelineTabWidget::copyFrame()
{
    TimelineWidget *timeline = currentSelectedTimelineWidget();
    MotionBaseModel *model = currentSelectedModel();
    if (timeline && model)
        model->copyFrames(timeline->frameIndex());
}

void TimelineTabWidget::pasteFrame()
{
    TimelineWidget *timeline = currentSelectedTimelineWidget();
    MotionBaseModel *model = currentSelectedModel();
    if (timeline && model)
        model->pasteFrames(timeline->frameIndex());
}

void TimelineTabWidget::pasteReversedFrame()
{
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
        UIGetBoneModel(m_boneTimeline)->pasteReversedFrame(m_boneTimeline->frameIndex());
        break;
    default:
        pasteFrame();
    }
}

void TimelineTabWidget::nextFrame()
{
    seekFrameIndexFromCurrentFrameIndex(1);
}

void TimelineTabWidget::previousFrame()
{
    seekFrameIndexFromCurrentFrameIndex(-1);
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
    case kSceneTabIndex:
        emit currentTabDidChange(kScene);
        break;
    }
}

void TimelineTabWidget::notifyCurrentTabIndex()
{
    setCurrentTabIndex(m_tabWidget->currentIndex());
}

void TimelineTabWidget::selectFrameIndices(int fromIndex, int toIndex)
{
    if (TimelineWidget *widget = currentSelectedTimelineWidget()) {
        if (fromIndex > toIndex)
            qSwap(fromIndex, toIndex);
        QList<int> frameIndices;
        for (int i = fromIndex; i <= toIndex; i++)
            frameIndices.append(i);
        widget->treeView()->selectFrameIndices(frameIndices, true);
    }
}

void TimelineTabWidget::seekFrameIndexFromCurrentFrameIndex(int frameIndex)
{
    if (TimelineWidget *timeline = currentSelectedTimelineWidget())
        timeline->setCurrentFrameIndex(timeline->frameIndex() + frameIndex);
}

TimelineWidget *TimelineTabWidget::currentSelectedTimelineWidget() const
{
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
        return m_boneTimeline;
    case kFaceTabIndex:
        return m_faceTimeline;
    case kSceneTabIndex:
        return m_sceneTimeline;
    default:
        return 0;
    }
}

MotionBaseModel *TimelineTabWidget::currentSelectedModel() const
{
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
        return static_cast<MotionBaseModel *>(m_boneTimeline->treeView()->model());
    case kFaceTabIndex:
        return static_cast<MotionBaseModel *>(m_faceTimeline->treeView()->model());
    case kSceneTabIndex:
        return static_cast<MotionBaseModel *>(m_sceneTimeline->treeView()->model());
    default:
        return 0;
    }
}

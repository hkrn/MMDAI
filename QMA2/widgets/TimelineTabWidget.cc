/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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
#include "dialogs/FrameSelectionDialog.h"
#include "dialogs/FrameWeightDialog.h"
#include "models/BoneMotionModel.h"
#include "models/FaceMotionModel.h"
#include "models/SceneMotionModel.h"
#include "widgets/TimelineTabWidget.h"
#include "widgets/TimelineTreeView.h"
#include "widgets/TimelineWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

using namespace vpvl;

TimelineTabWidget::TimelineTabWidget(QSettings *settings,
                                     BoneMotionModel *bmm,
                                     FaceMotionModel *fmm,
                                     SceneMotionModel *smm,
                                     QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_boneTimeline(0),
    m_faceTimeline(0),
    m_sceneTimeline(0),
    m_frameSelectionDialog(0),
    m_frameWeightDialog(0)
{
    m_tabWidget = new QTabWidget();
    m_boneTimeline = new TimelineWidget(bmm, this);
    m_boneTimeline->setEnableFrameIndexSpinBox(false);
    m_boneSelectButton = new QRadioButton();
    m_boneSelectButton->setChecked(true);
    m_boneSelectButton->setEnabled(false);
    m_boneRotateButton = new QRadioButton();
    m_boneRotateButton->setEnabled(false);
    m_boneMoveButton = new QRadioButton();
    m_boneMoveButton->setEnabled(false);
    m_boneButtonGroup = new QButtonGroup();
    connect(m_boneButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(selectButton(QAbstractButton*)));
    m_boneButtonGroup->addButton(m_boneSelectButton);
    m_boneButtonGroup->addButton(m_boneRotateButton);
    m_boneButtonGroup->addButton(m_boneMoveButton);
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addWidget(m_boneSelectButton);
    mainLayout->addWidget(m_boneRotateButton);
    mainLayout->addWidget(m_boneMoveButton);
    mainLayout->setAlignment(Qt::AlignCenter);
    /* hack bone timeline layout */
    reinterpret_cast<QVBoxLayout *>(m_boneTimeline->layout())->addLayout(mainLayout);
    m_tabWidget->insertTab(kBoneTabIndex, m_boneTimeline, "");
    m_faceTimeline = new TimelineWidget(fmm, this);
    m_faceTimeline->setEnableFrameIndexSpinBox(false);
    m_tabWidget->insertTab(kFaceTabIndex, m_faceTimeline, "");
    m_sceneTimeline = new TimelineWidget(smm, this);
    m_tabWidget->insertTab(kSceneTabIndex, m_sceneTimeline, "");
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setCurrentTabIndex(int)));
    connect(m_boneTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    connect(m_faceTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    connect(m_sceneTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    connect(bmm, SIGNAL(modelDidChange(vpvl::PMDModel*)), this, SLOT(toggleBoneEnable(vpvl::PMDModel*)));
    connect(bmm, SIGNAL(bonesDidSelect(QList<vpvl::Bone*>)), this, SLOT(toggleBoneButtonsByBone(QList<vpvl::Bone*>)));
    connect(fmm, SIGNAL(modelDidChange(vpvl::PMDModel*)), this, SLOT(toggleFaceEnable(vpvl::PMDModel*)));
    QVBoxLayout *layout = new QVBoxLayout();
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
    currentSelectedTimelineWidget()->treeView()->addKeyframesBySelectedIndices();
}

void TimelineTabWidget::loadPose(VPDFile *pose, PMDModel *model)
{
    BoneMotionModel *m = static_cast<BoneMotionModel *>(m_boneTimeline->treeView()->model());
    m->loadPose(pose, model, m_boneTimeline->frameIndex());
}

void TimelineTabWidget::retranslate()
{
    m_boneSelectButton->setText(tr("Select"));
    m_boneRotateButton->setText(tr("Rotate"));
    m_boneMoveButton->setText(tr("Move"));
    m_tabWidget->setTabText(kBoneTabIndex, tr("Bone"));
    m_tabWidget->setTabText(kFaceTabIndex, tr("Face"));
    m_tabWidget->setTabText(kSceneTabIndex, tr("Scene"));
    setWindowTitle(tr("Motion Timeline"));
}

void TimelineTabWidget::savePose(VPDFile *pose, PMDModel *model)
{
    BoneMotionModel *m = static_cast<BoneMotionModel *>(m_boneTimeline->treeView()->model());
    m->savePose(pose, model, m_boneTimeline->frameIndex());
}

void TimelineTabWidget::addBoneKeyFrameAtCurrentFrameIndex(Bone *bone)
{
    /*
     * 渡されたボーンの名前と位置と回転情報を元に新しいボーンのキーフレームとして登録する処理
     * (BoneKeyframe#setFrameIndex は KeyFramePair の第一引数を元に SetFramesCommand で行ってる)
     */
    if (bone) {
        BoneMotionModel::KeyFramePairList keyframes;
        BoneKeyframe *keyframe = new BoneKeyframe();
        keyframe->setDefaultInterpolationParameter();
        keyframe->setName(bone->name());
        keyframe->setPosition(bone->position());
        keyframe->setRotation(bone->rotation());
        keyframes.append(BoneMotionModel::KeyFramePair(m_boneTimeline->frameIndex(), BoneMotionModel::KeyFramePtr(keyframe)));
        BoneMotionModel *model = static_cast<BoneMotionModel *>(m_boneTimeline->treeView()->model());
        model->setFrames(keyframes);
    }
}

void TimelineTabWidget::addFaceKeyFrameAtCurrentFrameIndex(Face *face)
{
    /*
     * 渡された頂点モーフの名前と重み係数を元に新しい頂点モーフのキーフレームとして登録する処理
     * (FaceKeyframe#setFrameIndex は KeyFramePair の第一引数を元に SetFramesCommand で行ってる)
     */
    if (face) {
        FaceMotionModel::KeyFramePairList keyframes;
        FaceKeyframe *keyframe = new FaceKeyframe();
        keyframe->setName(face->name());
        keyframe->setWeight(face->weight());
        keyframes.append(FaceMotionModel::KeyFramePair(m_faceTimeline->frameIndex(), FaceMotionModel::KeyFramePtr(keyframe)));
        FaceMotionModel *model = static_cast<FaceMotionModel *>(m_faceTimeline->treeView()->model());
        model->setFrames(keyframes);
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
    {
        TimelineTreeView *view = m_boneTimeline->treeView();
        BoneMotionModel *model = static_cast<BoneMotionModel *>(view->model());
        const QModelIndexList &indices = view->selectionModel()->selectedIndexes();
        BoneMotionModel::KeyFramePairList boneFrames;
        foreach (const QModelIndex &index, indices) {
            BoneKeyframe *frame = new BoneKeyframe();
            QByteArray name = model->nameFromModelIndex(index);
            int frameIndex = MotionBaseModel::toFrameIndex(index);
            frame->setName(reinterpret_cast<const uint8_t *>(name.constData()));
            frame->setDefaultInterpolationParameter();
            boneFrames.append(BoneMotionModel::KeyFramePair(frameIndex, BoneMotionModel::KeyFramePtr(frame)));
        }
        model->setFrames(boneFrames);
        break;
    }
    case kFaceTabIndex:
    {
        TimelineTreeView *view = m_faceTimeline->treeView();
        FaceMotionModel *model = static_cast<FaceMotionModel *>(view->model());
        const QModelIndexList &indices = view->selectionModel()->selectedIndexes();
        FaceMotionModel::KeyFramePairList faceFrames;
        foreach (const QModelIndex &index, indices) {
            FaceKeyframe *frame = new FaceKeyframe();
            QByteArray name = model->nameFromModelIndex(index);
            int frameIndex = MotionBaseModel::toFrameIndex(index);
            frame->setName(reinterpret_cast<const uint8_t *>(name.constData()));
            frame->setWeight(0);
            faceFrames.append(FaceMotionModel::KeyFramePair(frameIndex, FaceMotionModel::KeyFramePtr(frame)));
        }
        model->setFrames(faceFrames);
        break;
    }
    }
}

void TimelineTabWidget::deleteFrame()
{
    currentSelectedTimelineWidget()->treeView()->deleteKeyframesBySelectedIndices();
}

void TimelineTabWidget::copyFrame()
{
    TimelineWidget *timeline = currentSelectedTimelineWidget();
    timeline->treeView()->copyKeyframes(timeline->frameIndex());
}

void TimelineTabWidget::pasteFrame()
{
    TimelineWidget *timeline = currentSelectedTimelineWidget();
    timeline->treeView()->pasteKeyframes(timeline->frameIndex());
}

void TimelineTabWidget::pasteReversedFrame()
{
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
    {
        BoneMotionModel *model = static_cast<BoneMotionModel *>(m_boneTimeline->treeView()->model());
        model->pasteReversedFrame(m_boneTimeline->frameIndex());
        break;
    }
    default:
    {
        pasteFrame();
    }
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

void TimelineTabWidget::toggleBoneEnable(PMDModel *model)
{
    bool value = model ? true : false;
    m_boneTimeline->setEnableFrameIndexSpinBox(value);
    m_boneSelectButton->setChecked(true);
    m_boneSelectButton->setEnabled(value);
    m_boneRotateButton->setEnabled(false);
    m_boneMoveButton->setEnabled(false);
}

void TimelineTabWidget::toggleFaceEnable(PMDModel *model)
{
    m_faceTimeline->setEnableFrameIndexSpinBox(model ? true : false);
}

void TimelineTabWidget::toggleBoneButtonsByBone(const QList<Bone *> &bones)
{
    if (!bones.isEmpty()) {
        Bone *bone = bones.first();
        bool movable = bone->isMovable(), rotateable = bone->isRotateable();
        m_boneRotateButton->setCheckable(rotateable);
        m_boneRotateButton->setEnabled(rotateable);
        m_boneMoveButton->setCheckable(movable);
        m_boneMoveButton->setEnabled(movable);
    }
    else {
        m_boneRotateButton->setCheckable(false);
        m_boneRotateButton->setEnabled(false);
        m_boneMoveButton->setCheckable(false);
        m_boneMoveButton->setEnabled(false);
    }
}

void TimelineTabWidget::selectAllRegisteredKeyframes()
{
    MotionBaseModel *model = static_cast<MotionBaseModel *>(currentSelectedTimelineWidget()->treeView()->model());
    selectFrameIndices(0, model->maxFrameIndex());
}

void TimelineTabWidget::openFrameSelectionDialog()
{
    if (!m_frameSelectionDialog) {
        m_frameSelectionDialog = new FrameSelectionDialog(this);
        connect(m_frameSelectionDialog, SIGNAL(frameIndicesDidSelect(int,int)),
                this, SLOT(selectFrameIndices(int,int)));
    }
    MotionBaseModel *model = static_cast<MotionBaseModel *>(currentSelectedTimelineWidget()->treeView()->model());
    m_frameSelectionDialog->setMaxFrameIndex(model->maxFrameIndex());
    m_frameSelectionDialog->show();
}

void TimelineTabWidget::openFrameWeightDialog()
{
    if (!m_frameWeightDialog) {
        m_frameWeightDialog = new FrameWeightDialog(this);
        connect(m_frameWeightDialog, SIGNAL(keyframeWeightDidSet(float)),
                this, SLOT(setKeyframeWeight(float)));
    }
    m_frameWeightDialog->resetValue();
    m_frameWeightDialog->show();
}

void TimelineTabWidget::selectButton(QAbstractButton *button)
{
    if (button == m_boneSelectButton)
        emit editModeDidSet(SceneWidget::kSelect);
    else if (button == m_boneRotateButton)
        emit editModeDidSet(SceneWidget::kRotate);
    else if (button == m_boneMoveButton)
        emit editModeDidSet(SceneWidget::kMove);
}

void TimelineTabWidget::selectFrameIndices(int fromIndex, int toIndex)
{
    /* from と to の値が逆転していないかの検証 (本来は from <= to が正しい) */
    if (fromIndex > toIndex)
        qSwap(fromIndex, toIndex);
    QList<int> frameIndices;
    for (int i = fromIndex; i <= toIndex; i++)
        frameIndices.append(i);
    currentSelectedTimelineWidget()->treeView()->selectFrameIndices(frameIndices, true);
}

void TimelineTabWidget::setKeyframeWeight(float value)
{
    currentSelectedTimelineWidget()->treeView()->setKeyframeWeightBySelectedIndices(value);
}

void TimelineTabWidget::seekFrameIndexFromCurrentFrameIndex(int frameIndex)
{
    TimelineWidget *timeline = currentSelectedTimelineWidget();
    currentSelectedTimelineWidget()->setCurrentFrameIndex(timeline->frameIndex() + frameIndex);
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
        qFatal("Unexpected tab index value: %d", m_tabWidget->currentIndex());
        return 0;
    }
}

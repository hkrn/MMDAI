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
#include "models/MorphMotionModel.h"
#include "models/SceneMotionModel.h"
#include "widgets/InterpolationWidget.h"
#include "widgets/TimelineTabWidget.h"
#include "widgets/TimelineTreeView.h"
#include "widgets/TimelineWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

using namespace vpvl;

TimelineTabWidget::TimelineTabWidget(QSettings *settings,
                                     BoneMotionModel *bmm,
                                     MorphMotionModel *mmm,
                                     SceneMotionModel *smm,
                                     QWidget *parent) :
    QWidget(parent),
    m_settings(settings),
    m_boneTimeline(0),
    m_morphTimeline(0),
    m_sceneTimeline(0),
    m_interpolationWidget(0),
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
    m_morphTimeline = new TimelineWidget(mmm, this);
    m_morphTimeline->setEnableFrameIndexSpinBox(false);
    m_tabWidget->insertTab(kMorphTabIndex, m_morphTimeline, "");
    m_sceneTimeline = new TimelineWidget(smm, this);
    m_tabWidget->insertTab(kSceneTabIndex, m_sceneTimeline, "");
    m_interpolationWidget = new InterpolationWidget(bmm, smm);
    connect(this, SIGNAL(currentTabDidChange(int)), m_interpolationWidget, SLOT(setMode(int)));
    m_tabWidget->insertTab(kInterpolationTabIndex, m_interpolationWidget, "");
    connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setCurrentTabIndex(int)));
    /* シグナルチェーン (motionDidSeek) を発行し、モデル側のシグナルを TimelineTabWidget のシグナルとして一本化して取り扱う */
    connect(m_boneTimeline, SIGNAL(motionDidSeek(float)), SIGNAL(motionDidSeek(float)));
    connect(m_morphTimeline, SIGNAL(motionDidSeek(float)), SIGNAL(motionDidSeek(float)));
    connect(m_sceneTimeline, SIGNAL(motionDidSeek(float)), SIGNAL(motionDidSeek(float)));
    connect(bmm, SIGNAL(modelDidChange(vpvl::PMDModel*)), SLOT(toggleBoneEnable(vpvl::PMDModel*)));
    connect(bmm, SIGNAL(bonesDidSelect(QList<vpvl::Bone*>)), SLOT(toggleBoneButtonsByBone(QList<vpvl::Bone*>)));
    connect(mmm, SIGNAL(modelDidChange(vpvl::PMDModel*)), SLOT(toggleFaceEnable(vpvl::PMDModel*)));
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

void TimelineTabWidget::loadPose(VPDFilePtr pose, PMDModel *model)
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
    m_tabWidget->setTabText(kMorphTabIndex, tr("Morph"));
    m_tabWidget->setTabText(kSceneTabIndex, tr("Scene"));
    m_tabWidget->setTabText(kInterpolationTabIndex, tr("Interpolation"));
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
        MorphMotionModel::KeyFramePairList keyframes;
        FaceKeyframe *keyframe = new FaceKeyframe();
        keyframe->setName(face->name());
        keyframe->setWeight(face->weight());
        keyframes.append(MorphMotionModel::KeyFramePair(m_morphTimeline->frameIndex(), MorphMotionModel::KeyFramePtr(keyframe)));
        MorphMotionModel *model = static_cast<MorphMotionModel *>(m_morphTimeline->treeView()->model());
        model->setFrames(keyframes);
    }
}

void TimelineTabWidget::setCurrentFrameIndex(int value)
{
    /* 二重呼出になってしまうため、一時的に motionDidSeek シグナルを止める */
    disconnect(m_boneTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    disconnect(m_morphTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    disconnect(m_sceneTimeline, SIGNAL(motionDidSeek(float)), this, SIGNAL(motionDidSeek(float)));
    m_boneTimeline->setCurrentFrameIndex(value);
    m_morphTimeline->setCurrentFrameIndex(value);
    m_sceneTimeline->setCurrentFrameIndex(value);
    connect(m_boneTimeline, SIGNAL(motionDidSeek(float)), SIGNAL(motionDidSeek(float)));
    connect(m_morphTimeline, SIGNAL(motionDidSeek(float)), SIGNAL(motionDidSeek(float)));
    connect(m_sceneTimeline, SIGNAL(motionDidSeek(float)), SIGNAL(motionDidSeek(float)));
}

void TimelineTabWidget::setCurrentFrameIndexZero()
{
    setCurrentFrameIndex(0);
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
    case kMorphTabIndex:
    {
        TimelineTreeView *view = m_morphTimeline->treeView();
        MorphMotionModel *model = static_cast<MorphMotionModel *>(view->model());
        const QModelIndexList &indices = view->selectionModel()->selectedIndexes();
        MorphMotionModel::KeyFramePairList faceFrames;
        foreach (const QModelIndex &index, indices) {
            FaceKeyframe *frame = new FaceKeyframe();
            QByteArray name = model->nameFromModelIndex(index);
            int frameIndex = MotionBaseModel::toFrameIndex(index);
            frame->setName(reinterpret_cast<const uint8_t *>(name.constData()));
            frame->setWeight(0);
            faceFrames.append(MorphMotionModel::KeyFramePair(frameIndex, MorphMotionModel::KeyFramePtr(frame)));
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

void TimelineTabWidget::copyKeyframes()
{
    TimelineWidget *widget = currentSelectedTimelineWidget();
    TimelineTreeView *treeView = widget->treeView();
    MotionBaseModel *model = static_cast<MotionBaseModel *>(treeView->model());
    model->copyKeyframesByModelIndices(treeView->selectionModel()->selectedIndexes(), widget->frameIndex());
}

void TimelineTabWidget::cutKeyframes()
{
    TimelineWidget *widget = currentSelectedTimelineWidget();
    TimelineTreeView *treeView = widget->treeView();
    MotionBaseModel *model = static_cast<MotionBaseModel *>(treeView->model());
    model->cutKeyframesByModelIndices(treeView->selectionModel()->selectedIndexes(), widget->frameIndex());
}

void TimelineTabWidget::pasteKeyframes()
{
    TimelineWidget *widget = currentSelectedTimelineWidget();
    TimelineTreeView *treeView = widget->treeView();
    MotionBaseModel *model = static_cast<MotionBaseModel *>(treeView->model());
    model->pasteKeyframesByFrameIndex(widget->frameIndex());
}

void TimelineTabWidget::pasteKeyframesWithReverse()
{
    TimelineTreeView *treeView;
    BoneMotionModel *model;
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
        treeView = m_boneTimeline->treeView();
        model = static_cast<BoneMotionModel *>(treeView->model());
        model->pasteReversedFrame(m_boneTimeline->frameIndex());
        break;
    default:
        pasteKeyframes();
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
    case kMorphTabIndex:
        emit currentTabDidChange(kMorph);
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
    m_morphTimeline->setEnableFrameIndexSpinBox(model ? true : false);
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
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex: {
        FrameWeightDialog dialog(kBone);
        connect(&dialog, SIGNAL(boneWeightDidSet(vpvl::Vector3,vpvl::Vector3)),
                m_boneTimeline->treeView(), SLOT(setBoneKeyframesWeightBySelectedIndices(vpvl::Vector3,vpvl::Vector3)));
        dialog.exec();
        break;
    }
    case kMorphTabIndex: {
        FrameWeightDialog dialog(kMorph);
        connect(&dialog, SIGNAL(morphKeyframeWeightDidSet(float)),
                m_morphTimeline->treeView(), SLOT(setMorphKeyframesWeightBySelectedIndices(float)));
        dialog.exec();
        break;
    }
    default:
        QMessageBox::warning(this, tr("Not supported operation"),
                             tr("The timeline is not supported adjusting keyframe weight."));
        break;
    }
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
    case kMorphTabIndex:
        return m_morphTimeline;
    case kSceneTabIndex:
        return m_sceneTimeline;
    default:
        qFatal("Unexpected tab index value: %d", m_tabWidget->currentIndex());
        return 0;
    }
}

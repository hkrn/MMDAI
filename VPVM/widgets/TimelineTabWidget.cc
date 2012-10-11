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
#include "dialogs/InterpolationDialog.h"
#include "models/BoneMotionModel.h"
#include "models/MorphMotionModel.h"
#include "models/SceneMotionModel.h"
#include "widgets/TimelineTabWidget.h"
#include "widgets/TimelineTreeView.h"
#include "widgets/TimelineWidget.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>
#include <vpvl2/qt/CString.h>

/* lupdate cannot parse tr() syntax correctly */

namespace vpvm
{

using namespace vpvl2;
using namespace vpvl2::qt;

TimelineTabWidget::TimelineTabWidget(QSettings *settingsRef,
                                     BoneMotionModel *bmm,
                                     MorphMotionModel *mmm,
                                     SceneMotionModel *smm,
                                     QWidget *parent)
    : QWidget(parent),
      m_tabWidget(new QTabWidget()),
      m_boneTimeline(new TimelineWidget(bmm, true, this)),
      m_morphTimeline(new TimelineWidget(mmm, true, this)),
      m_sceneTimeline(new TimelineWidget(smm, false, this)),
      m_boneButtonGroup(new QButtonGroup()),
      m_boneSelectButton(new QRadioButton()),
      m_boneRotateButton(new QRadioButton()),
      m_boneMoveButton(new QRadioButton()),
      m_interpolationDialog(new InterpolationDialog(bmm, smm)),
      m_settingsRef(settingsRef),
      m_lastSelectedModelRef(0)
{
    m_boneTimeline->setFrameIndexSpinBoxEnable(false);
    m_boneSelectButton->setChecked(true);
    m_boneSelectButton->setEnabled(false);
    m_boneRotateButton->setEnabled(false);
    m_boneMoveButton->setEnabled(false);
    connect(m_boneButtonGroup.data(), SIGNAL(buttonClicked(QAbstractButton*)), SLOT(selectButton(QAbstractButton*)));
    m_boneButtonGroup->addButton(m_boneSelectButton.data());
    m_boneButtonGroup->addButton(m_boneRotateButton.data());
    m_boneButtonGroup->addButton(m_boneMoveButton.data());
    QScopedPointer<QHBoxLayout> subLayout(new QHBoxLayout());
    subLayout->addWidget(m_boneSelectButton.data());
    subLayout->addWidget(m_boneRotateButton.data());
    subLayout->addWidget(m_boneMoveButton.data());
    subLayout->setAlignment(Qt::AlignCenter);
    /* hack bone timeline layout */
    reinterpret_cast<QVBoxLayout *>(m_boneTimeline->layout())->addLayout(subLayout.take());
    m_tabWidget->insertTab(kBoneTabIndex, m_boneTimeline.data(), "");
    m_morphTimeline->setFrameIndexSpinBoxEnable(false);
    m_tabWidget->insertTab(kMorphTabIndex, m_morphTimeline.data(), "");
    m_tabWidget->insertTab(kSceneTabIndex, m_sceneTimeline.data(), "");
    connect(m_tabWidget.data(), SIGNAL(currentChanged(int)), this, SLOT(setCurrentTabIndex(int)));
    connect(m_boneTimeline->treeViewRef()->frozenViewSelectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectBonesByItemSelection(QItemSelection)));
    /* シグナルチェーン (motionDidSeek) を発行し、モデル側のシグナルを TimelineTabWidget のシグナルとして一本化して取り扱う */
    connect(m_boneTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    connect(m_morphTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    connect(m_sceneTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    connect(m_boneTimeline->treeViewRef(), SIGNAL(modelIndexDidSelect(QModelIndexList)), SLOT(openInterpolationDialog(QModelIndexList)));
    connect(m_sceneTimeline->treeViewRef(), SIGNAL(modelIndexDidSelect(QModelIndexList)), SLOT(openInterpolationDialog(QModelIndexList)));
    connect(bmm, SIGNAL(modelDidChange(IModel*)), SLOT(toggleBoneEnable(IModel*)));
    connect(mmm, SIGNAL(modelDidChange(IModel*)), SLOT(toggleMorphEnable(IModel*)));
    connect(bmm, SIGNAL(bonesDidSelect(QList<IBone*>)), SLOT(toggleBoneButtonsByBone(QList<IBone*>)));
    /* モーションを読み込んだらフローズンビューを忘れずに更新しておく(フローズンビューが勢い良くスクロール出来てしまうことを防ぐ) */
    connect(bmm, SIGNAL(motionDidUpdate(IModel*)), m_boneTimeline->treeViewRef(), SLOT(updateFrozenTreeView()));
    connect(mmm, SIGNAL(motionDidUpdate(IModel*)), m_morphTimeline->treeViewRef(), SLOT(updateFrozenTreeView()));
    connect(smm, SIGNAL(motionDidUpdate(IModel*)), m_sceneTimeline->treeViewRef(), SLOT(updateFrozenTreeView()));
    /* フレームが切り替わったら現在のフレーム位置を設定し直す */
    connect(bmm, SIGNAL(timeIndexDidChange(IKeyframe::TimeIndex,IKeyframe::TimeIndex)),
            m_boneTimeline.data(), SLOT(setCurrentTimeIndex(IKeyframe::TimeIndex)));
    connect(mmm, SIGNAL(timeIndexDidChange(IKeyframe::TimeIndex,IKeyframe::TimeIndex)),
            m_morphTimeline.data(), SLOT(setCurrentTimeIndex(IKeyframe::TimeIndex)));
    connect(smm, SIGNAL(timeIndexDidChange(IKeyframe::TimeIndex,IKeyframe::TimeIndex)),
            m_sceneTimeline.data(), SLOT(setCurrentTimeIndex(IKeyframe::TimeIndex)));
    QScopedPointer<QVBoxLayout> layout(new QVBoxLayout());
    layout->addWidget(m_tabWidget.data());
    retranslate();
    setLayout(layout.take());
    restoreGeometry(m_settingsRef->value("timelineTabWidget/geometry").toByteArray());
}

TimelineTabWidget::~TimelineTabWidget()
{
    disconnect(m_tabWidget.data(), SIGNAL(currentChanged(int)), this, SLOT(setCurrentTabIndex(int)));
}

void TimelineTabWidget::addKeyframesFromSelectedIndices()
{
    currentSelectedTimelineWidgetRef()->treeViewRef()->addKeyframesBySelectedIndices();
}

void TimelineTabWidget::loadPose(VPDFilePtr pose, IModel *model)
{
    BoneMotionModel *m = static_cast<BoneMotionModel *>(m_boneTimeline->treeViewRef()->model());
    m->loadPose(pose, model, m_boneTimeline->selectedFrameIndex());
}

void TimelineTabWidget::retranslate()
{
    m_boneSelectButton->setText(vpvm::TimelineTabWidget::tr("Select"));
    m_boneRotateButton->setText(vpvm::TimelineTabWidget::tr("Rotate"));
    m_boneMoveButton->setText(vpvm::TimelineTabWidget::tr("Move"));
    m_tabWidget->setTabText(kBoneTabIndex, vpvm::TimelineTabWidget::tr("Bone"));
    m_tabWidget->setTabText(kMorphTabIndex, vpvm::TimelineTabWidget::tr("Morph"));
    m_tabWidget->setTabText(kSceneTabIndex, vpvm::TimelineTabWidget::tr("Scene"));
    setWindowTitle(vpvm::TimelineTabWidget::tr("Motion Timeline"));
}

void TimelineTabWidget::savePose(VPDFile *pose, IModel *model)
{
    BoneMotionModel *m = static_cast<BoneMotionModel *>(m_boneTimeline->treeViewRef()->model());
    m->savePose(pose, model, m_boneTimeline->selectedFrameIndex());
}

void TimelineTabWidget::addMorphKeyframesAtCurrentFrameIndex(IMorph *morph)
{
    /*
     * 渡された頂点モーフの名前と重み係数を元に新しい頂点モーフのキーフレームとして登録する処理
     * (FaceKeyframe#setFrameIndex は KeyFramePair の第一引数を元に SetFramesCommand で行ってる)
     */
    if (morph) {
        MorphMotionModel *model = static_cast<MorphMotionModel *>(m_morphTimeline->treeViewRef()->model());
        MorphMotionModel::KeyFramePairList keyframes;
        QScopedPointer<IMorphKeyframe> keyframe;
        int frameIndex = m_morphTimeline->selectedFrameIndex();
        keyframe.reset(model->factoryRef()->createMorphKeyframe(model->currentMotionRef()));
        keyframe->setName(morph->name());
        keyframe->setWeight(morph->weight());
        keyframes.append(MorphMotionModel::KeyFramePair(frameIndex, MorphMotionModel::KeyFramePtr(keyframe.take())));
        model->setKeyframes(keyframes);
    }
}

void TimelineTabWidget::setCurrentFrameIndex(int value)
{
    /* 二重呼出になってしまうため、一時的に motionDidSeek シグナルを止める */
    disconnect(m_boneTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)),
               this, SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    disconnect(m_morphTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)),
               this, SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    disconnect(m_sceneTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)),
               this, SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    m_boneTimeline->setCurrentTimeIndex(value);
    m_morphTimeline->setCurrentTimeIndex(value);
    m_sceneTimeline->setCurrentTimeIndex(value);
    connect(m_boneTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)),
            SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    connect(m_morphTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)),
            SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    connect(m_sceneTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)),
            SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
}

void TimelineTabWidget::setCurrentFrameIndexZero()
{
    setCurrentFrameIndex(0);
}

void TimelineTabWidget::insertKeyframesBySelectedIndices()
{
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
    {
        /* 選択されたボーンをキーフレームとして登録する */
        TimelineTreeView *view = m_boneTimeline->treeViewRef();
        const QModelIndexList &indices = view->selectionModel()->selectedIndexes();
        BoneMotionModel *model = static_cast<BoneMotionModel *>(view->model());
        Factory *factory = model->factoryRef();
        BoneMotionModel::KeyFramePairList boneFrames;
        QScopedPointer<IBoneKeyframe> frame;
        foreach (const QModelIndex &index, indices) {
            const QString &name = model->nameFromModelIndex(index);
            int frameIndex = MotionBaseModel::toTimeIndex(index);
            CString s(name);
            frame.reset(factory->createBoneKeyframe(model->currentMotionRef()));
            frame->setName(&s);
            frame->setDefaultInterpolationParameter();
            boneFrames.append(BoneMotionModel::KeyFramePair(frameIndex, BoneMotionModel::KeyFramePtr(frame.take())));
        }
        model->setKeyframes(boneFrames);
        break;
    }
    case kMorphTabIndex:
    {
        /* 選択されたモーフをキーフレームとして登録する */
        TimelineTreeView *view = m_morphTimeline->treeViewRef();
        const QModelIndexList &indices = view->selectionModel()->selectedIndexes();
        MorphMotionModel *model = static_cast<MorphMotionModel *>(view->model());
        Factory *factory = model->factoryRef();
        MorphMotionModel::KeyFramePairList faceFrames;
        QScopedPointer<IMorphKeyframe> frame;
        foreach (const QModelIndex &index, indices) {
            const QString &name = model->nameFromModelIndex(index);
            int frameIndex = MotionBaseModel::toTimeIndex(index);
            CString s(name);
            frame.reset(factory->createMorphKeyframe(model->currentMotionRef()));
            frame->setName(&s);
            frame->setWeight(0);
            faceFrames.append(MorphMotionModel::KeyFramePair(frameIndex, MorphMotionModel::KeyFramePtr(frame.take())));
        }
        model->setKeyframes(faceFrames);
        break;
    }
    }
}

void TimelineTabWidget::deleteKeyframesBySelectedIndices()
{
    /* 選択されたボーンまたはモーフに登録されているキーフレームを削除する */
    currentSelectedTimelineWidgetRef()->treeViewRef()->deleteKeyframesBySelectedIndices();
}

void TimelineTabWidget::copyKeyframes()
{
    /* 選択されたボーンまたはモーフに登録されているキーフレームを対象のフレームの位置にコピーする */
    TimelineWidget *widget = currentSelectedTimelineWidgetRef();
    TimelineTreeView *treeView = widget->treeViewRef();
    MotionBaseModel *model = static_cast<MotionBaseModel *>(treeView->model());
    model->copyKeyframesByModelIndices(treeView->selectionModel()->selectedIndexes(), widget->selectedFrameIndex());
}

void TimelineTabWidget::cutKeyframes()
{
    /* 選択されたボーンまたはモーフに登録されているキーフレームをカット（対象のフレームの位置をコピーして削除） */
    TimelineWidget *widget = currentSelectedTimelineWidgetRef();
    TimelineTreeView *treeView = widget->treeViewRef();
    MotionBaseModel *model = static_cast<MotionBaseModel *>(treeView->model());
    model->cutKeyframesByModelIndices(treeView->selectionModel()->selectedIndexes(), widget->selectedFrameIndex());
}

void TimelineTabWidget::pasteKeyframes()
{
    /* 選択されたボーンまたはモーフに登録されているキーフレームを対象のフレームの位置にペーストする */
    TimelineWidget *widget = currentSelectedTimelineWidgetRef();
    TimelineTreeView *treeView = widget->treeViewRef();
    MotionBaseModel *model = static_cast<MotionBaseModel *>(treeView->model());
    model->pasteKeyframesByTimeIndex(widget->selectedFrameIndex());
}

void TimelineTabWidget::pasteKeyframesWithReverse()
{
    /* 選択されたボーンに登録されているキーフレームを対象のフレームの位置に反転ペーストする */
    TimelineTreeView *treeView;
    BoneMotionModel *model;
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
        treeView = m_boneTimeline->treeViewRef();
        model = static_cast<BoneMotionModel *>(treeView->model());
        model->pasteReversedFrame(m_boneTimeline->selectedFrameIndex());
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
    Type type;
    IModel *lastSelectedModel = 0;
    switch (index) {
    case kBoneTabIndex: {
        static_cast<PMDMotionModel *>(m_boneTimeline->treeViewRef()->model())->setActiveUndoStack();
        type = kBone;
        lastSelectedModel = m_lastSelectedModelRef;
        break;
    }
    case kMorphTabIndex: {
        static_cast<PMDMotionModel *>(m_morphTimeline->treeViewRef()->model())->setActiveUndoStack();
        type = kMorph;
        lastSelectedModel = m_lastSelectedModelRef;
        break;
    }
    case kSceneTabIndex: {
        SceneMotionModel *model = static_cast<SceneMotionModel *>(m_sceneTimeline->treeViewRef()->model());
        model->setActiveUndoStack();
        type = kScene;
        break;
    }
    default:
        return;
    }
    emit currentTabDidChange(type);
    emit currentModelDidChange(lastSelectedModel);
}

void TimelineTabWidget::notifyCurrentTabIndex()
{
    /*
     * 場面タブでモデル読み込みをするとボーンとモーフのキーフレームが消失してしまう。
     * そのため、ボーンモードに強制切り替えにする
     */
    if (m_tabWidget->currentIndex() == kSceneTabIndex) {
        setCurrentTabIndex(kBoneTabIndex);
        m_tabWidget->setCurrentIndex(kBoneTabIndex);
    }
    else {
        setCurrentTabIndex(m_tabWidget->currentIndex());
    }
}

void TimelineTabWidget::toggleBoneEnable(IModel *model)
{
    bool value = model ? true : false;
    m_boneTimeline->treeViewRef()->updateFrozenTreeView();
    m_boneTimeline->setFrameIndexSpinBoxEnable(value);
    m_boneSelectButton->setChecked(true);
    m_boneSelectButton->setEnabled(value);
    m_boneRotateButton->setEnabled(false);
    m_boneMoveButton->setEnabled(false);
}

void TimelineTabWidget::toggleMorphEnable(IModel *model)
{
    m_morphTimeline->treeViewRef()->updateFrozenTreeView();
    m_morphTimeline->setFrameIndexSpinBoxEnable(model ? true : false);
}

void TimelineTabWidget::toggleBoneButtonsByBone(const QList<IBone *> &bones)
{
    if (!bones.isEmpty()) {
        IBone *bone = bones.first();
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
    /* 登録済みのすべてのキーフレームを選択状態にする */
    MotionBaseModel *model = static_cast<MotionBaseModel *>(currentSelectedTimelineWidgetRef()->treeViewRef()->model());
    selectFrameIndices(0, model->maxFrameIndex());
}

void TimelineTabWidget::openFrameSelectionDialog()
{
    /* キーフレーム選択ダイアログを表示する */
    if (!m_frameSelectionDialog) {
        m_frameSelectionDialog.reset(new FrameSelectionDialog(this));
        connect(m_frameSelectionDialog.data(), SIGNAL(frameIndicesDidSelect(int,int)),
                this, SLOT(selectFrameIndices(int,int)));
    }
    MotionBaseModel *model = static_cast<MotionBaseModel *>(currentSelectedTimelineWidgetRef()->treeViewRef()->model());
    m_frameSelectionDialog->setMaxFrameIndex(model->maxFrameIndex());
    m_frameSelectionDialog->show();
}

void TimelineTabWidget::openFrameWeightDialog()
{
    /* キーフレームの重み付けダイアログを表示する */
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex: {
        FrameWeightDialog dialog(kBone);
        connect(&dialog, SIGNAL(boneWeightDidSet(Vector3,Vector3)),
                m_boneTimeline->treeViewRef(), SLOT(setBoneKeyframesWeightBySelectedIndices(Vector3,Vector3)));
        dialog.exec();
        break;
    }
    case kMorphTabIndex: {
        FrameWeightDialog dialog(kMorph);
        connect(&dialog, SIGNAL(morphKeyframeWeightDidSet(float)),
                m_morphTimeline->treeViewRef(), SLOT(setMorphKeyframesWeightBySelectedIndices(float)));
        dialog.exec();
        break;
    }
    default:
        warning(this, vpvm::TimelineTabWidget::tr("Not supported operation"),
                vpvm::TimelineTabWidget::tr("The timeline is not supported adjusting keyframe weight."));
        break;
    }
}

void TimelineTabWidget::openInterpolationDialog(const QModelIndexList &indices)
{
    /* 補間ダイアログを表示する */
    m_interpolationDialog->setMode(m_tabWidget->currentIndex());
    m_interpolationDialog->setModelIndices(indices);
    if (m_interpolationDialog->hasValidKeyframes()) {
        m_interpolationDialog->show();
    }
    else {
        warning(this,
                vpvm::TimelineTabWidget::tr("No keyframes selected"),
                vpvm::TimelineTabWidget::tr("Select bone or camera keyframe(s) to open interpolation dialog."));
    }
}

void TimelineTabWidget::openInterpolationDialogBySelectedIndices()
{
    TimelineTreeView *view = 0;
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
        view = m_boneTimeline->treeViewRef();
        break;
    case kSceneTabIndex:
        view = m_sceneTimeline->treeViewRef();
        break;
    default:
        break;
    }
    if (view) {
        const QModelIndexList &indices = view->selectionModel()->selectedIndexes();
        openInterpolationDialog(indices);
    }
    else {
        warning(this,
                vpvm::TimelineTabWidget::tr("Interpolation is not supported"),
                vpvm::TimelineTabWidget::tr("Configuration of morph interpolation is not supported (always linear)."));
    }
}

void TimelineTabWidget::selectBones(const QList<IBone *> &bones)
{
    /* 前回の選択状態をリセットして引数に渡された対象のボーンを選択状態にする */
    TimelineTreeView *boneTreeView = m_boneTimeline->treeViewRef();
    BoneMotionModel *bmm = static_cast<BoneMotionModel *>(boneTreeView->model());
    QItemSelectionModel *selectionModel = boneTreeView->selectionModel();
    int currentFrameIndex = m_boneTimeline->selectedFrameIndex();
    const QModelIndexList &indices = bmm->modelIndicesFromBones(bones, currentFrameIndex);
    selectionModel->clearSelection();
    foreach (const QModelIndex &index, indices)
        selectionModel->select(index, QItemSelectionModel::Select);
}

void TimelineTabWidget::selectBonesByItemSelection(const QItemSelection &selection)
{
    BoneMotionModel *bmm = static_cast<BoneMotionModel *>(m_boneTimeline->treeViewRef()->model());
    const QModelIndexList &indices = selection.indexes();
    if (!indices.empty()) {
        QModelIndexList bone;
        bone.append(indices.first());
        bmm->selectBonesByModelIndices(bone);
    }
}

void TimelineTabWidget::selectButton(QAbstractButton *button)
{
    if (button == m_boneSelectButton.data())
        emit editModeDidSet(SceneWidget::kSelect);
    else if (button == m_boneRotateButton.data())
        emit editModeDidSet(SceneWidget::kRotate);
    else if (button == m_boneMoveButton.data())
        emit editModeDidSet(SceneWidget::kMove);
}

void TimelineTabWidget::setLastSelectedModel(IModel *model)
{
    /* タブ移動時でモデル選択を切り替えるため最後に選択したモデルのポインタを保存する処理。NULL はスキップする */
    if (model)
        m_lastSelectedModelRef = model;
    /*
     * 最初の列以外を隠す。モデルを選択した後でないと BoneMotionModel と MorphMotionModel の
     * columnCount が 1 を返してしまうため、ここで処理する
     */
    m_boneTimeline->treeViewRef()->updateFrozenTreeView();
    m_morphTimeline->treeViewRef()->updateFrozenTreeView();
    m_sceneTimeline->treeViewRef()->updateFrozenTreeView();
}

void TimelineTabWidget::clearLastSelectedModel()
{
    m_lastSelectedModelRef = 0;
}

void TimelineTabWidget::selectFrameIndices(int fromIndex, int toIndex)
{
    /* from と to の値が逆転していないかの検証 (本来は from <= to が正しい) */
    if (fromIndex > toIndex)
        qSwap(fromIndex, toIndex);
    QList<int> frameIndices;
    for (int i = fromIndex; i <= toIndex; i++)
        frameIndices.append(i);
    currentSelectedTimelineWidgetRef()->treeViewRef()->selectFrameIndices(frameIndices, true);
}

void TimelineTabWidget::seekFrameIndexFromCurrentFrameIndex(int frameIndex)
{
    /* 指定されたフレームの位置にシークする */
    TimelineWidget *timeline = currentSelectedTimelineWidgetRef();
    currentSelectedTimelineWidgetRef()->setCurrentTimeIndex(timeline->selectedFrameIndex() + frameIndex);
}

TimelineWidget *TimelineTabWidget::currentSelectedTimelineWidgetRef() const
{
    switch (m_tabWidget->currentIndex()) {
    case kBoneTabIndex:
        return m_boneTimeline.data();
    case kMorphTabIndex:
        return m_morphTimeline.data();
    case kSceneTabIndex:
        return m_sceneTimeline.data();
    default:
        qFatal("Unexpected tab index value: %d", m_tabWidget->currentIndex());
        return 0;
    }
}

} /* namespace vpvm */

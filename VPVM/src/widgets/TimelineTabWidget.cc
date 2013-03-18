/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "FrameSelectionDialog.h"
#include "FrameWeightDialog.h"
#include "InterpolationDialog.h"
#include "BoneMotionModel.h"
#include "MorphMotionModel.h"
#include "SceneMotionModel.h"
#include "TimelineTabWidget.h"
#include "TimelineTreeView.h"
#include "TimelineWidget.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/qt/Util.h>

#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets>
#endif

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
      m_morphSlider(new QSlider(Qt::Horizontal)),
      m_morphSpinbox(new QDoubleSpinBox()),
      m_interpolationDialog(new InterpolationDialog(bmm, smm)),
      m_settingsRef(settingsRef),
      m_lastSelectedModelRef(0),
      m_lastEditMode(SceneWidget::kSelect)
{
    m_boneTimeline->setTimeIndexSpinBoxEnable(false);
    m_boneSelectButton->setChecked(true);
    m_boneSelectButton->setEnabled(false);
    m_boneRotateButton->setEnabled(false);
    m_boneMoveButton->setEnabled(false);
    connect(m_boneButtonGroup.data(), SIGNAL(buttonClicked(QAbstractButton*)), SLOT(selectButton(QAbstractButton*)));
    m_morphSlider->setEnabled(false);
    m_morphSpinbox->setEnabled(false);
    m_morphSlider->setRange(0, 1000);
    m_morphSpinbox->setDecimals(3);
    m_morphSpinbox->setSingleStep(0.01);
    connect(m_morphSlider.data(), SIGNAL(valueChanged(int)), SLOT(updateMorphValue(int)));
    connect(m_morphSpinbox.data(), SIGNAL(valueChanged(double)), SLOT(updateMorphValue(double)));
    m_morphSpinbox->setRange(0, 1);
    m_boneButtonGroup->addButton(m_boneSelectButton.data());
    m_boneButtonGroup->addButton(m_boneRotateButton.data());
    m_boneButtonGroup->addButton(m_boneMoveButton.data());
    /* ボーンのタイムラインに「選択」、「回転」、「移動」のラジオボタンを追加 */
    QScopedPointer<QHBoxLayout> subLayout(new QHBoxLayout());
    subLayout->addWidget(m_boneSelectButton.data());
    subLayout->addWidget(m_boneRotateButton.data());
    subLayout->addWidget(m_boneMoveButton.data());
    subLayout->setAlignment(Qt::AlignCenter);
    /* hack bone timeline layout */
    reinterpret_cast<QVBoxLayout *>(m_boneTimeline->layout())->addLayout(subLayout.take());
    m_tabWidget->insertTab(kBoneTabIndex, m_boneTimeline.data(), "");
    subLayout.reset(new QHBoxLayout());
    subLayout->addWidget(m_morphSlider.data());
    subLayout->addWidget(m_morphSpinbox.data());
    subLayout->setAlignment(Qt::AlignCenter);
    /* hack bone timeline layout */
    reinterpret_cast<QVBoxLayout *>(m_morphTimeline->layout())->addLayout(subLayout.take());
    m_morphTimeline->setTimeIndexSpinBoxEnable(false);
    m_tabWidget->insertTab(kMorphTabIndex, m_morphTimeline.data(), "");
    m_tabWidget->insertTab(kSceneTabIndex, m_sceneTimeline.data(), "");
    connect(m_tabWidget.data(), SIGNAL(currentChanged(int)), this, SLOT(setCurrentTabIndex(int)));
    connect(m_boneTimeline->treeViewRef()->frozenViewSelectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectBonesByItemSelection(QItemSelection)));
    connect(m_morphTimeline->treeViewRef()->frozenViewSelectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectMorphsByItemSelection(QItemSelection)));
    /* シグナルチェーン (motionDidSeek) を発行し、モデル側のシグナルを TimelineTabWidget のシグナルとして一本化して取り扱う */
    connect(m_boneTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    connect(m_morphTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    connect(m_sceneTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)));
    connect(m_morphTimeline.data(), SIGNAL(motionDidSeek(IKeyframe::TimeIndex,bool,bool)), SLOT(updateMorphValue()));
    connect(m_boneTimeline->treeViewRef(), SIGNAL(modelIndexDidSelect(QModelIndexList)), SLOT(openInterpolationDialog(QModelIndexList)));
    connect(m_sceneTimeline->treeViewRef(), SIGNAL(modelIndexDidSelect(QModelIndexList)), SLOT(openInterpolationDialog(QModelIndexList)));
    /* モデルが選択された時タイムライン上のボタンなどの有効無効を切り替える */
    connect(bmm, SIGNAL(modelDidChange(IModelSharedPtr)), SLOT(toggleBoneEnable(IModelSharedPtr)));
    connect(mmm, SIGNAL(modelDidChange(IModelSharedPtr)), SLOT(toggleMorphEnable(IModelSharedPtr)));
    /* ボーンまたはモーフが選択された時のタイムライン上のボタンなどの有効無効を切り替える */
    connect(bmm, SIGNAL(bonesDidSelect(QList<IBone*>)), SLOT(toggleBoneButtonsByBones(QList<IBone*>)));
    connect(mmm, SIGNAL(morphsDidSelect(QList<IMorph*>)), SLOT(toggleMorphByMorph(QList<IMorph*>)));
    /* モーションを読み込んだらフローズンビューを忘れずに更新しておく(フローズンビューが勢い良くスクロール出来てしまうことを防ぐ) */
    connect(bmm, SIGNAL(motionDidUpdate(IModelSharedPtr)), m_boneTimeline->treeViewRef(), SLOT(updateFrozenTreeView()));
    connect(mmm, SIGNAL(motionDidUpdate(IModelSharedPtr)), m_morphTimeline->treeViewRef(), SLOT(updateFrozenTreeView()));
    connect(smm, SIGNAL(motionDidUpdate(IModelSharedPtr)), m_sceneTimeline->treeViewRef(), SLOT(updateFrozenTreeView()));
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
    TimelineTreeView *treeView = currentSelectedTimelineWidgetRef()->treeViewRef();
    treeView->addKeyframesBySelectedIndices();
}

void TimelineTabWidget::loadPose(PosePtr pose, IModelSharedPtr model)
{
    BoneMotionModel *bmm = static_cast<BoneMotionModel *>(m_boneTimeline->treeViewRef()->model());
    bmm->loadPose(pose, model, m_boneTimeline->selectedTimeIndex());
    MorphMotionModel *mmm = static_cast<MorphMotionModel *>(m_morphTimeline->treeViewRef()->model());
    mmm->loadPose(pose, model, m_morphTimeline->selectedTimeIndex());
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

void TimelineTabWidget::addMorphKeyframesAtCurrentTimeIndex(IMorph *morph)
{
    /*
     * 渡された頂点モーフの名前と重み係数を元に新しい頂点モーフのキーフレームとして登録する処理
     * (FaceKeyframe#setTimeIndex は KeyFramePair の第一引数を元に SetFramesCommand で行ってる)
     */
    if (morph) {
        MorphMotionModel *model = static_cast<MorphMotionModel *>(m_morphTimeline->treeViewRef()->model());
        MorphMotionModel::KeyFramePairList keyframes;
        QScopedPointer<IMorphKeyframe> keyframe;
        int timeIndex = m_morphTimeline->selectedTimeIndex();
        keyframe.reset(model->factoryRef()->createMorphKeyframe(model->currentMotionRef().data()));
        keyframe->setName(morph->name());
        keyframe->setWeight(morph->weight());
        keyframes.append(MorphMotionModel::KeyFramePair(timeIndex, MorphMotionModel::KeyFramePtr(keyframe.take())));
        model->setKeyframes(keyframes);
    }
}

void TimelineTabWidget::setCurrentTimeIndex(int value)
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

void TimelineTabWidget::setCurrentTimeIndexZero()
{
    setCurrentTimeIndex(0);
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
        IMotionSharedPtr motion = model->currentMotionRef();
        Factory *factory = model->factoryRef();
        BoneMotionModel::KeyFramePairList boneFrames;
        QScopedPointer<IBoneKeyframe> frame;
        foreach (const QModelIndex &index, indices) {
            const QString &name = model->nameFromModelIndex(index);
            int timeIndex = MotionBaseModel::toTimeIndex(index);
            String s(Util::fromQString(name));
            frame.reset(factory->createBoneKeyframe(motion.data()));
            frame->setName(&s);
            frame->setDefaultInterpolationParameter();
            boneFrames.append(BoneMotionModel::KeyFramePair(timeIndex, BoneMotionModel::KeyFramePtr(frame.take())));
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
        IMotionSharedPtr motion = model->currentMotionRef();
        Factory *factory = model->factoryRef();
        MorphMotionModel::KeyFramePairList faceFrames;
        QScopedPointer<IMorphKeyframe> frame;
        foreach (const QModelIndex &index, indices) {
            const QString &name = model->nameFromModelIndex(index);
            int timeIndex = MotionBaseModel::toTimeIndex(index);
            String s(Util::fromQString(name));
            frame.reset(factory->createMorphKeyframe(motion.data()));
            frame->setName(&s);
            frame->setWeight(0);
            faceFrames.append(MorphMotionModel::KeyFramePair(timeIndex, MorphMotionModel::KeyFramePtr(frame.take())));
        }
        model->setKeyframes(faceFrames);
        break;
    }
    }
}

void TimelineTabWidget::deleteKeyframesBySelectedIndices()
{
    /* 選択されたボーンまたはモーフに登録されているキーフレームを削除する */
    TimelineTreeView *treeView = currentSelectedTimelineWidgetRef()->treeViewRef();
    treeView->deleteKeyframesBySelectedIndices();
}

void TimelineTabWidget::copyKeyframes()
{
    /* 選択されたボーンまたはモーフに登録されているキーフレームを対象のフレームの位置にコピーする */
    TimelineWidget *widget = currentSelectedTimelineWidgetRef();
    TimelineTreeView *treeView = widget->treeViewRef();
    MotionBaseModel *model = static_cast<MotionBaseModel *>(treeView->model());
    model->copyKeyframesByModelIndices(treeView->selectionModel()->selectedIndexes(), widget->selectedTimeIndex());
}

void TimelineTabWidget::cutKeyframes()
{
    /* 選択されたボーンまたはモーフに登録されているキーフレームをカット（対象のフレームの位置をコピーして削除） */
    TimelineWidget *widget = currentSelectedTimelineWidgetRef();
    TimelineTreeView *treeView = widget->treeViewRef();
    MotionBaseModel *model = static_cast<MotionBaseModel *>(treeView->model());
    model->cutKeyframesByModelIndices(treeView->selectionModel()->selectedIndexes(), widget->selectedTimeIndex());
}

void TimelineTabWidget::pasteKeyframes()
{
    /* 選択されたボーンまたはモーフに登録されているキーフレームを対象のフレームの位置にペーストする */
    TimelineWidget *widget = currentSelectedTimelineWidgetRef();
    TimelineTreeView *treeView = widget->treeViewRef();
    MotionBaseModel *model = static_cast<MotionBaseModel *>(treeView->model());
    model->pasteKeyframesByTimeIndex(widget->selectedTimeIndex());
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
        model->pasteReversedFrame(m_boneTimeline->selectedTimeIndex());
        break;
    default:
        pasteKeyframes();
    }
}

void TimelineTabWidget::nextFrame()
{
    seekTimeIndexFromCurrentTimeIndex(1);
}

void TimelineTabWidget::previousFrame()
{
    seekTimeIndexFromCurrentTimeIndex(-1);
}

void TimelineTabWidget::setCurrentTabIndex(int index)
{
    IModelSharedPtr lastSelectedModel;
    SceneWidget::EditMode mode = SceneWidget::kNone;
    switch (index) {
    case kBoneTabIndex: {
        PMDMotionModel *model = static_cast<PMDMotionModel *>(m_boneTimeline->treeViewRef()->model());
        mode = m_lastEditMode;
        model->activateUndoStack();
        lastSelectedModel = m_lastSelectedModelRef;
        break;
    }
    case kMorphTabIndex: {
        PMDMotionModel *model = static_cast<PMDMotionModel *>(m_morphTimeline->treeViewRef()->model());
        model->activateUndoStack();
        lastSelectedModel = m_lastSelectedModelRef;
        break;
    }
    case kSceneTabIndex: {
        SceneMotionModel *model = static_cast<SceneMotionModel *>(m_sceneTimeline->treeViewRef()->model());
        model->setActiveUndoStack();
        break;
    }
    default:
        return;
    }
    emit currentModelDidChange(lastSelectedModel, mode);
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

void TimelineTabWidget::toggleBoneEnable(const IModelSharedPtr model)
{
    m_boneTimeline->treeViewRef()->updateFrozenTreeView();
    m_boneTimeline->setTimeIndexSpinBoxEnable(model);
    /* デフォルトは「選択」ボタンが有効になる */
    m_boneSelectButton->setChecked(true);
    m_boneSelectButton->setEnabled(model);
    m_boneRotateButton->setEnabled(false);
    m_boneMoveButton->setEnabled(false);
}

void TimelineTabWidget::toggleMorphEnable(const IModelSharedPtr model)
{
    m_morphTimeline->treeViewRef()->updateFrozenTreeView();
    m_morphTimeline->setTimeIndexSpinBoxEnable(model);
}

void TimelineTabWidget::toggleBoneButtonsByBones(const QList<IBone *> &bones)
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

void TimelineTabWidget::toggleMorphByMorph(const QList<IMorph *> &morphs)
{
    MorphMotionModel *mmm = static_cast<MorphMotionModel *>(m_morphTimeline->treeViewRef()->model());
    if (!morphs.isEmpty()) {
        mmm->selectMorphs(morphs);
        m_morphSlider->setEnabled(true);
        m_morphSpinbox->setEnabled(true);
    }
    else {
        mmm->selectMorphs(QList<IMorph *>());
        m_morphSlider->setEnabled(false);
        m_morphSpinbox->setEnabled(false);
    }
}

void TimelineTabWidget::selectAllRegisteredKeyframes()
{
    /* 登録済みのすべてのキーフレームを選択状態にする */
    MotionBaseModel *model = static_cast<MotionBaseModel *>(currentSelectedTimelineWidgetRef()->treeViewRef()->model());
    selectFrameIndices(0, model->maxTimeIndex());
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
    m_frameSelectionDialog->setMaxTimeIndex(model->maxTimeIndex());
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
        Util::warning(this, vpvm::TimelineTabWidget::tr("Not supported operation"),
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
        Util::warning(this,
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
        Util::warning(this,
                      vpvm::TimelineTabWidget::tr("Interpolation is not supported"),
                      vpvm::TimelineTabWidget::tr("Configuration of morph interpolation is not supported (always linear)."));
    }
}

void TimelineTabWidget::selectBones(const QList<IBone *> &bones)
{
    /* 前回の選択状態をリセットして引数に渡された対象のボーンを選択状態にする */
    TimelineTreeView *boneTreeView = m_boneTimeline->treeViewRef();
    BoneMotionModel *bmm = static_cast<BoneMotionModel *>(boneTreeView->model());
    if (!bmm->isSelectionIdentical(bones)) {
        QItemSelectionModel *selectionModel = boneTreeView->selectionModel();
        int currentTimeIndex = m_boneTimeline->selectedTimeIndex();
        const QModelIndexList &indices = bmm->modelIndicesFromBones(bones, currentTimeIndex);
        selectionModel->clearSelection();
        foreach (const QModelIndex &index, indices)
            selectionModel->select(index, QItemSelectionModel::Select);
        bmm->selectBones(bones);
    }
}

void TimelineTabWidget::selectMorphs(const QList<IMorph *> &morphs)
{
    /* 前回の選択状態をリセットして引数に渡された対象のモーフを選択状態にする */
    TimelineTreeView *morphTreeView = m_morphTimeline->treeViewRef();
    MorphMotionModel *mmm = static_cast<MorphMotionModel *>(morphTreeView->model());
    if (!mmm->isSelectionIdentical(morphs)) {
        QItemSelectionModel *selectionModel = morphTreeView->selectionModel();
        int currentTimeIndex = m_boneTimeline->selectedTimeIndex();
        const QModelIndexList &indices = mmm->modelIndicesFromMorphs(morphs, currentTimeIndex);
        selectionModel->clearSelection();
        foreach (const QModelIndex &index, indices)
            selectionModel->select(index, QItemSelectionModel::Select);
        mmm->selectMorphs(morphs);
    }
    if (!morphs.isEmpty()) {
        IMorph *morph = morphs.first();
        updateMorphValue(morph->weight());
    }
}

void TimelineTabWidget::selectBonesByItemSelection(const QItemSelection &selection)
{
    TimelineTreeView *treeViewRef = m_boneTimeline->treeViewRef();
    BoneMotionModel *bmm = static_cast<BoneMotionModel *>(treeViewRef->model());
    const QModelIndexList &indices = selection.indexes();
    if (!indices.empty()) {
        const QModelIndex &index = indices.first();
        QModelIndexList bone;
        bone.append(index);
        /* 最初に選択されたボーンのインデックスを BoneMotionModel の selectBones に伝播させる */
        bmm->selectBonesByModelIndices(bone);
        if (index.column() == 0) {
            QItemSelection newSelection;
            /*
             *上で選択された部分がカテゴリの場合は名前からボーンを経由してモデルのインデックスを検索して
             * 現在のタイムラインのフレーム位置にあるフレームを選択状態にする
             */
            String s(Util::fromQString(bmm->nameFromModelIndex(index)));
            QList<IBone *> bones; bones.append(bmm->selectedModel()->findBone(&s));
            const QModelIndexList &indices = bmm->modelIndicesFromBones(bones, m_boneTimeline->currentTimeIndex());
            foreach (const QModelIndex &index, indices)
                newSelection.append(QItemSelectionRange(index));
            treeViewRef->selectionModel()->select(newSelection, QItemSelectionModel::ClearAndSelect);
        }
    }
}

void TimelineTabWidget::selectMorphsByItemSelection(const QItemSelection &selection)
{
    TimelineTreeView *treeViewRef = m_morphTimeline->treeViewRef();
    MorphMotionModel *mmm = static_cast<MorphMotionModel *>(treeViewRef->model());
    const QModelIndexList &indices = selection.indexes();
    if (!indices.empty()) {
        const QModelIndex &index = indices.first();
        QModelIndexList morph;
        morph.append(index);
        /* 最初に選択されたモーフのインデックスを MorphMotionModel の selectMorphs に伝播させる */
        mmm->selectMorphsByModelIndices(morph);
        if (index.column() == 0) {
            QItemSelection newSelection;
            /*
             *上で選択された部分がカテゴリの場合は名前からモーフを経由してモデルのインデックスを検索して
             * 現在のタイムラインのフレーム位置にあるフレームを選択状態にする
             */
            String s(Util::fromQString(mmm->nameFromModelIndex(index)));
            QList<IMorph *> morphs; morphs.append(mmm->selectedModel()->findMorph(&s));
            const QModelIndexList &indices = mmm->modelIndicesFromMorphs(morphs, m_morphTimeline->currentTimeIndex());
            foreach (const QModelIndex &index, indices)
                newSelection.append(QItemSelectionRange(index));
            treeViewRef->selectionModel()->select(newSelection, QItemSelectionModel::ClearAndSelect);
        }
    }
}

void TimelineTabWidget::selectButton(QAbstractButton *button)
{
    if (button == m_boneSelectButton.data()) {
        m_lastEditMode = SceneWidget::kSelect;
    }
    else if (button == m_boneRotateButton.data()) {
        m_lastEditMode = SceneWidget::kRotate;
    }
    else if (button == m_boneMoveButton.data()) {
        m_lastEditMode = SceneWidget::kMove;
    }
    emit editModeDidSet(m_lastEditMode);
}

void TimelineTabWidget::setLastSelectedModel(IModelSharedPtr model)
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
    m_lastSelectedModelRef.clear();
}

void TimelineTabWidget::selectFrameIndices(int fromIndex, int toIndex)
{
    /* from と to の値が逆転していないかの検証 (本来は from <= to が正しい) */
    if (fromIndex > toIndex)
        qSwap(fromIndex, toIndex);
    QList<int> frameIndices;
    for (int i = fromIndex; i <= toIndex; i++)
        frameIndices.append(i);
    TimelineTreeView *treeView = currentSelectedTimelineWidgetRef()->treeViewRef();
    treeView->selectFrameIndices(frameIndices, true);
}

void TimelineTabWidget::seekTimeIndexFromCurrentTimeIndex(int timeIndex)
{
    /* 指定されたフレームの位置にシークする */
    TimelineWidget *timeline = currentSelectedTimelineWidgetRef();
    timeline->setCurrentTimeIndex(timeline->selectedTimeIndex() + timeIndex);
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

void TimelineTabWidget::updateMorphValue()
{
    /* モーフモーションがシークされた時に呼ばれる */
    MorphMotionModel *mmm = static_cast<MorphMotionModel *>(m_morphTimeline->treeViewRef()->model());
    if (IMorph *morph = mmm->selectedMorph()) {
        updateMorphValue(morph->weight());
    }
}

void TimelineTabWidget::updateMorphValue(int value)
{
    /* モーフスライダーから呼ばれる */
    updateMorphValue(value / 1000.0);
}

void TimelineTabWidget::updateMorphValue(double value)
{
    /* モーフスライダーまたはスピンボックスから呼ばれる */
    MorphMotionModel *mmm = static_cast<MorphMotionModel *>(m_morphTimeline->treeViewRef()->model());
    if (IMorph *morph = mmm->selectedMorph()) {
        mmm->setWeight(value, morph);
        m_morphSlider->setValue(value * 1000);
        m_morphSpinbox->setValue(value);
    }
}

} /* namespace vpvm */

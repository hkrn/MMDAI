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

#ifndef TIMELINETABWIDGET_H
#define TIMELINETABWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QAbstractItemView>
#include "common/SceneWidget.h"

namespace vpvl {
class Bone;
class Face;
class PMDModel;
class VPDPose;
}

class QAbstractButton;
class QButtonGroup;
class QRadioButton;
class QSettings;
class QTabWidget;
class FrameSelectionDialog;
class FrameWeightDialog;
class InterpolationWidget;
class TimelineWidget;
class BoneMotionModel;
class MorphMotionModel;
class MotionBaseModel;
class SceneMotionModel;
class VPDFile;

class TimelineTabWidget : public QWidget
{
    Q_OBJECT

public:
    enum Type {
        kBone,
        kMorph,
        kScene
    };

    static const int kSceneTabIndex = 0;
    static const int kBoneTabIndex = 1;
    static const int kMorphTabIndex = 2;
    static const int kInterpolationTabIndex = 3;

    explicit TimelineTabWidget(QSettings *settings,
                               BoneMotionModel *bmm,
                               MorphMotionModel *mmm,
                               SceneMotionModel *smm,
                               QWidget *parent = 0);
    ~TimelineTabWidget();

    InterpolationWidget *interpolationWidget() const { return m_interpolationWidget; }

public slots:
    void addKeyFramesFromSelectedIndices();
    void loadPose(VPDFile *pose, vpvl::PMDModel *model);
    void savePose(VPDFile *pose, vpvl::PMDModel *model);
    void selectFrameIndices(int fromIndex, int toIndex);

signals:
    void motionDidSeek(float frameIndex);
    void currentTabDidChange(int type);

private slots:
    void retranslate();
    void addBoneKeyFrameAtCurrentFrameIndex(vpvl::Bone *bone);
    void addFaceKeyFrameAtCurrentFrameIndex(vpvl::Face *face);
    void setCurrentFrameIndex(int value);
    void setCurrentFrameIndexZero();
    void insertFrame();
    void deleteFrame();
    void copyKeyframes();
    void cutKeyframes();
    void pasteKeyframes();
    void pasteKeyframesWithReverse();
    void nextFrame();
    void previousFrame();
    void setCurrentTabIndex(int index);
    void notifyCurrentTabIndex();
    void toggleBoneEnable(vpvl::PMDModel *model);
    void toggleFaceEnable(vpvl::PMDModel *model);
    void toggleBoneButtonsByBone(const QList<vpvl::Bone *> &bones);
    void selectAllRegisteredKeyframes();
    void openFrameSelectionDialog();
    void openFrameWeightDialog();

signals:
    void editModeDidSet(SceneWidget::EditMode mode);

private slots:
    void selectButton(QAbstractButton *button);

private:
    void seekFrameIndexFromCurrentFrameIndex(int frameIndex);
    TimelineWidget *currentSelectedTimelineWidget() const;

    QSettings *m_settings;
    QTabWidget *m_tabWidget;
    TimelineWidget *m_boneTimeline;
    TimelineWidget *m_morphTimeline;
    TimelineWidget *m_sceneTimeline;
    InterpolationWidget *m_interpolationWidget;
    QButtonGroup *m_boneButtonGroup;
    QRadioButton *m_boneSelectButton;
    QRadioButton *m_boneRotateButton;
    QRadioButton *m_boneMoveButton;
    FrameSelectionDialog *m_frameSelectionDialog;
    FrameWeightDialog *m_frameWeightDialog;

    Q_DISABLE_COPY(TimelineTabWidget)
};

#endif // TIMELINETABWIDGET_H

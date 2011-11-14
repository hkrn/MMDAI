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

#ifndef TIMELINETABWIDGET_H
#define TIMELINETABWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QAbstractItemView>

namespace vpvl {
class Bone;
class Face;
class PMDModel;
class VPDPose;
}

class QSettings;
class QTabWidget;
class TimelineWidget;
class BoneMotionModel;
class FaceMotionModel;
class SceneMotionModel;
class VPDFile;

class TimelineTabWidget : public QWidget
{
    Q_OBJECT

public:
    enum Type {
        kBone,
        kFace,
        kScene
    };

    explicit TimelineTabWidget(QSettings *settings,
                               BoneMotionModel *bmm,
                               FaceMotionModel *fmm,
                               SceneMotionModel *smm,
                               QWidget *parent = 0);
    ~TimelineTabWidget();

public slots:
    void addKeyFramesFromSelectedIndices();
    void loadPose(VPDFile *pose, vpvl::PMDModel *model);
    void savePose(VPDFile *pose, vpvl::PMDModel *model);

signals:
    void motionDidSeek(float frameIndex);
    void currentTabDidChange(int type);

private slots:
    void retranslate();
    void addBoneKeyFramesFromSelectedIndices();
    void addFaceKeyFramesFromSelectedIndices();
    void addSceneKeyFramesFromSelectedIndices();
    void addBoneKeyFrameAtCurrentFrameIndex(vpvl::Bone *bone);
    void addFaceKeyFrameAtCurrentFrameIndex(vpvl::Face *face);
    void setCurrentFrameIndexZero();
    void insertFrame();
    void deleteFrame();
    void copyFrame();
    void pasteFrame();
    void pasteReversedFrame();
    void setCurrentTabIndex(int index);
    void notifyCurrentTabIndex();

private:
    QSettings *m_settings;
    QTabWidget *m_tabWidget;
    TimelineWidget *m_boneTimeline;
    TimelineWidget *m_faceTimeline;
    TimelineWidget *m_sceneTimeline;

    Q_DISABLE_COPY(TimelineTabWidget)
};

#endif // TIMELINETABWIDGET_H

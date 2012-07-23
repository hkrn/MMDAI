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

#ifndef SCENEMOTIONMODEL_H
#define SCENEMOTIONMODEL_H

#include "models/MotionBaseModel.h"

#include <vpvl2/ICameraKeyframe.h>

namespace vpvl2 {
class Factory;
class ILightKeyframe;
class IModel;
class Scene;
}

class SceneWidget;

class SceneMotionModel : public MotionBaseModel
{
    Q_OBJECT

public:
    typedef QSharedPointer<vpvl2::ICameraKeyframe> CameraKeyframePtr;
    typedef QPair<int, CameraKeyframePtr> CameraKeyframePair;
    typedef QList<CameraKeyframePair> CameraKeyframePairList;
    typedef QSharedPointer<vpvl2::ILightKeyframe> LightKeyframePtr;
    typedef QPair<int, LightKeyframePtr> LightKeyframePair;
    typedef QList<LightKeyframePair> LightKeyframePairList;

    explicit SceneMotionModel(vpvl2::Factory *factory,
                              QUndoGroup *undo,
                              const SceneWidget *sceneWidget,
                              QObject *parent = 0);
    ~SceneMotionModel();

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int maxFrameIndex() const;
    bool forceCameraUpdate() const;
    const QModelIndex frameIndexToModelIndex(ITreeItem *item, int timeIndex) const;

    void saveMotion(vpvl2::IMotion *motion);
    void copyKeyframesByModelIndices(const QModelIndexList &indices, int timeIndex);
    void pasteKeyframesByTimeIndex(int timeIndex);
    const QString nameFromModelIndex(const QModelIndex &index) const;
    CameraKeyframePairList keyframesFromModelIndices(const QModelIndexList &indices) const;

    void setKeyframes(const CameraKeyframePairList &cameraKeyframes, const LightKeyframePairList &lightKeyframes);
    void setActiveUndoStack();
    void refreshScene();
    vpvl2::Factory *factory() const { return m_factory; }

public slots:
    void addKeyframesByModelIndices(const QModelIndexList &indices);
    void deleteKeyframesByModelIndices(const QModelIndexList &indices);
    void applyKeyframeWeightByModelIndices(const QModelIndexList &indices, float value);
    void removeMotion();
    void loadMotion(vpvl2::IMotion *motion);
    void markAsNew() { setModified(false); }

signals:
    void cameraMotionDidLoad();
    void motionDidUpdate(vpvl2::IModel *model);

protected:
    ITreeItem *root() const { return m_rootTreeItem; }

private:
    const SceneWidget *m_sceneWidget;
    QModelIndex m_cameraIndex;
    QModelIndex m_lightIndex;
    QUndoStack *m_stack;
    Values m_cameraData;
    Values m_lightData;
    vpvl2::Factory *m_factory;
    vpvl2::ICameraKeyframe::InterpolationParameter m_cameraInterpolationParameter;
    ITreeItem *m_rootTreeItem;
    ITreeItem *m_cameraTreeItem;
    ITreeItem *m_lightTreeItem;

    Q_DISABLE_COPY(SceneMotionModel)
};

#endif // SCENEMOTIONMODEL_H

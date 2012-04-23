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

#ifndef BONEMOTIONMODEL_H
#define BONEMOTIONMODEL_H

#include "common/VPDFile.h"
#include "models/PMDMotionModel.h"

#include <vpvl2/IBoneKeyframe.h>

namespace vpvl2 {
class Factory;
class IBone;
class IModel;
class IMotion;
}

class SceneWidget;

class BoneMotionModel : public PMDMotionModel
{
    Q_OBJECT

public:
    enum ResetType {
        kX,
        kY,
        kZ,
        kRotation
    };

    typedef QSharedPointer<vpvl2::IBoneKeyframe> KeyFramePtr;
    typedef QPair<int, KeyFramePtr> KeyFramePair;
    typedef QList<KeyFramePair> KeyFramePairList;

    BoneMotionModel(vpvl2::Factory *factory, QUndoGroup *undo, const SceneWidget *sceneWidget, QObject *parent = 0);
    ~BoneMotionModel();

    void saveMotion(vpvl2::IMotion *motion);
    void copyKeyframesByModelIndices(const QModelIndexList &indices, int frameIndex);
    void pasteKeyframesByFrameIndex(int frameIndex);
    void pasteReversedFrame(int frameIndex);
    void applyKeyframeWeightByModelIndices(const QModelIndexList &indices, const vpvl2::Vector3 &position, const vpvl2::Vector3 &rotation);
    const QString nameFromModelIndex(const QModelIndex &index) const;

    void loadPose(VPDFilePtr pose, vpvl2::IModel *model, int frameIndex);
    void savePose(VPDFile *pose, vpvl2::IModel *model, int frameIndex);
    void setFrames(const KeyFramePairList &frames);
    void resetBone(ResetType type);
    void resetAllBones();
    void setPosition(int coordinate, float value);
    void setRotation(int coordinate, float value);

    vpvl2::IBone *findBone(const QString &name) const;
    vpvl2::IBone *selectedBone() const { return m_selectedBones.isEmpty() ? 0 : m_selectedBones.first(); }
    vpvl2::Factory *factory() const { return m_factory; }
    bool isBoneSelected() const { return m_model != 0 && selectedBone() != 0; }
    const vpvl2::IBoneKeyframe::InterpolationParameter &interpolationParameter() const { return m_interpolationParameter; }
    void setInterpolationParameter(const vpvl2::IBoneKeyframe::InterpolationParameter &value) { m_interpolationParameter = value; }

public slots:
    void addKeyframesByModelIndices(const QModelIndexList &indices);
    void selectKeyframesByModelIndices(const QModelIndexList &indices);
    void deleteKeyframesByModelIndices(const QModelIndexList &indices);
    void selectBonesByModelIndices(const QModelIndexList &indices);
    void removeModel();
    void removeMotion();
    void setPMDModel(vpvl2::IModel *model);
    void loadMotion(vpvl2::IMotion *motion, vpvl2::IModel *model);
    void rotateAngle(const vpvl2::Scalar &value, vpvl2::IBone *bone, int flags);
    void translateDelta(const vpvl2::Vector3 &delta, vpvl2::IBone *bone, int flags);
    void translateTo(const vpvl2::Vector3 &position, vpvl2::IBone *bone, int flags);
    void selectBones(const QList<vpvl2::IBone *> &bones);
    void saveTransform();
    void commitTransform();

signals:
    void positionDidChange(vpvl2::IBone *bone, const vpvl2::Vector3 &lastPosition);
    void rotationDidChange(vpvl2::IBone *bone, const vpvl2::Quaternion &lastRotation);
    void bonesDidSelect(const QList<vpvl2::IBone *> &bones);
    void keyframesDidSelect(const QList<BoneMotionModel::KeyFramePtr> &frames);

private:
    void translateInternal(const vpvl2::Vector3 &position, const vpvl2::Vector3 &delta, vpvl2::IBone *bone, int flags);

    const SceneWidget *m_sceneWidget;
    KeyFramePairList m_copiedKeyframes;
    QList<vpvl2::IBone *> m_selectedBones;
    vpvl2::Factory *m_factory;
    vpvl2::IBoneKeyframe::InterpolationParameter m_interpolationParameter;
    /* 操作時のボーンの位置と回転量を保存する。操作中は変化しない (vpvl2::IMorphKeyframe::State と重複するが...) */
    QHash<vpvl2::IBone *, QPair<vpvl2::Vector3, vpvl2::Quaternion> > m_boneTransformStates;

    Q_DISABLE_COPY(BoneMotionModel)
};

#endif // BONEMOTIONMODEL_H

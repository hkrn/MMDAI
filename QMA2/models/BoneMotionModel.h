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

#include <vpvl/BaseAnimation.h>
#include <vpvl/BoneKeyFrame.h>

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

    typedef QSharedPointer<vpvl::BoneKeyframe> KeyFramePtr;
    typedef QPair<int, KeyFramePtr> KeyFramePair;
    typedef QList<KeyFramePair> KeyFramePairList;

    BoneMotionModel(QUndoGroup *undo, const SceneWidget *sceneWidget, QObject *parent = 0);
    ~BoneMotionModel();

    void saveMotion(vpvl::VMDMotion *motion);
    void copyKeyframesByModelIndices(const QModelIndexList &indices, int frameIndex);
    void pasteKeyframesByFrameIndex(int frameIndex);
    void pasteReversedFrame(int frameIndex);
    void applyKeyframeWeightByModelIndices(const QModelIndexList &indices, const vpvl::Vector3 &position, const vpvl::Vector3 &rotation);
    const QByteArray nameFromModelIndex(const QModelIndex &index) const;

    void loadPose(VPDFilePtr pose, vpvl::PMDModel *model, int frameIndex);
    void savePose(VPDFile *pose, vpvl::PMDModel *model, int frameIndex);
    void setFrames(const KeyFramePairList &frames);
    void resetBone(ResetType type);
    void resetAllBones();
    void setPosition(int coordinate, float value);
    void setRotation(int coordinate, float value);

    vpvl::Bone *findBone(const QString &name);
    vpvl::Bone *selectedBone() const { return m_selectedBones.isEmpty() ? 0 : m_selectedBones.first(); }
    bool isBoneSelected() const { return m_model != 0 && selectedBone() != 0; }
    const vpvl::BoneKeyframe::InterpolationParameter &interpolationParameter() const { return m_interpolationParameter; }
    void setInterpolationParameter(const vpvl::BoneKeyframe::InterpolationParameter &value) { m_interpolationParameter = value; }

public slots:
    void addKeyframesByModelIndices(const QModelIndexList &indices);
    void selectKeyframesByModelIndices(const QModelIndexList &indices);
    void deleteKeyframesByModelIndices(const QModelIndexList &indices);
    void selectBonesByModelIndices(const QModelIndexList &indices);
    void removeModel();
    void removeMotion();
    void setPMDModel(vpvl::PMDModel *model);
    void loadMotion(vpvl::VMDMotion *motion, vpvl::PMDModel *model);
    void rotateAngle(const vpvl::Scalar &value, vpvl::Bone *bone, int flags);
    void translateDelta(const vpvl::Vector3 &delta, vpvl::Bone *bone, int flags);
    void selectBones(const QList<vpvl::Bone *> &bones);
    void saveTransform();
    void commitTransform();

signals:
    void positionDidChange(vpvl::Bone *bone, const vpvl::Vector3 &lastPosition);
    void rotationDidChange(vpvl::Bone *bone, const vpvl::Quaternion &lastRotation);
    void bonesDidSelect(const QList<vpvl::Bone *> &bones);
    void keyframesDidSelect(const QList<BoneMotionModel::KeyFramePtr> &frames);

private:
    const SceneWidget *m_sceneWidget;
    KeyFramePairList m_copiedKeyframes;
    QList<vpvl::Bone *> m_selectedBones;
    vpvl::PMDModel::State *m_state;
    vpvl::BoneKeyframe::InterpolationParameter m_interpolationParameter;
    QHash<vpvl::Bone *, QPair<vpvl::Vector3, vpvl::Quaternion> > m_boneTransformStates;

    Q_DISABLE_COPY(BoneMotionModel)
};

#endif // BONEMOTIONMODEL_H

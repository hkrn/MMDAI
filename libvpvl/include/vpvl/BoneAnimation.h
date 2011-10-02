/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef VPVL_BONEANIMATION_H_
#define VPVL_BONEANIMATION_H_

#include "vpvl/BaseAnimation.h"

namespace vpvl
{

class Bone;
class BoneKeyFrame;
class PMDModel;
typedef Array<BoneKeyFrame *> BoneKeyFrameList;
typedef struct BoneAnimationInternal BoneAnimationInternal;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * BoneAnimation class represents a bone Animation that includes many bone key frames
 * of a Vocaloid Motion Data object inherits BaseAnimation.
 */

class VPVL_API BoneAnimation : public BaseAnimation
{
public:
    BoneAnimation();
    ~BoneAnimation();

    static const float kStartingMarginFrame;

    void read(const uint8_t *data, uint32_t size);
    void seek(float frameAt);
    void takeSnap(const Vector3 &center);
    void reset();
    void refresh();

    /**
     * Attach this to the model.
     *
     * After calling this method, internal states to animate are built.
     * If you modified frames of this animation, you should call
     * refresh method to rebuild internal states.
     *
     * This method has no effect if you have already called it.
     *
     * @param model A model to attach the motion
     * @see refresh
     */
    void attachModel(PMDModel *model);

    /**
     * Get a bone key frame associated with index.
     *
     * @param i A frame index to get key frame
     * @return A bone key frame associated with index
     */
    BoneKeyFrame *frameAt(uint32_t i) const {
        return static_cast<BoneKeyFrame *>(m_frames[i]);
    }

    /**
     * Get whether this animation has a frame to control center bone.
     *
     * @return True if the model has center bone
     */
    bool hasCenterBoneAnimation() const {
        return m_hasCenterBoneAnimation;
    }

    /**
     * Get an attached model of this animation.
     *
     * @return An attached model
     */
    PMDModel *attachedModel() const {
        return m_model;
    }

private:
    static float weightValue(const BoneKeyFrame *keyFrame,
                             float w,
                             uint32_t at);
    static void lerpVector3(const BoneKeyFrame *keyFrame,
                            const Vector3 &from,
                            const Vector3 &to,
                            float w,
                            uint32_t at,
                            float &value);
    void buildInternalNodes(vpvl::PMDModel *model);
    void calculateFrames(float frameAt, BoneAnimationInternal *node);

    Hash<HashString, BoneAnimationInternal *> m_name2node;
    PMDModel *m_model;
    bool m_hasCenterBoneAnimation;

    VPVL_DISABLE_COPY_AND_ASSIGN(BoneAnimation)
};

}

#endif

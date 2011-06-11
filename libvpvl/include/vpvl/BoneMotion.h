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

#ifndef VPVL_BONEMOTION_H_
#define VPVL_BONEMOTION_H_

#include <LinearMath/btHashMap.h>
#include "vpvl/BaseMotion.h"

namespace vpvl
{

class Bone;
class BoneKeyFrame;
class PMDModel;
typedef struct BoneMotionInternal BoneMotionInternal;
typedef btAlignedObjectArray<BoneKeyFrame *> BoneKeyFrameList;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * BoneMotion class represents a bone motion that includes many bone key frames
 * of a Vocaloid Motion Data object inherits BaseMotion.
 */

class BoneMotion : public BaseMotion
{
public:
    BoneMotion();
    ~BoneMotion();

    static const float kStartingMarginFrame;

    void read(const uint8_t *data, uint32_t size);
    void seek(float frameAt);
    void takeSnap(const btVector3 &center);
    void attachModel(PMDModel *model);
    void reset();

    const BoneKeyFrameList &frames() const {
        return m_frames;
    }
    bool hasCenterBoneMotion() const {
        return m_hasCenterBoneMotion;
    }
    PMDModel *attachedModel() const {
        return m_model;
    }

private:
    static float weightValue(const BoneKeyFrame *keyFrame,
                             float w,
                             uint32_t at);
    static void lerpVector3(const BoneKeyFrame *keyFrame,
                            const btVector3 &from,
                            const btVector3 &to,
                            float w,
                            uint32_t at,
                            float &value);
    void calculateFrames(float frameAt, BoneMotionInternal *node);

    BoneKeyFrameList m_frames;
    btHashMap<btHashString, BoneMotionInternal *> m_name2node;
    PMDModel *m_model;
    bool m_hasCenterBoneMotion;
};

}

#endif

/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
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

#include "vpvl/BaseMotion.h"

namespace vpvl
{

class Bone;
class BoneKeyFrame;
class PMDModel;
typedef btAlignedObjectArray<BoneKeyFrame *> BoneKeyFrameList;

class BoneMotion
{
public:
    BoneMotion();
    ~BoneMotion();

    static const float kStartingMarginFrame;

    void read(const char *data, uint32_t size);
    void translateFrames(PMDModel *model);
    void calculateFrames(float frameAt);
    void sortFrames();
    void reset();

    const BoneKeyFrameList &frames() const {
        return m_frames;
    }
    const btVector3 &position() const {
        return m_position;
    }
    const btQuaternion &rotation() const {
        return m_rotation;
    }
    const btVector3 &snapPosition() const {
        return m_snapPosition;
    }
    const btQuaternion &snapRotation() const {
        return m_snapRotation;
    }
    const void setSnapPosition(const btVector3 &value) {
        m_snapPosition = value;
    }
    const void setSnapRotation(const btQuaternion &value) {
        m_snapRotation = value;
    }

private:
    static void lerpPosition(const BoneKeyFrame *keyFrame,
                             const btVector3 &from,
                             const btVector3 &to,
                             float w,
                             uint32_t at,
                             float &value);
    void takeSnap(const btVector3 &center);

    Bone *m_bone;
    BoneKeyFrameList m_frames;
    btVector3 m_position;
    btVector3 m_snapPosition;
    btQuaternion m_rotation;
    btQuaternion m_snapRotation;
    uint32_t m_lastIndex;
    uint32_t m_lastLoopStartIndex;
    uint32_t m_noBoneSmearIndex;
    bool m_overrideFirst;
};

}

#endif

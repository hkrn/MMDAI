/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#ifndef VPVL2_VMD_BONEANIMATION_H_
#define VPVL2_VMD_BONEANIMATION_H_

#include "vpvl2/IModel.h"
#include "vpvl2/vmd/BaseAnimation.h"
#include "vpvl2/vmd/BaseKeyframe.h"

namespace vpvl2
{
class IEncoding;

namespace vmd
{
class BoneKeyframe;

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

class VPVL2_API BoneAnimation : public BaseAnimation
{
public:
    struct InternalBoneKeyFrameList;

    BoneAnimation(IEncoding *encoding);
    ~BoneAnimation();

    void read(const uint8_t *data, int size);
    void seek(const IKeyframe::TimeIndex &frameAt);
    void reset();
    void setParentModel(IModel *model);
    BoneKeyframe *frameAt(int i) const;
    BoneKeyframe *findKeyframe(const IKeyframe::TimeIndex &timeIndex, const IString *name) const;

    bool isNullFrameEnabled() const { return m_enableNullFrame; }
    void setNullFrameEnable(bool value) { m_enableNullFrame = value; }

private:
    static const IKeyframe::SmoothPrecision weightValue(const BoneKeyframe *keyFrame,
                                                        const IKeyframe::SmoothPrecision &w,
                                                        int at);
    static void lerpVector3(const BoneKeyframe *keyFrame,
                            const Vector3 &from,
                            const Vector3 &to,
                            const IKeyframe::SmoothPrecision &w,
                            int at,
                            IKeyframe::SmoothPrecision &value);
    void buildInternalKeyFrameList(IModel *model);
    void calculateFrames(const IKeyframe::TimeIndex &frameAt, InternalBoneKeyFrameList *keyFrames);

    IEncoding *m_encoding;
    Hash<HashString, InternalBoneKeyFrameList *> m_name2keyframes;
    IModel *m_model;
    bool m_enableNullFrame;

    VPVL2_DISABLE_COPY_AND_ASSIGN(BoneAnimation)
};

}
}

#endif

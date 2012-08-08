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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/mvd/BoneKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct BoneKeyframeChunk {
    int layerIndex;
    uint64_t timeIndex;
    float position[3];
    float rotation[4];
    InterpolationPair x;
    InterpolationPair y;
    InterpolationPair z;
    InterpolationPair r;
};

#pragma pack(pop)

BoneKeyframe::BoneKeyframe(IEncoding *encoding)
    : BaseKeyframe(),
      m_encoding(encoding)
{
}

BoneKeyframe::~BoneKeyframe()
{
}

bool BoneKeyframe::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    static BoneKeyframeChunk keyframe;
    if (!internal::validateSize(ptr, sizeof(keyframe), rest)) {
        return false;
    }
    return true;
}

void BoneKeyframe::read(const uint8_t *data)
{
}

void BoneKeyframe::write(uint8_t *data) const
{
}

size_t BoneKeyframe::estimateSize() const
{
    return 0;
}

IBoneKeyframe *BoneKeyframe::clone() const
{
    return 0;
}

void BoneKeyframe::setDefaultInterpolationParameter()
{
}

void BoneKeyframe::setInterpolationParameter(InterpolationType type, const QuadWord &value)
{
}

void BoneKeyframe::getInterpolationParameter(InterpolationType type, QuadWord &value) const
{
}

const Vector3 &BoneKeyframe::position() const
{
    return kZeroV3;
}

const Quaternion &BoneKeyframe::rotation() const
{
    return Quaternion::getIdentity();
}

void BoneKeyframe::setPosition(const Vector3 &value)
{
}

void BoneKeyframe::setRotation(const Quaternion &value)
{
}

}
}

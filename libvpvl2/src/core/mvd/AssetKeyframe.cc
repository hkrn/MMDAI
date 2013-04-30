/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "vpvl2/mvd/AssetKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct AssetKeyframeChunk {
    uint64_t timeIndex;
    uint8_t visible;
    uint8_t shadow;
    uint8_t addBlend;
    uint8_t reserved;
    float32_t scaleFactor;
    float32_t opacity;
    int32_t modelID;
    int32_t boneID;
};

#pragma pack(pop)

AssetKeyframe::AssetKeyframe(const Motion *motionRef)
    : VPVL2_KEYFRAME_INITIALIZE_FIELDS(),
      m_motionRef(motionRef)
{
}

AssetKeyframe::~AssetKeyframe()
{
    VPVL2_KEYFRAME_DESTROY_FIELDS()
}

size_t AssetKeyframe::size()
{
    static AssetKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool AssetKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        return false;
    }
    return true;
}

void AssetKeyframe::read(const uint8_t * /* data */)
{
}

void AssetKeyframe::write(uint8_t * /* data */) const
{
}

size_t AssetKeyframe::estimateSize() const
{
    return size();
}

/*
IAssetKeyframe *AssetKeyframe::clone() const
{
    return 0;
}
*/

const Motion *AssetKeyframe::parentMotionRef() const
{
    return m_motionRef;
}

void AssetKeyframe::setName(const IString * /* value */)
{
}

IKeyframe::Type AssetKeyframe::type() const
{
    return IKeyframe::kAssetKeyframe;
}

} /* namespace mvd */
} /* namespace vpvl2 */

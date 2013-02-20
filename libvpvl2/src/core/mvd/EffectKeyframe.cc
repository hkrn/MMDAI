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

#include "vpvl2/mvd/EffectKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct EffectKeyframeChunk {
    EffectKeyframeChunk() {}
    uint64_t timeIndex;
    uint8_t visible;
    uint8_t addBlend;
    uint8_t shadow;
    uint8_t reserved;
    float scaleFactor;
    float opacity;
    int modelID;
    int boneID;
};

#pragma pack(pop)

EffectKeyframe::EffectKeyframe(const Motion *motionRef)
    : VPVL2_KEYFRAME_INITIALIZE_FIELDS(),
      m_ptr(0),
      m_motionRef(motionRef),
      m_parentModelRef(0),
      m_parentBoneRef(0),
      m_scaleFactor(0),
      m_opacity(0),
      m_visible(false),
      m_addBlend(false),
      m_shadow(false)
{
}

EffectKeyframe::~EffectKeyframe()
{
    VPVL2_KEYFRAME_DESTROY_FIELDS()
    delete m_ptr;
    m_ptr = 0;
    m_motionRef = 0;
    m_parentModelRef = 0;
    m_parentBoneRef = 0;
    m_scaleFactor = 0;
    m_opacity = 0;
    m_visible = false;
    m_addBlend = false;
    m_shadow = false;
}

size_t EffectKeyframe::size()
{
    static const EffectKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool EffectKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        return false;
    }
    return true;
}

void EffectKeyframe::read(const uint8_t * /* data */)
{
}

void EffectKeyframe::write(uint8_t * /* data */) const
{
}

size_t EffectKeyframe::estimateSize() const
{
    return size();
}

IEffectKeyframe *EffectKeyframe::clone() const
{
    EffectKeyframe *keyframe = m_ptr = new EffectKeyframe(m_motionRef);
    keyframe->setTimeIndex(m_timeIndex);
    keyframe->setLayerIndex(m_layerIndex);
    keyframe->setVisible(m_visible);
    keyframe->setAddBlendEnable(m_addBlend);
    keyframe->setShadowEnable(m_shadow);
    keyframe->setScaleFactor(m_scaleFactor);
    keyframe->setOpacity(m_opacity);
    keyframe->setParentModelRef(m_parentModelRef);
    keyframe->setParentBoneRef(m_parentBoneRef);
    m_ptr = 0;
    return keyframe;
}

void EffectKeyframe::setName(const IString * /* value */)
{
}

IKeyframe::Type EffectKeyframe::type() const
{
    return kEffectKeyframe;
}

const Motion *EffectKeyframe::parentMotionRef() const
{
    return m_motionRef;
}

bool EffectKeyframe::isVisible() const
{
    return m_visible;
}

bool EffectKeyframe::isAddBlendEnabled() const
{
    return m_addBlend;
}

bool EffectKeyframe::isShadowEnabled() const
{
    return m_shadow;
}

float EffectKeyframe::scaleFactor() const
{
    return m_scaleFactor;
}

float EffectKeyframe::opacity() const
{
    return m_opacity;
}

IModel *EffectKeyframe::parentModelRef() const
{
    return m_parentModelRef;
}

IBone *EffectKeyframe::parentBoneRef() const
{
    return m_parentBoneRef;
}

void EffectKeyframe::setVisible(bool value)
{
    m_visible = value;
}

void EffectKeyframe::setAddBlendEnable(bool value)
{
    m_addBlend = value;
}

void EffectKeyframe::setShadowEnable(bool value)
{
    m_shadow = value;
}

void EffectKeyframe::setScaleFactor(float value)
{
    m_scaleFactor = value;
}

void EffectKeyframe::setOpacity(float value)
{
    m_opacity = value;
}

void EffectKeyframe::setParentModelRef(IModel *value)
{
    m_parentModelRef = value;
}

void EffectKeyframe::setParentBoneRef(IBone *value)
{
    m_parentBoneRef = value;
}

} /* namespace mvd */
} /* namespace vpvl2 */

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

#include "vpvl2/mvd/ModelKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct ModelKeyframeChunk {
    ModelKeyframeChunk() {}
    uint64_t timeIndex;
    uint8_t visible;
    uint8_t shadow;
    uint8_t addBlend;
    uint8_t physics;
    uint8_t physicsStillMode;
    uint8_t reserved[3];
    float32_t edgeWidth;
    uint8_t edgeColor[4];
};

#pragma pack(pop)

ModelKeyframe::ModelKeyframe(const Motion *motionRef, int countOfIKBones)
    : VPVL2_KEYFRAME_INITIALIZE_FIELDS(),
      m_ptr(0),
      m_motionRef(motionRef),
      m_edgeColor(kZeroC),
      m_edgeWidth(0),
      m_countOfIKBones(countOfIKBones),
      m_physicsStillMode(0),
      m_visible(false),
      m_shadow(false),
      m_addBlend(false),
      m_physics(false)
{
    m_bonesOfIK.reserve(countOfIKBones);
}

ModelKeyframe::~ModelKeyframe()
{
    VPVL2_KEYFRAME_DESTROY_FIELDS()
    delete m_ptr;
    m_ptr = 0;
    m_motionRef = 0;
    m_edgeColor.setZero();
    m_edgeWidth = 0;
    m_countOfIKBones = 0;
    m_physicsStillMode = 0;
    m_visible = false;
    m_shadow = false;
    m_addBlend = false;
    m_physics = false;
}

size_t ModelKeyframe::size()
{
    static const ModelKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool ModelKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, size_t countOfIK, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of MVD model keyframe detected: ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
        return false;
    }
    if (!internal::validateSize(ptr, sizeof(uint8_t), countOfIK, rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of MVD model keyframe (IK) detected: ptr=" << static_cast<const void *>(ptr) << " size=" <<  countOfIK << " rest=" << rest);
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of MVD reserved model keyframe detected: ptr=" << static_cast<const void *>(ptr) << " size=" << reserved << " rest=" << rest);
        return false;
    }
    return true;
}

void ModelKeyframe::read(const uint8_t *data)
{
    ModelKeyframeChunk chunk;
    internal::getData(data, chunk);
    setTimeIndex(TimeIndex(chunk.timeIndex));
    setVisible(chunk.visible != 0);
    setShadowEnable(chunk.shadow != 0);
    setAddBlendEnable(chunk.addBlend != 0);
    setPhysicsEnable(chunk.physics != 0);
    setPhysicsStillMode(chunk.physicsStillMode);
    setEdgeWidth(chunk.edgeWidth);
    const uint8_t *bonesOfIKPtr = data + size();
    for (int i = 0; i < m_countOfIKBones; i++) {
        m_bonesOfIK[i] = bonesOfIKPtr[i] != 0 ? true : false;
    }
}

void ModelKeyframe::write(uint8_t *data) const
{
    ModelKeyframeChunk chunk;
    chunk.timeIndex = uint64_t(timeIndex());
    chunk.visible = isVisible();
    chunk.shadow = isShadowEnabled();
    chunk.addBlend = isAddBlendEnabled();
    chunk.physics = isPhysicsEnabled();
    chunk.physicsStillMode = physicsStillMode();
    chunk.edgeWidth = edgeWidth();
    const Color &ec = edgeColor();
    for (int i = 0; i < 4; i++) {
        chunk.edgeColor[i] = uint8_t(ec[i] * 255);
    }
    internal::zerofill(chunk.reserved, sizeof(chunk.reserved));
    internal::writeBytes(&chunk, sizeof(chunk), data);
    for (int i = 0; i < m_countOfIKBones; i++) {
        internal::writeSignedIndex(m_bonesOfIK[i] ? 1 : 0, sizeof(uint8_t), data);
    }
}

size_t ModelKeyframe::estimateSize() const
{
    return size() + sizeof(uint8_t) * m_countOfIKBones;
}

IModelKeyframe *ModelKeyframe::clone() const
{
    ModelKeyframe *keyframe = m_ptr = new ModelKeyframe(m_motionRef, m_countOfIKBones);
    keyframe->setTimeIndex(m_timeIndex);
    keyframe->setLayerIndex(m_layerIndex);
    keyframe->setVisible(m_visible);
    keyframe->setAddBlendEnable(m_addBlend);
    keyframe->setShadowEnable(m_shadow);
    keyframe->setPhysicsEnable(m_physics);
    keyframe->setPhysicsStillMode(m_physicsStillMode);
    keyframe->setEdgeWidth(m_edgeWidth);
    keyframe->setEdgeColor(m_edgeColor);
    keyframe->m_bonesOfIK.copy(m_bonesOfIK);
    m_ptr = 0;
    return keyframe;
}

const Motion *ModelKeyframe::parentMotionRef() const
{
    return m_motionRef;
}

void ModelKeyframe::mergeIKState(const Hash<HashInt, IBone *> &bones) const
{
    const int nbones = m_bonesOfIK.count();
    for (int i = 0; i < nbones; i++) {
        if (IBone *const *bone = bones.find(i)) {
            (*bone)->setInverseKinematicsEnable(m_bonesOfIK[i]);
        }
    }
}

void ModelKeyframe::setIKState(const Hash<HashInt, IBone *> &bones)
{
    const int nbones = bones.count();
    m_bonesOfIK.resize(nbones);
    for (int i = 0; i < nbones; i++) {
        const IBone *const *bone = bones.value(i);
        m_bonesOfIK[i] = (*bone)->isInverseKinematicsEnabled();
    }
    m_countOfIKBones = m_bonesOfIK.count();
}

bool ModelKeyframe::isVisible() const
{
    return m_visible;
}

bool ModelKeyframe::isShadowEnabled() const
{
    return m_shadow;
}

bool ModelKeyframe::isAddBlendEnabled() const
{
    return m_addBlend;
}

bool ModelKeyframe::isPhysicsEnabled() const
{
    return m_physics;
}

uint8_t ModelKeyframe::physicsStillMode() const
{
    return m_physicsStillMode;
}

Scalar ModelKeyframe::edgeWidth() const
{
    return m_edgeWidth;
}

Color ModelKeyframe::edgeColor() const
{
    return m_edgeColor;
}

void ModelKeyframe::setVisible(bool value)
{
    m_visible = value;
}

void ModelKeyframe::setShadowEnable(bool value)
{
    m_shadow = value;
}

void ModelKeyframe::setAddBlendEnable(bool value)
{
    m_addBlend = value;
}

void ModelKeyframe::setPhysicsEnable(bool value)
{
    m_physics = value;
}

void ModelKeyframe::setPhysicsStillMode(uint8_t value)
{
    m_physicsStillMode = value;
}

void ModelKeyframe::setEdgeWidth(const Scalar &value)
{
    m_edgeWidth = value;
}

void ModelKeyframe::setEdgeColor(const Color &value)
{
    m_edgeColor = value;
}

void ModelKeyframe::setName(const IString * /* value */)
{
}

IKeyframe::Type ModelKeyframe::type() const
{
    return kModelKeyframe;
}

} /* namespace mvd */
} /* namespace vpvl2 */

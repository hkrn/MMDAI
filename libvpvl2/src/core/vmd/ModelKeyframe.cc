/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/ModelHelper.h"

#include "vpvl2/vmd/ModelKeyframe.h"

namespace vpvl2
{
namespace vmd
{

#pragma pack(push, 1)

struct ModelKeyframeChunk
{
    uint32 timeIndex;
    uint8 visible;
    uint32 nbones;
};

struct ModelIKStateChunk
{
    uint8 name[internal::kPMDBoneNameSize];
    uint8 enabled;
};

#pragma pack(pop)

const int ModelKeyframe::kNameSize = internal::kPMDBoneNameSize;

bool ModelKeyframe::preparse(uint8 *&ptr, vsize &rest, const int32 nkeyframes)
{
    ModelKeyframeChunk keyframe;
    ModelIKStateChunk state;
    for (int i = 0; i < nkeyframes; i++) {
        if (!internal::getTyped<ModelKeyframeChunk>(ptr, rest, keyframe)) {
            return false;
        }
        const int nbones = keyframe.nbones;
        for (int i = 0; i < nbones; i++) {
            if (!internal::getTyped<ModelIKStateChunk>(ptr, rest, state)) {
                return false;
            }
        }
    }
    return true;
}

ModelKeyframe::ModelKeyframe(IEncoding *encoding)
    : VPVL2_KEYFRAME_INITIALIZE_FIELDS(),
      m_encodingRef(encoding),
      m_visible(false)
{
}

ModelKeyframe::~ModelKeyframe()
{
    VPVL2_KEYFRAME_DESTROY_FIELDS();
    m_states.releaseAll();
    m_encodingRef = 0;
    m_visible = false;
}

void ModelKeyframe::read(const uint8 *data)
{
    ModelKeyframeChunk keyframe;
    ModelIKStateChunk state;
    uint8 *ptr = const_cast<uint8 *>(data);
    internal::getData(ptr, keyframe);
    ptr += sizeof(keyframe);
    m_timeIndex = keyframe.timeIndex;
    m_visible = keyframe.visible != 0;
    const int nbones = keyframe.nbones;
    for (int i = 0; i < nbones; i++) {
        internal::getData(ptr, state);
        IString *name = m_encodingRef->toString(state.name, IString::kShiftJIS, sizeof(state.name));
        if (!m_states.find(name->toHashString())) {
            m_states.insert(name->toHashString(), new IKState(name, state.enabled != 0));
        }
        else {
            delete name;
        }
    }
}

void ModelKeyframe::write(uint8 *data) const
{
    ModelKeyframeChunk keyframe;
    ModelIKStateChunk state;
    const int nbones = m_states.count();
    keyframe.timeIndex = uint32(m_timeIndex);
    keyframe.visible = uint8(m_visible);
    keyframe.nbones = nbones;
    internal::writeBytes(&keyframe, sizeof(keyframe), data);
    for (int i = 0; i < nbones; i++) {
        IKState *const *sptr = m_states.value(i), *s = *sptr;
        uint8 *ptr = m_encodingRef->toByteArray(s->name, IString::kShiftJIS), *sn = state.name;
        internal::writeBytes(ptr, sizeof(state.name), sn);
        state.enabled = s->enabled ? 1 : 0;
        internal::writeBytes(&state, sizeof(state), data);
    }
}

void ModelKeyframe::updateInverseKinematics(IModel *model) const
{
    const int nstates = m_states.count();
    for (int i = 0; i < nstates; i++) {
        const IKState *const *statePtr = m_states.value(i), *state = *statePtr;
        if (IBone *bone = model->findBoneRef(state->name)) {
            bone->setInverseKinematicsEnable(state->enabled);
        }
    }
}

vsize ModelKeyframe::estimateSize() const
{
    vsize size = 0;
    size += sizeof(ModelKeyframeChunk);
    size += m_states.count() * sizeof(ModelIKStateChunk);
    return size;
}

IModelKeyframe *ModelKeyframe::clone() const
{
    ModelKeyframe *keyframe = new ModelKeyframe(m_encodingRef);
    keyframe->setTimeIndex(m_timeIndex);
    keyframe->setVisible(m_visible);
    const int nstates = m_states.count();
    for (int i = 0; i < nstates; i++) {
        const IKState *state = *m_states.value(i);
        const IString *s = state->name;
        keyframe->m_states.insert(s->toHashString(), new IKState(s->clone(), state->enabled));
    }
    return keyframe;
}

IKeyframe::Type ModelKeyframe::type() const
{
    return kModelKeyframe;
}

void ModelKeyframe::setName(const IString * /* value */)
{
}

bool ModelKeyframe::isVisible() const
{
    return m_visible;
}

bool ModelKeyframe::isShadowEnabled() const
{
    return true;
}

bool ModelKeyframe::isAddBlendEnabled() const
{
    return true;
}

bool ModelKeyframe::isPhysicsEnabled() const
{
    return true;
}

bool ModelKeyframe::isInverseKinematicsEnabld(const IBone *value) const
{
    if (value) {
        if (const IString *name = value->name(IEncoding::kDefaultLanguage)) {
            if (IKState *const *state = m_states.find(name->toHashString())) {
                return (*state)->enabled;
            }
        }
    }
    return true;
}

uint8 ModelKeyframe::physicsStillMode() const
{
    return true;
}

IVertex::EdgeSizePrecision ModelKeyframe::edgeWidth() const
{
    return 1;
}

Color ModelKeyframe::edgeColor() const
{
    return kZeroC;
}

void ModelKeyframe::setVisible(bool value)
{
    m_visible = value;
}

void ModelKeyframe::setShadowEnable(bool /* value */)
{
}

void ModelKeyframe::setAddBlendEnable(bool /* value */)
{
}

void ModelKeyframe::setPhysicsEnable(bool /* value */)
{
}

void ModelKeyframe::setPhysicsStillMode(uint8 /* value */)
{
}

void ModelKeyframe::setEdgeWidth(const IVertex::EdgeSizePrecision & /* value */)
{
}

void ModelKeyframe::setEdgeColor(const Color & /* value */)
{
}

void ModelKeyframe::setInverseKinematicsEnable(IBone *bone, bool value)
{
    bone->setInverseKinematicsEnable(value);
}

} /* namespace vmd */
} /* namespace vpvl2 */

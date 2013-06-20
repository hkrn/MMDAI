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
#include "vpvl2/internal/MotionHelper.h"

#include "vpvl2/vmd/ModelAnimation.h"
#include "vpvl2/vmd/ModelKeyframe.h"

namespace vpvl2
{
namespace vmd
{

ModelAnimation::ModelAnimation(IModel *modelRef, IEncoding *encodingRef)
    : BaseAnimation(),
      m_encodingRef(encodingRef),
      m_modelRef(modelRef)
{
}

ModelAnimation::~ModelAnimation()
{
    m_modelRef = 0;
    m_encodingRef = 0;
}

void ModelAnimation::read(const uint8 *data, int size)
{
    uint8 *ptr = const_cast<uint8 *>(data);
    m_keyframes.reserve(size);
    for (int i = 0; i < size; i++) {
        ModelKeyframe *keyframe = m_keyframes.append(new ModelKeyframe(m_encodingRef));
        keyframe->read(ptr);
        ptr += keyframe->estimateSize();
    }
}

void ModelAnimation::seek(const IKeyframe::TimeIndex &timeIndexAt)
{
    if (m_modelRef && m_keyframes.count() > 0) {
        int fromIndex, toIndex;
        internal::MotionHelper::findKeyframeIndices(timeIndexAt, m_currentTimeIndex, m_lastTimeIndex, fromIndex, toIndex, m_keyframes);
        const ModelKeyframe *keyframeFrom = findKeyframeAt(fromIndex);
        keyframeFrom->updateInverseKinematics(m_modelRef);
        m_modelRef->setVisible(keyframeFrom->isVisible());
        m_previousTimeIndex = m_currentTimeIndex;
        m_currentTimeIndex = timeIndexAt;
    }
}

void ModelAnimation::setParentModelRef(IModel *model)
{
    m_modelRef = model;
}

vsize ModelAnimation::estimateSize() const
{
    vsize size = 0;
    const int nkeyframes = m_keyframes.count();
    for (int i = 0; i < nkeyframes; i++) {
        IKeyframe *keyframe = m_keyframes[i];
        size += keyframe->estimateSize();
    }
    return size;
}

ModelKeyframe *ModelAnimation::findKeyframeAt(int i) const
{
    return internal::checkBound(i, 0, m_keyframes.count()) ? reinterpret_cast<ModelKeyframe *>(m_keyframes[i]) : 0;
}

ModelKeyframe *ModelAnimation::findKeyframe(const IKeyframe::TimeIndex &timeIndex) const
{
    int index = findKeyframeIndex(timeIndex, m_keyframes);
    return index != -1 ? reinterpret_cast<ModelKeyframe *>(m_keyframes[index]) : 0;
}

} /* namespace vmd */
} /* namespace vpvl2 */

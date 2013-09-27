/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2013  hkrn

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

#include "vpvl2/vmd/ProjectAnimation.h"
#include "vpvl2/vmd/ProjectKeyframe.h"

namespace vpvl2
{
namespace vmd
{

ProjectAnimation::ProjectAnimation(IEncoding *encoding)
    : BaseAnimation(),
      m_encodingRef(encoding),
      m_modelRef(0),
      m_shadowDistance(0)
{
}

ProjectAnimation::~ProjectAnimation()
{
    m_modelRef = 0;
}

void ProjectAnimation::read(const uint8 *data, int size)
{
    uint8 *ptr = const_cast<uint8 *>(data);
    m_keyframes.reserve(size);
    for (int i = 0; i < size; i++) {
        ProjectKeyframe *keyframe = m_keyframes.append(new ProjectKeyframe(m_encodingRef));
        keyframe->read(ptr);
        ptr += keyframe->estimateSize();
    }
}

void ProjectAnimation::seek(const IKeyframe::TimeIndex &timeIndexAt)
{
    int fromIndex, toIndex;
    internal::MotionHelper::findKeyframeIndices(timeIndexAt, m_currentTimeIndex, m_lastTimeIndex, fromIndex, toIndex, m_keyframes);
    const ProjectKeyframe *keyframeFrom = findKeyframeAt(fromIndex), *keyframeTo = findKeyframeAt(toIndex);
    const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), timeIndexTo = keyframeTo->timeIndex();
    const IMorph::WeightPrecision &distanceFrom = keyframeFrom->shadowDistance();
    const IMorph::WeightPrecision &distanceTo = keyframeTo->shadowDistance();
    if (timeIndexFrom != timeIndexTo) {
        const IKeyframe::SmoothPrecision &w = (m_currentTimeIndex - timeIndexFrom) / (timeIndexTo - timeIndexFrom);
        m_shadowDistance = internal::MotionHelper::lerp(distanceFrom, distanceTo, w);
    }
    else {
        m_shadowDistance = distanceFrom;
    }
    m_previousTimeIndex = m_currentTimeIndex;
    m_currentTimeIndex = timeIndexAt;
}

void ProjectAnimation::update()
{
    int nkeyframes = m_keyframes.count();
    if (nkeyframes > 0) {
        m_keyframes.sort(internal::MotionHelper::KeyframeTimeIndexPredication());
        m_durationTimeIndex = m_keyframes[m_keyframes.count() - 1]->timeIndex();
    }
    else {
        m_durationTimeIndex = 0;
    }
}

ProjectKeyframe *ProjectAnimation::findKeyframe(const IKeyframe::TimeIndex &timeIndex) const
{
    int index = findKeyframeIndex(timeIndex, m_keyframes);
    return index != -1 ? reinterpret_cast<ProjectKeyframe *>(m_keyframes[index]) : 0;
}

ProjectKeyframe *ProjectAnimation::findKeyframeAt(int i) const
{
    return internal::checkBound(i, 0, m_keyframes.count()) ? reinterpret_cast<ProjectKeyframe *>(m_keyframes[i]) : 0;
}

} /* namespace vmd */
} /* namespace vpvl2 */

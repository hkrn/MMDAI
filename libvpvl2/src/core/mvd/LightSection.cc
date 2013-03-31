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

#include "vpvl2/mvd/LightKeyframe.h"
#include "vpvl2/mvd/LightSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct LightSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int reserved2;
};

#pragma pack(pop)

class LightSection::PrivateContext : public BaseSectionContext {
public:
    PrivateContext()
    {
    }
    ~PrivateContext() {
    }

    void seek(const IKeyframe::TimeIndex &timeIndex) {
        if (keyframes.count() > 0) {
            int fromIndex, toIndex;
            IKeyframe::TimeIndex currentTimeIndex;
            findKeyframeIndices(timeIndex, currentTimeIndex, fromIndex, toIndex);
            const LightKeyframe *keyframeFrom = reinterpret_cast<const LightKeyframe *>(keyframes[fromIndex]),
                    *keyframeTo = reinterpret_cast<const LightKeyframe *>(keyframes[toIndex]);
            const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), &timeIndexTo = keyframeTo->timeIndex();
            const Vector3 &colorFrom = keyframeFrom->color(), &colorTo = keyframeTo->color();
            const Vector3 &directionFrom = keyframeFrom->direction(), &directionTo = keyframeTo->direction();
            if (timeIndexFrom != timeIndexTo && timeIndexFrom < currentTimeIndex) {
                if (timeIndexTo <= currentTimeIndex) {
                    color = colorTo;
                    direction = directionTo;
                }
                else {
                    const IKeyframe::SmoothPrecision &w = calculateWeight(currentTimeIndex, timeIndexFrom, timeIndexTo);;
                    color = colorFrom.lerp(colorTo, Scalar(w));
                    direction = directionFrom.lerp(directionTo, Scalar(w));
                }
            }
            else {
                color = colorFrom;
                direction = directionFrom;
            }
        }
    }

    Vector3 color;
    Vector3 direction;
};

LightSection::LightSection(const Motion *motionRef)
    : BaseSection(motionRef),
      m_context(0)
{
    m_context = new LightSection::PrivateContext();
}

LightSection::~LightSection()
{
    release();
}

bool LightSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    LightSectionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of MVDLightSection header detected: " << rest);
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    if (!internal::validateSize(ptr, header.reserved2, rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of MVDLightSection header reserved detected: size=" << header.reserved2 << " rest=" << rest);
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const size_t reserved = header.sizeOfKeyframe - LightKeyframe::size();
    VPVL2_LOG(VLOG(2) << "MVDLightSection(Header): nkeyframes=" << nkeyframes);
    VPVL2_LOG(VLOG(2) << "MVDLightSection(Header): sizeofKeyframe=" << header.sizeOfKeyframe);
    VPVL2_LOG(VLOG(2) << "MVDLightSection(Header): reserved1=" << reserved);
    VPVL2_LOG(VLOG(2) << "MVDLightSection(Header): reserved2=" << header.reserved2);
    for (int i = 0; i < nkeyframes; i++) {
        if (!LightKeyframe::preparse(ptr, rest, reserved, info)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid size of MVDLightSection key detected: index=" << i << " rest=" << rest);
            return false;
        }
    }
    return true;
}

void LightSection::release()
{
    delete m_context;
    m_context = 0;
}

void LightSection::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    LightSectionHeader header;
    internal::getData(ptr, header);
    const size_t sizeOfKeyframe = header.sizeOfKeyframe;
    const int nkeyframes = header.countOfKeyframes;
    ptr += sizeof(header) + header.reserved2;
    m_context->keyframes.reserve(nkeyframes);
    for (int i = 0; i < nkeyframes; i++) {
        LightKeyframe *keyframe = m_context->keyframes.append(new LightKeyframe(m_motionRef));
        keyframe->read(ptr);
        setMaxTimeIndex(keyframe);
        ptr += sizeOfKeyframe;
    }
    m_context->keyframes.sort(KeyframeTimeIndexPredication());
}

void LightSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    saveCurrentTimeIndex(timeIndex);
}

void LightSection::write(uint8_t * /* data */) const
{
}

size_t LightSection::estimateSize() const
{
    return 0;
}

size_t LightSection::countKeyframes() const
{
    return m_context->keyframes.count();
}

void LightSection::addKeyframe(IKeyframe *keyframe)
{
    m_context->keyframes.append(keyframe);
    setMaxTimeIndex(keyframe);
}

void LightSection::deleteKeyframe(IKeyframe *&keyframe)
{
    m_context->keyframes.remove(keyframe);
    delete keyframe;
    keyframe = 0;
}

void LightSection::getKeyframes(const IKeyframe::TimeIndex & /* timeIndex */,
                                const IKeyframe::LayerIndex & /* layerIndex */,
                                Array<IKeyframe *> & /* keyframes */)
{
}

IKeyframe::LayerIndex LightSection::countLayers() const
{
    return 1;
}

ILightKeyframe *LightSection::findKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                           const IKeyframe::LayerIndex &layerIndex) const
{
    const int nkeyframes = m_context->keyframes.count();
    for (int i = 0; i < nkeyframes; i++) {
        LightKeyframe *keyframe = reinterpret_cast<LightKeyframe *>(m_context->keyframes[i]);
        if (keyframe->timeIndex() == timeIndex && keyframe->layerIndex() == layerIndex) {
            return keyframe;
        }
    }
    return 0;
}

ILightKeyframe *LightSection::findKeyframeAt(int index) const
{
    if (internal::checkBound(index, 0, m_context->keyframes.count())) {
        LightKeyframe *keyframe = reinterpret_cast<LightKeyframe *>(m_context->keyframes[index]);
        return keyframe;
    }
    return 0;
}

Vector3 LightSection::color() const
{
    return m_context->color;
}

Vector3 LightSection::direction() const
{
    return m_context->direction;
}

} /* namespace mvd */
} /* namespace vpvl2 */

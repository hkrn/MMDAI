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
        : keyframePtr(0)
    {
    }
    ~PrivateContext() {
        delete keyframePtr;
        keyframePtr = 0;
    }
    LightKeyframe *keyframePtr;
};

LightSection::LightSection(NameListSection *nameListSectionRef)
    : BaseSection(nameListSectionRef),
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
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    if (!internal::validateSize(ptr, header.reserved2, rest)) {
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const size_t reserved = header.sizeOfKeyframe - LightKeyframe::size();
    for (int i = 0; i < nkeyframes; i++) {
        if (!LightKeyframe::preparse(ptr, rest, reserved, info)) {
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
        LightKeyframe *keyframe = m_context->keyframePtr = new LightKeyframe();
        keyframe->read(ptr);
        addKeyframe0(keyframe, m_context->keyframes);
        ptr += sizeOfKeyframe;
    }
    m_context->keyframePtr = 0;
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
    addKeyframe0(keyframe, m_context->keyframes);
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

} /* namespace mvd */
} /* namespace vpvl2 */

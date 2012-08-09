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
#include "vpvl2/mvd/BoneSection.h"
#include "vpvl2/mvd/NameListSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct BoneSectionHeader {
    int key;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int countOfLayers;
};

#pragma pack(pop)

BoneSection::BoneSection(NameListSection *nameListSectionRef)
    : BaseSection(nameListSectionRef),
      m_keyframeListPtr(0),
      m_keyframePtr(0)
{
}

BoneSection::~BoneSection()
{
    release();
}

bool BoneSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    const BoneSectionHeader &header = *reinterpret_cast<const BoneSectionHeader *>(ptr);
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, sizeof(uint8_t), header.countOfLayers, rest)) {
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const size_t reserved = header.sizeOfKeyframe - BoneKeyframe::size();
    for (int i = 0; i < nkeyframes; i++) {
        if (!BoneKeyframe::preparse(ptr, rest, reserved, info)) {
            return false;
        }
    }
    return true;
}

void BoneSection::release()
{
    if (m_keyframeListPtr) {
        m_keyframeListPtr->releaseAll();
        delete m_keyframeListPtr;
        m_keyframeListPtr = 0;
    }
    delete m_keyframePtr;
    m_keyframePtr = 0;
    const int nitems = m_allKeyframes.count();
    for (int i = 0; i < nitems; i++) {
        BoneKeyframeList **keyframes = const_cast<BoneKeyframeList **>(m_allKeyframes.value(i));
        (*keyframes)->releaseAll();
    }
    m_allKeyframes.releaseAll();
}

void BoneSection::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    const BoneSectionHeader &header = *reinterpret_cast<const BoneSectionHeader *>(ptr);
    const size_t sizeOfKeyframe = header.sizeOfKeyframe;
    const int nkeyframes = header.countOfKeyframes;
    ptr += sizeof(header) + sizeof(uint8_t) * header.countOfLayers;
    m_keyframeListPtr = new BoneKeyframeList();
    for (int i = 0; i < nkeyframes; i++) {
        m_keyframePtr = new BoneKeyframe(m_nameListSectionRef);
        m_keyframePtr->read(ptr);
        m_keyframeListPtr->add(m_keyframePtr);
        ptr += sizeOfKeyframe;
    }
    m_allKeyframes.insert(header.key, m_keyframeListPtr);
    m_keyframeListPtr = 0;
    m_keyframePtr = 0;
}

void BoneSection::write(uint8_t *data) const
{
}

size_t BoneSection::estimateSize() const
{
    return 0;
}

} /* namespace mvd */
} /* namespace vpvl2 */

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

#include "vpvl2/mvd/ModelKeyframe.h"
#include "vpvl2/mvd/ModelSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct ModelSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int sizeOfIKBones;
    int countOfIKBones;
};

#pragma pack(pop)

class ModelSection::PrivateContext : public BaseSectionContext {
public:
    Array<IBone *> bones;
};

ModelSection::ModelSection(IModel *model, NameListSection *nameListSectionRef, size_t align)
    : BaseSection(nameListSectionRef),
      m_modelRef(model),
      m_keyframePtr(0),
      m_contextPtr(0),
      m_adjustAlighment(align),
      m_countOfIKBones(0)
{
}

ModelSection::~ModelSection()
{
    release();
}

bool ModelSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    const ModelSectionHeader &header = *reinterpret_cast<const ModelSectionHeader *>(ptr);
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        return false;
    }
    const int countOfIK = header.countOfIKBones;
    if (!internal::validateSize(ptr, sizeof(int), countOfIK, rest)) {
        return false;
    }
    const int sizeOfIK = header.sizeOfIKBones;
    if (!internal::validateSize(ptr, sizeOfIK - sizeof(int) * (countOfIK + 1), rest)) {
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const size_t adjust = info.adjustAlignment;
    const size_t reserved = header.sizeOfKeyframe - ((ModelKeyframe::size() - adjust) + countOfIK);
    for (int i = 0; i < nkeyframes; i++) {
        if (!ModelKeyframe::preparse(ptr, rest, reserved, countOfIK, info)) {
            return false;
        }
    }
    return true;
}

void ModelSection::release()
{
    delete m_keyframePtr;
    m_keyframePtr = 0;
    delete m_contextPtr;
    m_contextPtr = 0;
    m_adjustAlighment = 0;
    m_countOfIKBones = 0;
}

void ModelSection::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    const ModelSectionHeader &header = *reinterpret_cast<const ModelSectionHeader *>(ptr);
    const size_t sizeOfkeyframe = header.sizeOfKeyframe;
    const int nkeyframes = header.countOfKeyframes;
    const int nIKBones = header.countOfIKBones;
    delete m_contextPtr;
    m_contextPtr = new PrivateContext();
    m_contextPtr->bones.reserve(nIKBones);
    m_countOfIKBones = nIKBones;
    for (int i = 0; i < nIKBones; i++) {
        int boneID = *reinterpret_cast<int *>(ptr);
        (void) boneID;
        m_contextPtr->bones.add(0);
        ptr += sizeof(int);
    }
    ptr += header.sizeOfIKBones - sizeof(int) * (nIKBones + 1);
    m_contextPtr->keyframes = new PrivateContext::KeyframeCollection();
    m_contextPtr->keyframes->reserve(nkeyframes);
    for (int i = 0; i < nkeyframes; i++) {
        m_keyframePtr = new ModelKeyframe(m_nameListSectionRef, nIKBones);
        m_keyframePtr->read(ptr);
        m_contextPtr->keyframes->add(m_keyframePtr);
        btSetMax(m_maxTimeIndex, m_keyframePtr->timeIndex());
        ptr += sizeOfkeyframe;
    }
    m_keyframePtr = 0;
}

void ModelSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    if (m_contextPtr && m_modelRef) {
    }
    saveCurrentTimeIndex(timeIndex);
}

void ModelSection::write(uint8_t * /* data */) const
{
}

size_t ModelSection::estimateSize() const
{
    size_t size = 0;
    size += sizeof(ModelSectionHeader);
    if (m_contextPtr) {
        size += m_countOfIKBones * sizeof(int);
        size += m_adjustAlighment;
        const PrivateContext::KeyframeCollection *keyframes = m_contextPtr->keyframes;
        const int nkeyframes = keyframes->count();
        for (int i = 0; i < nkeyframes; i++) {
            const IKeyframe *keyframe = keyframes->at(i);
            size += keyframe->estimateSize();
        }
    }
    return size;
}

size_t ModelSection::countKeyframes() const
{
    return m_contextPtr ? m_contextPtr->keyframes->count() : 0;
}

} /* namespace mvd */
} /* namespace vpvl2 */

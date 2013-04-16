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

#include "vpvl2/mvd/MorphKeyframe.h"
#include "vpvl2/mvd/MorphSection.h"
#include "vpvl2/mvd/NameListSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct MorphSecionHeader {
    int key;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int reserved;
};

#pragma pack(pop)

class MorphSection::PrivateContext : public BaseSectionContext {
public:
    IMorph *morphRef;
    IMorph::WeightPrecision weight;
    PrivateContext()
        : morphRef(0),
          weight(0)
    {
    }
    ~PrivateContext() {
        morphRef = 0;
        weight = 0;
    }
    void seek(const IKeyframe::TimeIndex &timeIndex) {
        if (morphRef && keyframes.count() > 0) {
            int fromIndex, toIndex;
            IKeyframe::TimeIndex currentTimeIndex;
            findKeyframeIndices(timeIndex, currentTimeIndex, fromIndex, toIndex);
            const MorphKeyframe *keyframeFrom = reinterpret_cast<const MorphKeyframe *>(keyframes[fromIndex]),
                    *keyframeTo = reinterpret_cast<const MorphKeyframe *>(keyframes[toIndex]);
            const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), &timeIndexTo = keyframeTo->timeIndex();
            const IMorph::WeightPrecision &weightFrom = keyframeFrom->weight(), &weightTo = keyframeTo->weight();
            if (timeIndexFrom != timeIndexTo && timeIndexFrom < currentTimeIndex) {
                if (timeIndexTo <= currentTimeIndex) {
                    weight = weightTo;
                }
                else {
                    const IKeyframe::SmoothPrecision &w = calculateWeight(currentTimeIndex, timeIndexFrom, timeIndexTo);;
                    const Motion::InterpolationTable &tableForWeight = keyframeTo->tableForWeight();
                    if (tableForWeight.linear) {
                        weight = internal::lerp(weightFrom, weightTo, w);
                    }
                    else {
                        const IKeyframe::SmoothPrecision &weight2 = calculateInterpolatedWeight(tableForWeight, w);
                        weight = internal::lerp(weightFrom, weightTo, weight2);
                    }
                }
            }
            else {
                weight = weightFrom;
            }
            morphRef->setWeight(weight);
        }
    }
};

MorphSection::MorphSection(const Motion *motionRef, IModel *modelRef)
    : BaseSection(motionRef),
      m_modelRef(modelRef)
{
}

MorphSection::~MorphSection()
{
    release();
}

bool MorphSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    MorphSecionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of MVDMorphSection header detected: " << rest);
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    if (!internal::validateSize(ptr, header.reserved, rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of MVDMorphSection header reserved detected: size=" << header.reserved << " rest=" << rest);
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const size_t reserved = header.sizeOfKeyframe - MorphKeyframe::size();
    VPVL2_LOG(VLOG(2) << "MVDMorphSection(Header): key=" << header.key);
    VPVL2_LOG(VLOG(2) << "MVDMorphSection(Header): nkeyframes=" << nkeyframes);
    VPVL2_LOG(VLOG(2) << "MVDMorphSection(Header): sizeofKeyframe=" << header.sizeOfKeyframe);
    VPVL2_LOG(VLOG(2) << "MVDMorphSection(Header): reserved=" << reserved);
    for (int i = 0; i < nkeyframes; i++) {
        if (!MorphKeyframe::preparse(ptr, rest, reserved, info)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid size of MVDMorphSection key detected: index=" << i << " rest=" << rest);
            return false;
        }
    }
    return true;
}

void MorphSection::release()
{
    BaseSection::release();
    m_name2contexts.releaseAll();
    m_allKeyframeRefs.clear();
    m_context2names.clear();
}

void MorphSection::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    MorphSecionHeader header;
    internal::getData(ptr, header);
    const size_t sizeOfKeyframe = header.sizeOfKeyframe;
    const int nkeyframes = header.countOfKeyframes;
    const int key = header.key;
    const IString *name = m_nameListSectionRef->value(key);
    PrivateContext *contextPtr = m_name2contexts.insert(key, new PrivateContext());
    contextPtr->keyframes.reserve(nkeyframes);
    ptr += sizeof(header) + header.reserved;
    for (int i = 0; i < nkeyframes; i++) {
        MorphKeyframe *keyframePtr = contextPtr->keyframes.append(new MorphKeyframe(m_motionRef));
        keyframePtr->read(ptr);
        keyframePtr->setName(name);
        addKeyframe0(keyframePtr);
        ptr += sizeOfKeyframe;
    }
    contextPtr->keyframes.sort(KeyframeTimeIndexPredication());
    contextPtr->morphRef = m_modelRef ? m_modelRef->findMorph(m_nameListSectionRef->value(key)) : 0;
    m_context2names.insert(contextPtr, key);
}

void MorphSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    if (m_modelRef) {
        m_modelRef->resetVertices();
        const int ncontexts = m_name2contexts.count();
        for (int i = 0; i < ncontexts; i++) {
            if (PrivateContext *const *context = m_name2contexts.value(i)) {
                (*context)->seek(timeIndex);
            }
        }
    }
    saveCurrentTimeIndex(timeIndex);
}

void MorphSection::setParentModel(IModel *model)
{
    m_modelRef = model;
    if (model) {
        const int ncontexts = m_name2contexts.count();
        for (int i = 0; i < ncontexts; i++) {
            if (PrivateContext *const *context = m_name2contexts.value(i)) {
                PrivateContext *contextRef = *context;
                if (const int *key = m_context2names.find(contextRef)) {
                    IMorph *morph = model->findMorph(m_nameListSectionRef->value(*key));
                    contextRef->morphRef = morph;
                }
                else {
                    contextRef->morphRef = 0;
                }
            }
        }
    }
}

void MorphSection::write(uint8_t *data) const
{
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        const PrivateContext *const *context = m_name2contexts.value(i);
        const PrivateContext *contextRef = *context;
        const IMorph *morph = contextRef->morphRef;
        if (morph) {
            const PrivateContext::KeyframeCollection &keyframes = contextRef->keyframes;
            const int nkeyframes = keyframes.count();
            Motion::SectionTag tag;
            tag.type = Motion::kMorphSection;
            tag.minor = 0;
            internal::writeBytes(reinterpret_cast<const uint8_t *>(&tag), sizeof(tag), data);
            MorphSecionHeader header;
            header.countOfKeyframes = nkeyframes;
            header.key = m_nameListSectionRef->key(morph->name());
            header.reserved = 0;
            header.sizeOfKeyframe = MorphKeyframe::size();
            internal::writeBytes(reinterpret_cast<const uint8_t *>(&header) ,sizeof(header), data);
            for (int i = 0 ; i < nkeyframes; i++) {
                const IKeyframe *keyframe = keyframes[i];
                keyframe->write(data);
                data += keyframe->estimateSize();
            }
        }
    }
}

size_t MorphSection::estimateSize() const
{
    size_t size = 0;
    const int ncontexts = m_name2contexts.count();
    for (int i = 0; i < ncontexts; i++) {
        const PrivateContext *const *context = m_name2contexts.value(i);
        const PrivateContext *contextPtr = *context;
        if (contextPtr->morphRef) {
            const PrivateContext::KeyframeCollection &keyframes = contextPtr->keyframes;
            const int nkeyframes = keyframes.count();
            size += sizeof(Motion::SectionTag);
            size += sizeof(MorphSecionHeader);
            for (int i = 0 ; i < nkeyframes; i++) {
                const IKeyframe *keyframe = keyframes[i];
                size += keyframe->estimateSize();
            }
        }
    }
    return size;
}

size_t MorphSection::countKeyframes() const
{
    return m_allKeyframeRefs.count();
}

void MorphSection::addKeyframe(IKeyframe *keyframe)
{
    int key = m_nameListSectionRef->key(keyframe->name());
    PrivateContext *const *context = m_name2contexts.find(key), *contextPtr = 0;
    if (context) {
        contextPtr = *context;
        addKeyframe0(contextPtr->keyframes.append(keyframe));
    }
    else if (m_modelRef) {
        contextPtr = m_name2contexts.insert(key, new PrivateContext());
        contextPtr->morphRef = m_modelRef->findMorph(keyframe->name());
        addKeyframe0(contextPtr->keyframes.append(keyframe));
        m_context2names.insert(contextPtr, key);
    }
}

void MorphSection::deleteKeyframe(IKeyframe *&keyframe)
{
    int key = m_nameListSectionRef->key(keyframe->name());
    PrivateContext *const *context = m_name2contexts.find(key);
    if (context) {
        PrivateContext *contextPtr = *context;
        contextPtr->keyframes.remove(keyframe);
        m_allKeyframeRefs.remove(keyframe);
        if (contextPtr->keyframes.count() == 0) {
            m_name2contexts.remove(key);
            m_context2names.remove(contextPtr);
            delete contextPtr;
        }
        delete keyframe;
        keyframe = 0;
    }
}

void MorphSection::getKeyframes(const IKeyframe::TimeIndex & /* timeIndex */,
                                const IKeyframe::LayerIndex & /* layerIndex */,
                                Array<IKeyframe *> & /* keyframes */) const
{
}

void MorphSection::getAllKeyframes(Array<IKeyframe *> &keyframes) const
{
    keyframes.copy(m_allKeyframeRefs);
}

void MorphSection::setAllKeyframes(const Array<IKeyframe *> &value)
{
    release();
    const int nkeyframes = value.count();
    for (int i = 0; i < nkeyframes; i++) {
        IKeyframe *keyframe = value[i];
        if (keyframe && keyframe->type() == IKeyframe::kMorphKeyframe) {
            addKeyframe0(keyframe);
        }
    }
}

IKeyframe::LayerIndex MorphSection::countLayers(const IString * /* name */) const
{
    return 1;
}

IMorphKeyframe *MorphSection::findKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                           const IString *name,
                                           const IKeyframe::LayerIndex &layerIndex) const
{
    PrivateContext *const *context = m_name2contexts.find(m_nameListSectionRef->key(name));
    if (context) {
        const PrivateContext::KeyframeCollection &keyframes = (*context)->keyframes;
        const int nkeyframes = keyframes.count();
        for (int i = 0; i < nkeyframes; i++) {
            MorphKeyframe *keyframe = reinterpret_cast<MorphKeyframe *>(keyframes[i]);
            if (keyframe->timeIndex() == timeIndex && keyframe->layerIndex() == layerIndex) {
                return keyframe;
            }
        }
    }
    return 0;
}

IMorphKeyframe *MorphSection::findKeyframeAt(int index) const
{
    if (internal::checkBound(index, 0, m_allKeyframeRefs.count())) {
        MorphKeyframe *keyframe = reinterpret_cast<MorphKeyframe *>(m_allKeyframeRefs[index]);
        return keyframe;
    }
    return 0;
}

void MorphSection::addKeyframe0(IKeyframe *keyframe)
{
    BaseSection::setMaxTimeIndex(keyframe);
    m_allKeyframeRefs.append(keyframe);
}


} /* namespace mvd */
} /* namespace vpvl2 */

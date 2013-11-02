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

#include "vpvl2/mvd/MorphKeyframe.h"
#include "vpvl2/mvd/MorphSection.h"
#include "vpvl2/mvd/NameListSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct MorphSecionHeader {
    int32 key;
    int32 sizeOfKeyframe;
    int32 countOfKeyframes;
    int32 reserved;
};

#pragma pack(pop)

class MorphAnimationTrack : public BaseAnimationTrack {
public:
    IMorph *morphRef;
    IMorph::WeightPrecision weight;
    MorphAnimationTrack()
        : morphRef(0),
          weight(0)
    {
    }
    ~MorphAnimationTrack() {
        morphRef = 0;
        weight = 0;
    }
    void seek(const IKeyframe::TimeIndex &timeIndex) {
        if (morphRef && keyframes.count() > 0) {
            int fromIndex, toIndex;
            IKeyframe::TimeIndex currentTimeIndex;
            internal::MotionHelper::findKeyframeIndices(timeIndex, currentTimeIndex, m_lastIndex, fromIndex, toIndex, keyframes);
            const MorphKeyframe *keyframeFrom = reinterpret_cast<const MorphKeyframe *>(keyframes[fromIndex]),
                    *keyframeTo = reinterpret_cast<const MorphKeyframe *>(keyframes[toIndex]);
            const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), &timeIndexTo = keyframeTo->timeIndex();
            const IMorph::WeightPrecision &weightFrom = keyframeFrom->weight(), &weightTo = keyframeTo->weight();
            if (timeIndexFrom != timeIndexTo && timeIndexFrom < currentTimeIndex) {
                if (timeIndexTo <= currentTimeIndex) {
                    weight = weightTo;
                }
                else {
                    const IKeyframe::SmoothPrecision &w = internal::MotionHelper::calculateWeight(currentTimeIndex, timeIndexFrom, timeIndexTo);;
                    const internal::InterpolationTable &tableForWeight = keyframeTo->tableForWeight();
                    if (tableForWeight.linear) {
                        weight = internal::MotionHelper::lerp(weightFrom, weightTo, w);
                    }
                    else {
                        const IKeyframe::SmoothPrecision &weight2 = internal::MotionHelper::calculateInterpolatedWeight(tableForWeight, w);
                        weight = internal::MotionHelper::lerp(weightFrom, weightTo, weight2);
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

struct MorphSection::PrivateContext {
    PrivateContext(IModel *modelRef)
        : modelRef(modelRef)
    {
    }
    ~PrivateContext() {
        release();
        modelRef = 0;
    }
    void release() {
        name2tracks.releaseAll();
        allKeyframeRefs.clear();
        track2names.clear();
    }

    IModel *modelRef;
    Array<IKeyframe *> allKeyframeRefs;
    PointerHash<HashInt, MorphAnimationTrack> name2tracks;
    Hash<HashPtr, int> track2names;
};

MorphSection::MorphSection(const Motion *motionRef, IModel *modelRef)
    : BaseSection(motionRef),
      m_context(new PrivateContext(modelRef))
{
}

MorphSection::~MorphSection()
{
    release();
    internal::deleteObject(m_context);
}

bool MorphSection::preparse(uint8 *&ptr, vsize &rest, Motion::DataInfo &info)
{
    MorphSecionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVDMorphSection header detected: " << rest);
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    if (!internal::validateSize(ptr, header.reserved, rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVDMorphSection header reserved detected: size=" << header.reserved << " rest=" << rest);
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const vsize reserved = header.sizeOfKeyframe - MorphKeyframe::size();
    VPVL2_VLOG(2, "MVDMorphSection(Header): key=" << header.key);
    VPVL2_VLOG(2, "MVDMorphSection(Header): nkeyframes=" << nkeyframes);
    VPVL2_VLOG(2, "MVDMorphSection(Header): sizeofKeyframe=" << header.sizeOfKeyframe);
    VPVL2_VLOG(2, "MVDMorphSection(Header): reserved=" << reserved);
    for (int i = 0; i < nkeyframes; i++) {
        if (!MorphKeyframe::preparse(ptr, rest, reserved, info)) {
            VPVL2_LOG(WARNING, "Invalid size of MVDMorphSection key detected: index=" << i << " rest=" << rest);
            return false;
        }
    }
    return true;
}

void MorphSection::release()
{
    BaseSection::release();
    m_context->release();
}

void MorphSection::read(const uint8 *data)
{
    uint8 *ptr = const_cast<uint8 *>(data);
    MorphSecionHeader header;
    internal::getData(ptr, header);
    const vsize sizeOfKeyframe = header.sizeOfKeyframe;
    const int nkeyframes = header.countOfKeyframes;
    const int key = header.key;
    const IString *name = m_nameListSectionRef->value(key);
    MorphAnimationTrack *trackPtr = m_context->name2tracks.insert(key, new MorphAnimationTrack());
    trackPtr->keyframes.reserve(nkeyframes);
    ptr += sizeof(header) + header.reserved;
    for (int i = 0; i < nkeyframes; i++) {
        MorphKeyframe *keyframePtr = trackPtr->keyframes.append(new MorphKeyframe(m_motionRef));
        keyframePtr->read(ptr);
        keyframePtr->setName(name);
        BaseSection::setDuration(keyframePtr);
        m_context->allKeyframeRefs.append(keyframePtr);
        ptr += sizeOfKeyframe;
    }
    trackPtr->keyframes.sort(internal::MotionHelper::KeyframeTimeIndexPredication());
    trackPtr->morphRef = m_context->modelRef ? m_context->modelRef->findMorphRef(m_nameListSectionRef->value(key)) : 0;
    m_context->track2names.insert(trackPtr, key);
}

void MorphSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    if (m_context->modelRef) {
        const int ntracks = m_context->name2tracks.count();
        for (int i = 0; i < ntracks; i++) {
            if (MorphAnimationTrack *const *track = m_context->name2tracks.value(i)) {
                (*track)->seek(timeIndex);
            }
        }
    }
    saveCurrentTimeIndex(timeIndex);
}

void MorphSection::setParentModel(IModel *model)
{
    m_context->modelRef = model;
    if (model) {
        const int ntracks = m_context->name2tracks.count();
        for (int i = 0; i < ntracks; i++) {
            if (MorphAnimationTrack *const *track = m_context->name2tracks.value(i)) {
                MorphAnimationTrack *trackRef = *track;
                if (const int *key = m_context->track2names.find(trackRef)) {
                    IMorph *morph = model->findMorphRef(m_nameListSectionRef->value(*key));
                    trackRef->morphRef = morph;
                }
                else {
                    trackRef->morphRef = 0;
                }
            }
        }
    }
}

void MorphSection::write(uint8 *data) const
{
    const int ntracks = m_context->name2tracks.count();
    for (int i = 0; i < ntracks; i++) {
        const MorphAnimationTrack *const *track = m_context->name2tracks.value(i);
        const MorphAnimationTrack *trackRef = *track;
        if (const IMorph *morph = trackRef->morphRef) {
            const MorphAnimationTrack::KeyframeCollection &keyframes = trackRef->keyframes;
            const int nkeyframes = keyframes.count();
            Motion::SectionTag tag;
            tag.type = Motion::kMorphSection;
            tag.minor = 0;
            internal::writeBytes(&tag, sizeof(tag), data);
            MorphSecionHeader header;
            header.countOfKeyframes = nkeyframes;
            header.key = m_nameListSectionRef->key(morph->name(IEncoding::kDefaultLanguage));
            header.reserved = 0;
            header.sizeOfKeyframe = int32(MorphKeyframe::size());
            internal::writeBytes(&header ,sizeof(header), data);
            for (int i = 0 ; i < nkeyframes; i++) {
                const IKeyframe *keyframe = keyframes[i];
                keyframe->write(data);
                data += keyframe->estimateSize();
            }
        }
    }
}

vsize MorphSection::estimateSize() const
{
    vsize size = 0;
    const int ntracks = m_context->name2tracks.count();
    for (int i = 0; i < ntracks; i++) {
        const MorphAnimationTrack *const *track = m_context->name2tracks.value(i);
        const MorphAnimationTrack *trackPtr = *track;
        if (trackPtr->morphRef) {
            const MorphAnimationTrack::KeyframeCollection &keyframes = trackPtr->keyframes;
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

vsize MorphSection::countKeyframes() const
{
    return m_context->allKeyframeRefs.count();
}

void MorphSection::update()
{
    IKeyframe::TimeIndex durationTimeIndex = 0;
    const int nkeyframes = m_context->allKeyframeRefs.count();
    for (int i = 0; i < nkeyframes; i++) {
        IKeyframe *keyframe = m_context->allKeyframeRefs[i];
        btSetMax(durationTimeIndex, keyframe->timeIndex());
    }
    m_durationTimeIndex = durationTimeIndex;
}

void MorphSection::addKeyframe(IKeyframe *keyframe)
{
    int key = m_nameListSectionRef->key(keyframe->name());
    MorphAnimationTrack *const *track = m_context->name2tracks.find(key), *trackPtr = 0;
    if (track) {
        trackPtr = *track;
        trackPtr->keyframes.append(keyframe);
        BaseSection::setDuration(keyframe);
        m_context->allKeyframeRefs.append(keyframe);
    }
    else if (m_context->modelRef) {
        trackPtr = m_context->name2tracks.insert(key, new MorphAnimationTrack());
        trackPtr->morphRef = m_context->modelRef->findMorphRef(keyframe->name());
        trackPtr->keyframes.append(keyframe);
        BaseSection::setDuration(keyframe);
        m_context->allKeyframeRefs.append(keyframe);
        m_context->track2names.insert(trackPtr, key);
    }
}

void MorphSection::removeKeyframe(IKeyframe *keyframe)
{
    int key = m_nameListSectionRef->key(keyframe->name());
    if (MorphAnimationTrack *const *track = m_context->name2tracks.find(key)) {
        MorphAnimationTrack *trackPtr = *track;
        trackPtr->keyframes.remove(keyframe);
        m_context->allKeyframeRefs.remove(keyframe);
        if (trackPtr->keyframes.count() == 0) {
            m_context->name2tracks.remove(key);
            m_context->track2names.remove(trackPtr);
            internal::deleteObject(trackPtr);
        }
    }
}

void MorphSection::deleteKeyframe(IKeyframe *&keyframe)
{
    removeKeyframe(keyframe);
    internal::deleteObject(keyframe);
}

void MorphSection::getKeyframes(const IKeyframe::TimeIndex & /* timeIndex */,
                                const IKeyframe::LayerIndex & /* layerIndex */,
                                Array<IKeyframe *> & /* keyframes */) const
{
}

void MorphSection::getAllKeyframes(Array<IKeyframe *> &keyframes) const
{
    keyframes.copy(m_context->allKeyframeRefs);
}

void MorphSection::setAllKeyframes(const Array<IKeyframe *> &value)
{
    release();
    const int nkeyframes = value.count();
    for (int i = 0; i < nkeyframes; i++) {
        IKeyframe *keyframe = value[i];
        if (keyframe && keyframe->type() == IKeyframe::kMorphKeyframe) {
            BaseSection::setDuration(keyframe);
            m_context->allKeyframeRefs.append(keyframe);
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
    MorphAnimationTrack *const *track = m_context->name2tracks.find(m_nameListSectionRef->key(name));
    if (track) {
        const MorphAnimationTrack::KeyframeCollection &keyframes = (*track)->keyframes;
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
    if (internal::checkBound(index, 0, m_context->allKeyframeRefs.count())) {
        MorphKeyframe *keyframe = reinterpret_cast<MorphKeyframe *>(m_context->allKeyframeRefs[index]);
        return keyframe;
    }
    return 0;
}


} /* namespace mvd */
} /* namespace vpvl2 */

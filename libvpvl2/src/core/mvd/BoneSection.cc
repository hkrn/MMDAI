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
#include "vpvl2/internal/MotionHelper.h"

#include "vpvl2/mvd/BoneKeyframe.h"
#include "vpvl2/mvd/BoneSection.h"
#include "vpvl2/mvd/NameListSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct BoneSectionHeader {
    int32_t key;
    int32_t sizeOfKeyframe;
    int32_t countOfKeyframes;
    int32_t countOfLayers;
};

#pragma pack(pop)

class BoneAnimationTrack : public BaseAnimationTrack {
public:
    IBone *boneRef;
    Vector3 position;
    Quaternion rotation;
    IKeyframe::LayerIndex countOfLayers;
    BoneAnimationTrack()
        : boneRef(0),
          position(kZeroV3),
          rotation(Quaternion::getIdentity()),
          countOfLayers(1)
    {
    }
    ~BoneAnimationTrack() {
        boneRef = 0;
        position.setZero();
        rotation.setValue(0, 0, 0, 1);
        countOfLayers = 0;
    }
    void seek(const IKeyframe::TimeIndex &timeIndex) {
        if (boneRef && keyframes.count() > 0) {
            int fromIndex, toIndex;
            IKeyframe::TimeIndex currentTimeIndex;
            internal::MotionHelper::findKeyframeIndices(timeIndex, currentTimeIndex, m_lastIndex, fromIndex, toIndex, keyframes);
            const BoneKeyframe *keyframeFrom = reinterpret_cast<const BoneKeyframe *>(keyframes[fromIndex]),
                    *keyframeTo = reinterpret_cast<const BoneKeyframe *>(keyframes[toIndex]);
            const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), &timeIndexTo = keyframeTo->timeIndex();
            const Vector3 &positionFrom = keyframeFrom->localTranslation(), &positionTo = keyframeTo->localTranslation();
            const Quaternion &rotationFrom = keyframeFrom->localRotation(), &rotationTo = keyframeTo->localRotation();
            if (timeIndexFrom != timeIndexTo && timeIndexFrom < currentTimeIndex) {
                if (timeIndexTo <= currentTimeIndex) {
                    position = positionTo;
                    rotation = rotationTo;
                }
                else {
                    const IKeyframe::SmoothPrecision &weight = internal::MotionHelper::calculateWeight(currentTimeIndex, timeIndexFrom, timeIndexTo);
                    IKeyframe::SmoothPrecision x = 0, y = 0, z = 0;
                    internal::MotionHelper::interpolate(keyframeTo->tableForX(), positionFrom, positionTo, weight, 0, x);
                    internal::MotionHelper::interpolate(keyframeTo->tableForY(), positionFrom, positionTo, weight, 1, y);
                    internal::MotionHelper::interpolate(keyframeTo->tableForZ(), positionFrom, positionTo, weight, 2, z);
                    position.setValue(Scalar(x), Scalar(y), Scalar(z));
                    const internal::InterpolationTable &tableForRotation = keyframeTo->tableForRotation();
                    if (tableForRotation.linear) {
                        rotation = rotationFrom.slerp(rotationTo, Scalar(weight));
                    }
                    else {
                        const IKeyframe::SmoothPrecision &weight2 = internal::MotionHelper::calculateInterpolatedWeight(tableForRotation, weight);
                        rotation = rotationFrom.slerp(rotationTo, Scalar(weight2));
                    }
                }
            }
            else {
                position = positionFrom;
                rotation = rotationFrom;
            }
            boneRef->setLocalTranslation(position);
            boneRef->setLocalRotation(rotation);
        }
    }
};

struct BoneSection::PrivateContext {
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
    PointerHash<HashInt, BoneAnimationTrack> name2tracks;
    Hash<HashPtr, int> track2names;
};

BoneSection::BoneSection(const Motion *motionRef, IModel *modelRef)
    : BaseSection(motionRef),
      m_context(0)
{
    m_context = new PrivateContext(modelRef);
}

BoneSection::~BoneSection()
{
    release();
    delete m_context;
    m_context = 0;
}

bool BoneSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    BoneSectionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVDBoneSection header detected: " << rest);
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    if (!internal::validateSize(ptr, sizeof(uint8_t), header.countOfLayers, rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVDBoneSection layers detected: size=" << header.countOfLayers << " rest=" << rest);
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const size_t reserved = header.sizeOfKeyframe - BoneKeyframe::size();
    VPVL2_VLOG(2, "MVDBoneSection(Header): key=" << header.key);
    VPVL2_VLOG(2, "MVDBoneSection(Header): nkeyframes=" << nkeyframes);
    VPVL2_VLOG(2, "MVDBoneSection(Header): nlayers=" << header.countOfLayers);
    VPVL2_VLOG(2, "MVDBoneSection(Header): sizeofKeyframe=" << header.sizeOfKeyframe);
    VPVL2_VLOG(2, "MVDBoneSection(Header): reserved=" << reserved);
    for (int i = 0; i < nkeyframes; i++) {
        if (!BoneKeyframe::preparse(ptr, rest, reserved, info)) {
            VPVL2_LOG(WARNING, "Invalid size of MVDBoneSection key detected: index=" << i << " rest=" << rest);
            return false;
        }
    }
    return true;
}

void BoneSection::release()
{
    BaseSection::release();
    m_context->release();
}

void BoneSection::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    BoneSectionHeader header;
    internal::getData(ptr, header);
    const size_t sizeOfKeyframe = header.sizeOfKeyframe;
    const int nkeyframes = header.countOfKeyframes;
    ptr += sizeof(header) + sizeof(uint8_t) * header.countOfLayers;
    const int key = header.key;
    const IString *name = m_nameListSectionRef->value(key);
    BoneAnimationTrack *trackPtr = m_context->name2tracks.insert(key, new BoneAnimationTrack());
    m_context->track2names.insert(trackPtr, key);
    trackPtr->keyframes.reserve(nkeyframes);
    for (int i = 0; i < nkeyframes; i++) {
        BoneKeyframe *keyframePtr = trackPtr->keyframes.append(new BoneKeyframe(m_motionRef));
        keyframePtr->read(ptr);
        keyframePtr->setName(name);
        setMaxTimeIndex(keyframePtr);
        m_context->allKeyframeRefs.append(keyframePtr);
        ptr += sizeOfKeyframe;
    }
    trackPtr->keyframes.sort(internal::MotionHelper::KeyframeTimeIndexPredication());
    trackPtr->boneRef = m_context->modelRef ? m_context->modelRef->findBoneRef(name) : 0;
    trackPtr->countOfLayers = header.countOfLayers;
}

void BoneSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    if (m_context->modelRef) {
        const int ntracks = m_context->name2tracks.count();
        for (int i = 0; i < ntracks; i++) {
            if (BoneAnimationTrack *const *track = m_context->name2tracks.value(i)) {
                (*track)->seek(timeIndex);
            }
        }
    }
    saveCurrentTimeIndex(timeIndex);
}

void BoneSection::setParentModel(IModel *modelRef)
{
    m_context->modelRef = modelRef;
    if (modelRef) {
        const int ntracks = m_context->name2tracks.count();
        for (int i = 0; i < ntracks; i++) {
            if (BoneAnimationTrack *const *track = m_context->name2tracks.value(i)) {
                BoneAnimationTrack *trackRef = *track;
                if (const int *keyPtr = m_context->track2names.find(trackRef)) {
                    int key = *keyPtr;
                    IBone *bone = modelRef->findBoneRef(m_nameListSectionRef->value(key));
                    trackRef->boneRef = bone;
                }
                else {
                    trackRef->boneRef = 0;
                }
            }
        }
    }
}

void BoneSection::write(uint8_t *data) const
{
    const int ntracks = m_context->name2tracks.count();
    for (int i = 0; i < ntracks; i++) {
        const BoneAnimationTrack *const *track = m_context->name2tracks.value(i);
        const BoneAnimationTrack *trackRef = *track;
        if (const IBone *boneRef = trackRef->boneRef) {
            const BoneAnimationTrack::KeyframeCollection &keyframes = trackRef->keyframes;
            const int nkeyframes = keyframes.count();
            const int nlayers = trackRef->countOfLayers;
            Motion::SectionTag tag;
            tag.type = Motion::kBoneSection;
            tag.minor = 0;
            internal::writeBytes(&tag, sizeof(tag), data);
            BoneSectionHeader header;
            header.countOfKeyframes = nkeyframes;
            header.countOfLayers = nlayers;
            header.key = m_nameListSectionRef->key(boneRef->name());
            header.sizeOfKeyframe = BoneKeyframe::size();
            internal::writeBytes(&header, sizeof(header), data);
            for (int i = 0; i < nlayers; i++) {
                internal::writeSignedIndex(0, sizeof(uint8_t), data);
            }
            for (int i = 0 ; i < nkeyframes; i++) {
                const IKeyframe *keyframe = keyframes[i];
                keyframe->write(data);
                data += keyframe->estimateSize();
            }
        }
    }
}

size_t BoneSection::estimateSize() const
{
    size_t size = 0;
    const int ntracks = m_context->name2tracks.count();
    for (int i = 0; i < ntracks; i++) {
        const BoneAnimationTrack *const *track = m_context->name2tracks.value(i);
        const BoneAnimationTrack *trackRef = *track;
        if (trackRef->boneRef) {
            const BoneAnimationTrack::KeyframeCollection &keyframes = trackRef->keyframes;
            const int nkeyframes = keyframes.count();
            size += sizeof(Motion::SectionTag);
            size += sizeof(BoneSectionHeader);
            size += sizeof(uint8_t) * trackRef->countOfLayers;
            for (int i = 0 ; i < nkeyframes; i++) {
                const IKeyframe *keyframe = keyframes[i];
                size += keyframe->estimateSize();
            }
        }
    }
    return size;
}

size_t BoneSection::countKeyframes() const
{
    return m_context->allKeyframeRefs.count();
}

IKeyframe::LayerIndex BoneSection::countLayers(const IString *name) const
{
    BoneAnimationTrack *const *track = m_context->name2tracks.find(m_nameListSectionRef->key(name));
    return track ? (*track)->countOfLayers : 0;
}

void BoneSection::addKeyframe(IKeyframe *keyframe)
{
    int key = m_nameListSectionRef->key(keyframe->name());
    BoneAnimationTrack *const *track = m_context->name2tracks.find(key), *trackPtr = 0;
    if (track) {
        trackPtr = *track;
        trackPtr->keyframes.append(keyframe);
        setMaxTimeIndex(keyframe);
        m_context->allKeyframeRefs.append(keyframe);
    }
    else if (m_context->modelRef) {
        trackPtr = m_context->name2tracks.insert(key, new BoneAnimationTrack());
        trackPtr->boneRef = m_context->modelRef->findBoneRef(keyframe->name());
        trackPtr->keyframes.append(keyframe);
        setMaxTimeIndex(keyframe);
        m_context->allKeyframeRefs.append(keyframe);
        m_context->name2tracks.insert(key, trackPtr);
        m_context->track2names.insert(trackPtr, key);
    }
}

void BoneSection::deleteKeyframe(IKeyframe *&keyframe)
{
    int key = m_nameListSectionRef->key(keyframe->name());
    if (BoneAnimationTrack *const *track = m_context->name2tracks.find(key)) {
        BoneAnimationTrack *trackPtr = *track;
        trackPtr->keyframes.remove(keyframe);
        m_context->allKeyframeRefs.remove(keyframe);
        if (trackPtr->keyframes.count() == 0) {
            m_context->name2tracks.remove(key);
            m_context->track2names.remove(trackPtr);
            delete trackPtr;
        }
        delete keyframe;
        keyframe = 0;
    }
}

void BoneSection::getKeyframes(const IKeyframe::TimeIndex & /* timeIndex */,
                               const IKeyframe::LayerIndex & /* layerIndex */,
                               Array<IKeyframe *> & /* keyframes */) const
{
}

void BoneSection::getAllKeyframes(Array<IKeyframe *> &value) const
{
    value.copy(m_context->allKeyframeRefs);
}

void BoneSection::setAllKeyframes(const Array<IKeyframe *> &value)
{
    release();
    const int nkeyframes = value.count();
    for (int i = 0; i < nkeyframes; i++) {
        IKeyframe *keyframe = value[i];
        if (keyframe && keyframe->type() == IKeyframe::kBoneKeyframe) {
            setMaxTimeIndex(keyframe);
            m_context->allKeyframeRefs.append(keyframe);
        }
    }
}

IBoneKeyframe *BoneSection::findKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                         const IString *name,
                                         const IKeyframe::LayerIndex &layerIndex) const
{
    if (BoneAnimationTrack *const *track = m_context->name2tracks.find(m_nameListSectionRef->key(name))) {
        const BoneAnimationTrack::KeyframeCollection &keyframes = (*track)->keyframes;
        const int nkeyframes = keyframes.count();
        for (int i = 0; i < nkeyframes; i++) {
            BoneKeyframe *keyframe = reinterpret_cast<BoneKeyframe *>(keyframes[i]);
            if (keyframe->timeIndex() == timeIndex && keyframe->layerIndex() == layerIndex) {
                return keyframe;
            }
        }
    }
    return 0;
}

IBoneKeyframe *BoneSection::findKeyframeAt(int index) const
{
    if (internal::checkBound(index, 0, m_context->allKeyframeRefs.count())) {
        BoneKeyframe *keyframe = reinterpret_cast<BoneKeyframe *>(m_context->allKeyframeRefs[index]);
        return keyframe;
    }
    return 0;
}

} /* namespace mvd */
} /* namespace vpvl2 */

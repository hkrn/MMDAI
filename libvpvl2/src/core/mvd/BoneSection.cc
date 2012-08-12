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

struct BoneSection::PrivateContext : public BaseSectionContext {
    IBone *boneRef;
    Vector3 position;
    Quaternion rotation;
    int countOfLayers;
    PrivateContext()
        : boneRef(0),
          position(kZeroV3),
          rotation(Quaternion::getIdentity()),
          countOfLayers(0)
    {
    }
    ~PrivateContext() {
        boneRef = 0;
        position.setZero();
        rotation.setValue(0, 0, 0, 1);
        countOfLayers = 0;
    }
    void seek(const IKeyframe::TimeIndex &timeIndex) {
        int fromIndex, toIndex;
        IKeyframe::TimeIndex currentTimeIndex;
        findKeyframeIndices(timeIndex, currentTimeIndex, fromIndex, toIndex);
        const BoneKeyframe *keyframeFrom = reinterpret_cast<const BoneKeyframe *>(keyframes->at(fromIndex)),
                *keyframeTo = reinterpret_cast<const BoneKeyframe *>(keyframes->at(toIndex));
        const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(),
                timeIndexTo = keyframeTo->timeIndex();
        const Vector3 &positionFrom = keyframeFrom->position();
        const Quaternion &rotationFrom = keyframeFrom->rotation();
        const Vector3 &positionTo = keyframeTo->position();
        const Quaternion &rotationTo = keyframeTo->rotation();
        if (timeIndexFrom != timeIndexTo) {
            if (currentTimeIndex <= timeIndexFrom) {
                position = positionFrom;
                rotation = rotationFrom;
            }
            else if (currentTimeIndex >= timeIndexTo) {
                position = positionTo;
                rotation = rotationTo;
            }
            else {
                const IKeyframe::SmoothPrecision &weight = (currentTimeIndex - timeIndexFrom) / (timeIndexTo - timeIndexFrom);
                IKeyframe::SmoothPrecision x = 0, y = 0, z = 0;
                lerp(keyframeTo->tableForX(), positionFrom, positionTo, weight, 0, x);
                lerp(keyframeTo->tableForY(), positionFrom, positionTo, weight, 1, y);
                lerp(keyframeTo->tableForZ(), positionFrom, positionTo, weight, 2, z);
                position.setValue(x, y, z);
                const Motion::InterpolationTable &rt = keyframeTo->tableForRotation();
                if (rt.linear) {
                    rotation = rotationFrom.slerp(rotationTo, weight);
                }
                else {
                    const IKeyframe::SmoothPrecision &weight2 = calculateWeight(rt, weight);
                    rotation = rotationFrom.slerp(rotationTo, weight2);
                }
            }
        }
        else {
            position = positionFrom;
            rotation = rotationFrom;
        }
        boneRef->setPosition(position);
        boneRef->setRotation(rotation);
    }
};

BoneSection::BoneSection(IModel *model, NameListSection *nameListSectionRef)
    : BaseSection(nameListSectionRef),
      m_modelRef(model),
      m_keyframePtr(0),
      m_contextPtr(0)
{
}

BoneSection::~BoneSection()
{
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
    BaseSection::release();
    delete m_contextPtr;
    m_contextPtr = 0;
    delete m_keyframePtr;
    m_keyframePtr = 0;
    m_allKeyframes.releaseAll();
}

void BoneSection::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    const BoneSectionHeader &header = *reinterpret_cast<const BoneSectionHeader *>(ptr);
    const size_t sizeOfKeyframe = header.sizeOfKeyframe;
    const int nkeyframes = header.countOfKeyframes;
    ptr += sizeof(header) + sizeof(uint8_t) * header.countOfLayers;
    delete m_keyframeListPtr;
    m_keyframeListPtr = new KeyframeList();
    m_keyframeListPtr->reserve(nkeyframes);
    for (int i = 0; i < nkeyframes; i++) {
        m_keyframePtr = new BoneKeyframe(m_nameListSectionRef);
        m_keyframePtr->read(ptr);
        m_keyframeListPtr->add(m_keyframePtr);
        btSetMax(m_maxTimeIndex, m_keyframePtr->timeIndex());
        ptr += sizeOfKeyframe;
    }
    m_keyframeListPtr->sort(KeyframeTimeIndexPredication());
    delete m_contextPtr;
    m_contextPtr = new PrivateContext();
    m_contextPtr->keyframes = m_keyframeListPtr;
    m_contextPtr->boneRef = m_modelRef ? m_modelRef->findBone(m_nameListSectionRef->value(header.key)) : 0;
    m_contextPtr->countOfLayers = header.countOfLayers;
    m_allKeyframes.insert(header.key, m_contextPtr);
    m_keyframeListPtr = 0;
    m_keyframePtr = 0;
    m_contextPtr = 0;
}

void BoneSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    if (m_modelRef) {
        const int ncontexts = m_allKeyframes.count();
        for (int i = 0; i < ncontexts; i++) {
            PrivateContext **context = const_cast<PrivateContext **>(m_allKeyframes.value(i));
            (*context)->seek(timeIndex);
        }
    }
    m_previousTimeIndex = m_currentTimeIndex;
    m_currentTimeIndex = timeIndex;
}

void BoneSection::write(uint8_t * /* data */) const
{
}

size_t BoneSection::estimateSize() const
{
    size_t size = 0;
    const int ncontexts = m_allKeyframes.count();
    for (int i = 0; i < ncontexts; i++) {
        const PrivateContext *const *context = m_allKeyframes.value(i);
        const Array<IKeyframe *> *keyframes = (*context)->keyframes;
        const int nkeyframes = keyframes->count();
        size += sizeof(BoneSectionHeader);
        size += sizeof(uint8_t) * (*context)->countOfLayers;
        for (int i = 0 ; i < nkeyframes; i++) {
            size += keyframes->at(i)->estimateSize();
        }
    }
    return size;
}

size_t BoneSection::countKeyframes() const
{
    size_t nkeyframes = 0;
    const int ncontexts = m_allKeyframes.count();
    for (int i = 0; i < ncontexts; i++) {
        const PrivateContext *const *context = m_allKeyframes.value(i);
        nkeyframes += (*context)->keyframes->count();
    }
    return nkeyframes;
}

} /* namespace mvd */
} /* namespace vpvl2 */

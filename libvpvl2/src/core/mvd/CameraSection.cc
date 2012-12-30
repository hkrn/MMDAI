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

#include "vpvl2/mvd/CameraKeyframe.h"
#include "vpvl2/mvd/CameraSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct CameraSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int countOfLayers;
};

#pragma pack(pop)

class CameraSection::PrivateContext : public BaseSectionContext {
public:
    Vector3 position;
    Vector3 angle;
    Scalar distance;
    Scalar fov;
    IKeyframe::LayerIndex countOfLayers;
    PrivateContext()
        : position(kZeroV3),
          angle(kZeroV3),
          distance(0),
          fov(0),
          countOfLayers(1)
    {
    }
    ~PrivateContext() {
        position.setZero();
        angle.setZero();
        distance = 0;
        fov = 0;
        countOfLayers = 0;
    }
    void seek(const IKeyframe::TimeIndex &timeIndex) {
        int fromIndex, toIndex;
        IKeyframe::TimeIndex currentTimeIndex;
        findKeyframeIndices(timeIndex, currentTimeIndex, fromIndex, toIndex);
        const CameraKeyframe *keyframeFrom = reinterpret_cast<const CameraKeyframe *>(keyframes->at(fromIndex)),
                *keyframeTo = reinterpret_cast<const CameraKeyframe *>(keyframes->at(toIndex));
        const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), timeIndexTo = keyframeTo->timeIndex();
        const Scalar &distanceFrom = keyframeFrom->distance(), fovyFrom = keyframeFrom->fov();
        const Vector3 &positionFrom = keyframeFrom->lookAt(), angleFrom = keyframeFrom->angle();
        const Scalar &distanceTo = keyframeTo->distance(), fovyTo = keyframeTo->fov();
        const Vector3 &positionTo = keyframeTo->lookAt(), angleTo = keyframeTo->angle();
        if (timeIndexFrom != timeIndexTo) {
            if (currentTimeIndex <= timeIndexFrom) {
                distance = distanceFrom;
                position = positionFrom;
                angle = angleFrom;
                fov = fovyFrom;
            }
            else if (currentTimeIndex >= timeIndexTo) {
                distance = distanceTo;
                position = positionTo;
                angle = angleTo;
                fov = fovyTo;
            }
            else if (timeIndexTo - timeIndexFrom <= 1.0f) {
                distance = distanceFrom;
                position = positionFrom;
                angle = angleFrom;
                fov = fovyFrom;
            }
            else {
                const IKeyframe::SmoothPrecision &weight = calculateWeight(currentTimeIndex, timeIndexFrom, timeIndexTo);
                IKeyframe::SmoothPrecision x = 0, y = 0, z = 0;
                interpolate(keyframeTo->tableForPosition(), positionFrom, positionTo, weight, 0, x);
                interpolate(keyframeTo->tableForPosition(), positionFrom, positionTo, weight, 1, y);
                interpolate(keyframeTo->tableForPosition(), positionFrom, positionTo, weight, 2, z);
                position.setValue(Scalar(x), Scalar(y), Scalar(z));
                const Motion::InterpolationTable &tableForRotation = keyframeTo->tableForRotation();
                if (tableForRotation.linear) {
                    angle = angleFrom.lerp(angleTo, Scalar(weight));
                }
                else {
                    const IKeyframe::SmoothPrecision &weight2 = calculateInterpolatedWeight(tableForRotation, weight);
                    angle = angleFrom.lerp(angleTo, Scalar(weight2));
                }
                const Motion::InterpolationTable &tableForDistance = keyframeTo->tableForDistance();
                if (tableForDistance.linear) {
                    distance = Scalar(internal::lerp(distanceFrom, distanceTo, weight));
                }
                else {
                    const IKeyframe::SmoothPrecision &weight2 = calculateInterpolatedWeight(tableForDistance, weight);
                    distance = Scalar(internal::lerp(distanceFrom, distanceTo, weight2));
                }
                const Motion::InterpolationTable &tableForFov = keyframeTo->tableForFov();
                if (tableForFov.linear) {
                    fov = Scalar(internal::lerp(fovyFrom, fovyTo, weight));
                }
                else {
                    const IKeyframe::SmoothPrecision &weight2 = calculateInterpolatedWeight(tableForFov, weight);
                    fov = Scalar(internal::lerp(fovyFrom, fovyTo, weight2));
                }
            }
        }
        else {
            distance = distanceFrom;
            position = positionFrom;
            angle = angleFrom;
            fov = fovyFrom;
        }
    }
};

CameraSection::CameraSection(NameListSection *nameListSectionRef)
    : BaseSection(nameListSectionRef),
      m_keyframePtr(0),
      m_contextPtr(0)
{
}

CameraSection::~CameraSection()
{
    release();
}

bool CameraSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    CameraSectionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    if (!internal::validateSize(ptr, sizeof(uint8_t), header.countOfLayers, rest)) {
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const size_t reserved = header.sizeOfKeyframe - CameraKeyframe::size();
    for (int i = 0; i < nkeyframes; i++) {
        if (!CameraKeyframe::preparse(ptr, rest, reserved, info)) {
            return false;
        }
    }
    return true;
}

void CameraSection::release()
{
    delete m_keyframePtr;
    m_keyframePtr = 0;
    delete m_contextPtr;
    m_contextPtr = 0;
}

void CameraSection::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    CameraSectionHeader header;
    internal::getData(ptr, header);
    const size_t sizeOfkeyframe = header.sizeOfKeyframe;
    const int nkeyframes = header.countOfKeyframes;
    ptr += sizeof(header) + sizeof(uint8_t) * header.countOfLayers;
    delete m_contextPtr;
    m_contextPtr = new PrivateContext();
    m_contextPtr->keyframes = new PrivateContext::KeyframeCollection();
    m_contextPtr->keyframes->reserve(nkeyframes);
    m_contextPtr->countOfLayers = header.countOfLayers;
    for (int i = 0; i < nkeyframes; i++) {
        m_keyframePtr = new CameraKeyframe();
        m_keyframePtr->read(ptr);
        addKeyframe0(m_keyframePtr, m_contextPtr->keyframes);
        ptr += sizeOfkeyframe;
    }
    m_contextPtr->keyframes->sort(KeyframeTimeIndexPredication());
    m_keyframePtr = 0;
}

void CameraSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    if (m_contextPtr)
        m_contextPtr->seek(timeIndex);
    saveCurrentTimeIndex(timeIndex);
}

void CameraSection::write(uint8_t *data) const
{
    if (m_contextPtr) {
        const PrivateContext::KeyframeCollection *keyframes = m_contextPtr->keyframes;
        const int nkeyframes = keyframes->count();
        const int nlayers = m_contextPtr->countOfLayers;
        Motion::SectionTag tag;
        tag.type = Motion::kCameraSection;
        tag.minor = 0;
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&tag), sizeof(tag), data);
        CameraSectionHeader header;
        header.countOfKeyframes = nkeyframes;
        header.countOfLayers = nlayers;
        header.reserved = 0;
        header.sizeOfKeyframe = CameraKeyframe::size();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&header), sizeof(header), data);
        for (int i = 0; i < nlayers; i++) {
            internal::writeSignedIndex(0, sizeof(uint8_t), data);
        }
        for (int i = 0; i < nkeyframes; i++) {
            const IKeyframe *keyframe = keyframes->at(i);
            keyframe->write(data);
            data += keyframe->estimateSize();
        }
    }
}

size_t CameraSection::estimateSize() const
{
    size_t size = 0;
    if (m_contextPtr) {
        size += sizeof(Motion::SectionTag);
        size += sizeof(CameraSectionHeader);
        size += sizeof(uint8_t) * m_contextPtr->countOfLayers;
        const PrivateContext::KeyframeCollection *keyframes = m_contextPtr->keyframes;
        const int nkeyframes = keyframes->count();
        for (int i = 0; i < nkeyframes; i++) {
            const IKeyframe *keyframe = keyframes->at(i);
            size += keyframe->estimateSize();
        }
    }
    return size;
}

size_t CameraSection::countKeyframes() const
{
    return m_contextPtr ? m_contextPtr->keyframes->count() : 0;
}

void CameraSection::addKeyframe(IKeyframe *keyframe)
{
    if (!m_contextPtr) {
        m_contextPtr = new PrivateContext();
        m_contextPtr->keyframes = new PrivateContext::KeyframeCollection();
    }
    addKeyframe0(keyframe, m_contextPtr->keyframes);
}

void CameraSection::deleteKeyframe(IKeyframe *&keyframe)
{
    if (m_contextPtr) {
        m_contextPtr->keyframes->remove(keyframe);
        delete keyframe;
        keyframe = 0;
    }
}

void CameraSection::getKeyframes(const IKeyframe::TimeIndex & /* timeIndex */,
                                 const IKeyframe::LayerIndex & /* layerIndex */,
                                 Array<IKeyframe *> & /* keyframes */)
{
}

IKeyframe::LayerIndex CameraSection::countLayers() const
{
    return m_contextPtr ? m_contextPtr->countOfLayers : 0;
}

ICameraKeyframe *CameraSection::findKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                             const IKeyframe::LayerIndex &layerIndex) const
{
    const PrivateContext::KeyframeCollection *keyframes = m_contextPtr->keyframes;
    const int nkeyframes = keyframes->count();
    for (int i = 0; i < nkeyframes; i++) {
        mvd::CameraKeyframe *keyframe = reinterpret_cast<mvd::CameraKeyframe *>(keyframes->at(i));
        if (keyframe->timeIndex() == timeIndex && keyframe->layerIndex() == layerIndex) {
            return keyframe;
        }
    }
    return 0;
}

ICameraKeyframe *CameraSection::findKeyframeAt(int index) const
{
    if (m_contextPtr) {
        const PrivateContext::KeyframeCollection *keyframes = m_contextPtr->keyframes;
        if (internal::checkBound(index, 0, keyframes->count())) {
            mvd::CameraKeyframe *keyframe = reinterpret_cast<mvd::CameraKeyframe *>(keyframes->at(index));
            return keyframe;
        }
    }
    return 0;
}

const Vector3 &CameraSection::position() const
{
    return m_contextPtr ? m_contextPtr->position : kZeroV3;
}

const Vector3 &CameraSection::angle() const
{
    return m_contextPtr ? m_contextPtr->angle : kZeroV3;
}

Scalar CameraSection::fov() const
{
    return m_contextPtr ? m_contextPtr->fov : 0;
}

Scalar CameraSection::distance() const
{
    return m_contextPtr ? m_contextPtr->distance : 0;
}

void CameraSection::addKeyframe0(IKeyframe *keyframe, BaseSectionContext::KeyframeCollection *keyframes)
{
    keyframes->add(keyframe);
    btSetMax(m_maxTimeIndex, keyframe->timeIndex());
}

} /* namespace mvd */
} /* namespace vpvl2 */

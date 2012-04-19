/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#include "vpvl2/vmd/BoneAnimation.h"
#include "vpvl2/vmd/BoneKeyframe.h"
#include "vpvl2/vmd/CameraAnimation.h"
#include "vpvl2/vmd/CameraKeyframe.h"
#include "vpvl2/vmd/LightAnimation.h"
#include "vpvl2/vmd/LightKeyframe.h"
#include "vpvl2/vmd/MorphAnimation.h"
#include "vpvl2/vmd/MorphKeyframe.h"
#include "vpvl2/vmd/Motion.h"

namespace vpvl2
{
namespace vmd
{

const uint8_t *Motion::kSignature = reinterpret_cast<const uint8_t *>("Vocaloid Motion Data 0002");

Motion::Motion(IModel *model, IEncoding *encoding)
    : m_model(model),
      m_encoding(encoding),
      m_boneMotion(encoding),
      m_morphMotion(encoding),
      m_error(kNoError),
      m_active(true)
{
    internal::zerofill(&m_name, sizeof(m_name));
}

Motion::~Motion()
{
    release();
}

bool Motion::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    // Header(30) + Name(20)
    if (kSignatureSize + kNameSize > rest) {
        m_error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    info.basePtr = ptr;

    // Check the signature is valid
    if (memcmp(ptr, kSignature, sizeof(kSignature)) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }
    ptr += kSignatureSize;
    info.namePtr = ptr;
    ptr += kNameSize;
    rest -= kSignatureSize + kNameSize;

    // Bone key frame
    size_t nBoneKeyFrames, nMorphFrames, nCameraKeyFrames, nLightKeyFrames;
    if (!internal::size32(ptr, rest, nBoneKeyFrames)) {
        m_error = kBoneKeyFramesSizeError;
        return false;
    }
    info.boneKeyframePtr = ptr;
    if (!internal::validateSize(ptr, BoneKeyframe::strideSize(), nBoneKeyFrames, rest)) {
        m_error = kBoneKeyFramesError;
        return false;
    }
    info.boneKeyframeCount = nBoneKeyFrames;

    // Morph key frame
    if (!internal::size32(ptr, rest, nMorphFrames)) {
        m_error = kMorphKeyFramesSizeError;
        return false;
    }
    info.morphKeyframePtr = ptr;
    if (!internal::validateSize(ptr, MorphKeyframe::strideSize(), nMorphFrames, rest)) {
        m_error = kMorphKeyFramesError;
        return false;
    }
    info.morphKeyframeCount = nMorphFrames;

    // Camera key frame
    if (!internal::size32(ptr, rest, nCameraKeyFrames)) {
        m_error = kCameraKeyFramesSizeError;
        return false;
    }
    info.cameraKeyframePtr = ptr;
    if (!internal::validateSize(ptr, CameraKeyframe::strideSize(), nCameraKeyFrames, rest)) {
        m_error = kCameraKeyFramesError;
        return false;
    }
    info.cameraKeyframeCount = nCameraKeyFrames;

    // Light key frame
    if (!internal::size32(ptr, rest, nLightKeyFrames)) {
        m_error = kLightKeyFramesSizeError;
        return false;
    }
    info.lightKeyframePtr = ptr;
    if (!internal::validateSize(ptr, LightKeyframe::strideSize(), nLightKeyFrames, rest)) {
        m_error = kCameraKeyFramesError;
        return false;
    }
    info.lightKeyframeCount = nLightKeyFrames;

    return true;
}

bool Motion::load(const uint8_t *data, size_t size)
{
    DataInfo info;
    internal::zerofill(&info, sizeof(info));
    if (preparse(data, size, info)) {
        release();
        parseHeader(info);
        parseBoneFrames(info);
        parseMorphFrames(info);
        parseCameraFrames(info);
        parseLightFrames(info);
        parseSelfShadowFrames(info);
        return true;
    }
    return false;
}

void Motion::save(uint8_t *data) const
{
    internal::writeBytes(kSignature, kSignatureSize, data);
    uint8_t *name = m_encoding->toByteArray(m_name, IString::kShiftJIS);
    internal::copyBytes(data, name, sizeof(m_name));
    m_encoding->disposeByteArray(name);
    data += kNameSize;
    int nBoneFrames = m_boneMotion.countKeyframes();
    internal::writeBytes(reinterpret_cast<uint8_t *>(&nBoneFrames), sizeof(nBoneFrames), data);
    for (int i = 0; i < nBoneFrames; i++) {
        BoneKeyframe *frame = m_boneMotion.frameAt(i);
        frame->write(data);
        data += BoneKeyframe::strideSize();
    }
    int nMorphFrames = m_morphMotion.countKeyframes();
    internal::writeBytes(reinterpret_cast<uint8_t *>(&nMorphFrames), sizeof(nMorphFrames), data);
    for (int i = 0; i < nMorphFrames; i++) {
        MorphKeyframe *frame = m_morphMotion.frameAt(i);
        frame->write(data);
        data += MorphKeyframe::strideSize();
    }
    int nCameraFrames = m_cameraMotion.countKeyframes();
    internal::writeBytes(reinterpret_cast<uint8_t *>(&nCameraFrames), sizeof(nCameraFrames), data);
    for (int i = 0; i < nCameraFrames; i++) {
        CameraKeyframe *frame = m_cameraMotion.frameAt(i);
        frame->write(data);
        data += CameraKeyframe::strideSize();
    }
    int nLightFrames = m_lightMotion.countKeyframes();
    internal::writeBytes(reinterpret_cast<uint8_t *>(&nLightFrames), sizeof(nLightFrames), data);
    for (int i = 0; i < nLightFrames; i++) {
        LightKeyframe *frame = m_lightMotion.frameAt(i);
        frame->write(data);
        data += LightKeyframe::strideSize();
    }
    int empty = 0;
    internal::writeBytes(reinterpret_cast<uint8_t *>(&empty), sizeof(empty), data);
}

size_t Motion::estimateSize() const
{
    /*
     * header[30]
     * name[20]
     * bone size
     * morph size
     * camera size
     * light size (empty)
     * selfshadow size (empty)
     */
    return kSignatureSize + kNameSize + sizeof(int) * 5
            + m_boneMotion.countKeyframes() * BoneKeyframe::strideSize()
            + m_morphMotion.countKeyframes() * MorphKeyframe::strideSize()
            + m_cameraMotion.countKeyframes() * CameraKeyframe::strideSize()
            + m_lightMotion.countKeyframes() * LightKeyframe::strideSize();
}

void Motion::setParentModel(IModel *model)
{
    m_boneMotion.setParentModel(model);
    m_morphMotion.setParentModel(model);
    m_model = model;
}

void Motion::seek(float frameIndex)
{
    m_boneMotion.seek(frameIndex);
    m_morphMotion.seek(frameIndex);
    m_active = maxFrameIndex() > frameIndex;
}

void Motion::advance(float delta)
{
    if (delta == 0.0f) {
        m_boneMotion.advance(delta);
        m_morphMotion.advance(delta);
    }
    else if (m_active) {
        // The motion is active and continue to advance
        m_boneMotion.advance(delta);
        m_morphMotion.advance(delta);
        if (m_boneMotion.currentIndex() >= m_boneMotion.maxIndex() &&
                m_morphMotion.currentIndex() >= m_morphMotion.maxIndex()) {
            m_active = false;
        }
    }
}

void Motion::reload()
{
    /* rebuild internal keyframe nodes */
    m_boneMotion.setParentModel(m_model);
    m_morphMotion.setParentModel(m_model);
    reset();
}

void Motion::reset()
{
    m_boneMotion.seek(0.0f);
    m_morphMotion.seek(0.0f);
    m_boneMotion.reset();
    m_morphMotion.reset();
    m_active = true;
}

float Motion::maxFrameIndex() const
{
    return btMax(m_boneMotion.maxIndex(), m_morphMotion.maxIndex());
}

bool Motion::isReachedTo(float atEnd) const
{
    // force inactive motion is reached
    return !m_active || (m_boneMotion.currentIndex() >= atEnd && m_morphMotion.currentIndex() >= atEnd);
}

bool Motion::isNullFrameEnabled() const
{
    return m_boneMotion.isNullFrameEnabled() && m_morphMotion.isNullFrameEnabled();
}

void Motion::setNullFrameEnable(bool value)
{
    m_boneMotion.setNullFrameEnable(value);
    m_morphMotion.setNullFrameEnable(value);
}

void Motion::addKeyframe(IKeyframe *value)
{
    switch (value->type()) {
    case IKeyframe::kBone:
        m_boneMotion.addKeyframe(value);
        break;
    case IKeyframe::kCamera:
        m_cameraMotion.addKeyframe(value);
        break;
    case IKeyframe::kLight:
        m_lightMotion.addKeyframe(value);
        break;
    case IKeyframe::kMorph:
        m_morphMotion.addKeyframe(value);
        break;
    default:
        break;
    }
}

int Motion::countKeyframes(IKeyframe::Type value) const
{
    switch (value) {
    case IKeyframe::kBone:
        return m_boneMotion.countKeyframes();
    case IKeyframe::kCamera:
        return m_cameraMotion.countKeyframes();
    case IKeyframe::kLight:
        return m_lightMotion.countKeyframes();
    case IKeyframe::kMorph:
        return m_morphMotion.countKeyframes();
    default:
        return 0;
    }
}

void Motion::deleteKeyframe(IKeyframe *value)
{
    switch (value->type()) {
    case IKeyframe::kBone:
        m_boneMotion.deleteKeyframe(value->frameIndex(), value->name());
        break;
    case IKeyframe::kCamera:
        m_cameraMotion.deleteKeyframe(value->frameIndex(), value->name());
        break;
    case IKeyframe::kLight:
        m_lightMotion.deleteKeyframe(value->frameIndex(), value->name());
        break;
    case IKeyframe::kMorph:
        m_morphMotion.deleteKeyframe(value->frameIndex(), value->name());
        break;
    default:
        break;
    }
}

void Motion::parseHeader(const DataInfo &info)
{
    m_name = m_encoding->toString(info.namePtr, IString::kShiftJIS, kNameSize);
}

void Motion::parseBoneFrames(const DataInfo &info)
{
    m_boneMotion.read(info.boneKeyframePtr, info.boneKeyframeCount);
    m_boneMotion.setParentModel(m_model);
}

void Motion::parseMorphFrames(const DataInfo &info)
{
    m_morphMotion.read(info.morphKeyframePtr, info.morphKeyframeCount);
    m_morphMotion.setParentModel(m_model);
}

void Motion::parseCameraFrames(const DataInfo &info)
{
    m_cameraMotion.read(info.cameraKeyframePtr, info.cameraKeyframeCount);
}

void Motion::parseLightFrames(const DataInfo &info)
{
    m_lightMotion.read(info.lightKeyframePtr, info.lightKeyframeCount);
}

void Motion::parseSelfShadowFrames(const DataInfo & /* info */)
{
}

void Motion::release()
{
    delete m_name;
    m_name = 0;
}

}
}

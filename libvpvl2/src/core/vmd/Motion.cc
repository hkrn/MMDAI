/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2013  hkrn                                    */
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
    : m_motionPtr(0),
      m_parentSceneRef(0),
      m_parentModelRef(model),
      m_encodingRef(encoding),
      m_name(0),
      m_boneMotion(encoding),
      m_morphMotion(encoding),
      m_error(kNoError),
      m_active(true)
{
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
    if (memcmp(ptr, kSignature, sizeof(kSignature) - 1) != 0) {
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

    size_t cameraKeyframeStrideSize = CameraKeyframe::strideSize();
    if (!internal::validateSize(ptr, cameraKeyframeStrideSize, nCameraKeyFrames, rest)) {
        m_error = kCameraKeyFramesError;
        return false;
    }
    info.cameraKeyframeCount = nCameraKeyFrames;

    // workaround for no camera keyframe
    if (nCameraKeyFrames == 0 && rest > cameraKeyframeStrideSize)
        internal::validateSize(ptr, cameraKeyframeStrideSize, 1, rest);

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
    uint8_t *name = m_encodingRef->toByteArray(m_name, IString::kShiftJIS);
    internal::copyBytes(data, name, kNameSize);
    m_encodingRef->disposeByteArray(name);
    data += kNameSize;
    int nBoneFrames = m_boneMotion.countKeyframes();
    internal::writeBytes(reinterpret_cast<uint8_t *>(&nBoneFrames), sizeof(nBoneFrames), data);
    for (int i = 0; i < nBoneFrames; i++) {
        BoneKeyframe *keyframe = m_boneMotion.keyframeAt(i);
        keyframe->write(data);
        data += BoneKeyframe::strideSize();
    }
    int nMorphFrames = m_morphMotion.countKeyframes();
    internal::writeBytes(reinterpret_cast<uint8_t *>(&nMorphFrames), sizeof(nMorphFrames), data);
    for (int i = 0; i < nMorphFrames; i++) {
        MorphKeyframe *keyframe = m_morphMotion.keyframeAt(i);
        keyframe->write(data);
        data += MorphKeyframe::strideSize();
    }
    int nCameraFrames = m_cameraMotion.countKeyframes();
    internal::writeBytes(reinterpret_cast<uint8_t *>(&nCameraFrames), sizeof(nCameraFrames), data);
    for (int i = 0; i < nCameraFrames; i++) {
        CameraKeyframe *keyframe = m_cameraMotion.frameAt(i);
        keyframe->write(data);
        data += CameraKeyframe::strideSize();
    }
    int nLightFrames = m_lightMotion.countKeyframes();
    internal::writeBytes(reinterpret_cast<uint8_t *>(&nLightFrames), sizeof(nLightFrames), data);
    for (int i = 0; i < nLightFrames; i++) {
        LightKeyframe *keyframe = m_lightMotion.frameAt(i);
        keyframe->write(data);
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

void Motion::setParentSceneRef(Scene *value)
{
    m_parentSceneRef = value;
}

void Motion::setParentModelRef(IModel *value)
{
    m_boneMotion.setParentModel(value);
    m_morphMotion.setParentModel(value);
    m_parentModelRef = value;
    if (value) {
        const IString *name = value->name();
        if (name)
            internal::setString(name, m_name);
    }
}

void Motion::seek(const IKeyframe::TimeIndex &timeIndex)
{
    m_boneMotion.seek(timeIndex);
    m_morphMotion.seek(timeIndex);
    m_active = maxTimeIndex() > timeIndex;
}

void Motion::seekScene(const IKeyframe::TimeIndex &timeIndex, Scene *scene)
{
    if (m_cameraMotion.countKeyframes() > 0) {
        m_cameraMotion.seek(timeIndex);
        ICamera *camera = scene->camera();
        camera->setLookAt(m_cameraMotion.position());
        camera->setAngle(m_cameraMotion.angle());
        camera->setFov(m_cameraMotion.fovy());
        camera->setDistance(m_cameraMotion.distance());
    }
    if (m_lightMotion.countKeyframes() > 0) {
        m_lightMotion.seek(timeIndex);
        ILight *light = scene->light();
        light->setColor(m_lightMotion.color());
        light->setDirection(m_lightMotion.direction());
    }
}

void Motion::advance(const IKeyframe::TimeIndex &deltaTimeIndex)
{
    if (deltaTimeIndex == 0) {
        m_boneMotion.advance(deltaTimeIndex);
        m_morphMotion.advance(deltaTimeIndex);
    }
    else if (m_active) {
        // The motion is active and continue to advance
        m_boneMotion.advance(deltaTimeIndex);
        m_morphMotion.advance(deltaTimeIndex);
        if (m_boneMotion.currentTimeIndex() >= m_boneMotion.maxTimeIndex() &&
                m_morphMotion.currentTimeIndex() >= m_morphMotion.maxTimeIndex()) {
            m_active = false;
        }
    }
}

void Motion::advanceScene(const IKeyframe::TimeIndex &deltaTimeIndex, Scene *scene)
{
    if (m_cameraMotion.countKeyframes() > 0) {
        m_cameraMotion.advance(deltaTimeIndex);
        ICamera *camera = scene->camera();
        camera->setLookAt(m_cameraMotion.position());
        camera->setAngle(m_cameraMotion.angle());
        camera->setFov(m_cameraMotion.fovy());
        camera->setDistance(m_cameraMotion.distance());
    }
    if (m_lightMotion.countKeyframes() > 0) {
        m_lightMotion.advance(deltaTimeIndex);
        ILight *light = scene->light();
        light->setColor(m_lightMotion.color());
        light->setDirection(m_lightMotion.direction());
    }
}

void Motion::reload()
{
    /* rebuild internal keyframe nodes */
    m_boneMotion.setParentModel(m_parentModelRef);
    m_morphMotion.setParentModel(m_parentModelRef);
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

IKeyframe::TimeIndex Motion::maxTimeIndex() const
{
    return btMax(btMax(btMax(m_boneMotion.maxTimeIndex(), m_morphMotion.maxTimeIndex()), m_cameraMotion.maxTimeIndex()), m_lightMotion.maxTimeIndex());
}

bool Motion::isReachedTo(const IKeyframe::TimeIndex &atEnd) const
{
    // force inactive motion is reached
    return !m_active || (m_boneMotion.currentTimeIndex() >= atEnd && m_morphMotion.currentTimeIndex() >= atEnd);
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
    if (!value || value->layerIndex() != 0)
        return;
    switch (value->type()) {
    case IKeyframe::kBoneKeyframe:
        m_boneMotion.addKeyframe(value);
        break;
    case IKeyframe::kCameraKeyframe:
        m_cameraMotion.addKeyframe(value);
        break;
    case IKeyframe::kLightKeyframe:
        m_lightMotion.addKeyframe(value);
        break;
    case IKeyframe::kMorphKeyframe:
        m_morphMotion.addKeyframe(value);
        break;
    default:
        break;
    }
}

void Motion::replaceKeyframe(IKeyframe *value)
{
    if (!value || value->layerIndex() != 0)
        return;
    switch (value->type()) {
    case IKeyframe::kBoneKeyframe: {
        IKeyframe *keyframeToDelete = m_boneMotion.findKeyframe(value->timeIndex(), value->name());
        if (keyframeToDelete)
            m_boneMotion.deleteKeyframe(keyframeToDelete);
        m_boneMotion.addKeyframe(value);
        update(IKeyframe::kBoneKeyframe);
        break;
    }
    case IKeyframe::kCameraKeyframe: {
        IKeyframe *keyframeToDelete = m_cameraMotion.findKeyframe(value->timeIndex());
        if (keyframeToDelete)
            m_cameraMotion.deleteKeyframe(keyframeToDelete);
        m_cameraMotion.addKeyframe(value);
        update(IKeyframe::kCameraKeyframe);
        break;
    }
    case IKeyframe::kLightKeyframe: {
        IKeyframe *keyframeToDelete = m_lightMotion.findKeyframe(value->timeIndex());
        if (keyframeToDelete)
            m_lightMotion.deleteKeyframe(keyframeToDelete);
        m_lightMotion.addKeyframe(value);
        update(IKeyframe::kLightKeyframe);
        break;
    }
    case IKeyframe::kMorphKeyframe: {
        IKeyframe *keyframeToDelete = m_morphMotion.findKeyframe(value->timeIndex(), value->name());
        if (keyframeToDelete)
            m_morphMotion.deleteKeyframe(keyframeToDelete);
        m_morphMotion.addKeyframe(value);
        update(IKeyframe::kMorphKeyframe);
        break;
    }
    default:
        break;
    }
}

int Motion::countKeyframes(IKeyframe::Type value) const
{
    switch (value) {
    case IKeyframe::kBoneKeyframe:
        return m_boneMotion.countKeyframes();
    case IKeyframe::kCameraKeyframe:
        return m_cameraMotion.countKeyframes();
    case IKeyframe::kLightKeyframe:
        return m_lightMotion.countKeyframes();
    case IKeyframe::kMorphKeyframe:
        return m_morphMotion.countKeyframes();
    default:
        return 0;
    }
}

void Motion::getKeyframes(const IKeyframe::TimeIndex &timeIndex,
                          const IKeyframe::LayerIndex &layerIndex,
                          IKeyframe::Type type,
                          Array<IKeyframe *> &keyframes)
{
    if (layerIndex != -1 && layerIndex != 0)
        return;
    switch (type) {
    case IKeyframe::kBoneKeyframe:
        m_boneMotion.getKeyframes(timeIndex, keyframes);
        break;
    case IKeyframe::kCameraKeyframe:
        m_cameraMotion.getKeyframes(timeIndex, keyframes);
        break;
    case IKeyframe::kLightKeyframe:
        m_lightMotion.getKeyframes(timeIndex, keyframes);
        break;
    case IKeyframe::kMorphKeyframe:
        m_morphMotion.getKeyframes(timeIndex, keyframes);
        break;
    default:
        break;
    }
}

IKeyframe::LayerIndex Motion::countLayers(const IString * /* name */,
                                          IKeyframe::Type /* type */) const
{
    return 1;
}

IBoneKeyframe *Motion::findBoneKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                        const IString *name,
                                        const IKeyframe::LayerIndex &layerIndex) const
{
    return layerIndex == 0 ? m_boneMotion.findKeyframe(timeIndex, name) : 0;
}

IBoneKeyframe *Motion::findBoneKeyframeAt(int index) const
{
    return m_boneMotion.keyframeAt(index);
}

ICameraKeyframe *Motion::findCameraKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                            const IKeyframe::LayerIndex &layerIndex) const
{
    return layerIndex == 0 ? m_cameraMotion.findKeyframe(timeIndex) : 0;
}

ICameraKeyframe *Motion::findCameraKeyframeAt(int index) const
{
    return m_cameraMotion.frameAt(index);
}

IEffectKeyframe *Motion::findEffectKeyframe(const IKeyframe::TimeIndex & /* timeIndex */,
                                            const IString * /* name */,
                                            const IKeyframe::LayerIndex & /* layerIndex */) const
{
    return 0;
}

IEffectKeyframe *Motion::findEffectKeyframeAt(int /* index */) const
{
    return 0;
}

ILightKeyframe *Motion::findLightKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                          const IKeyframe::LayerIndex &layerIndex) const
{
    return layerIndex == 0 ? m_lightMotion.findKeyframe(timeIndex) : 0;
}

ILightKeyframe *Motion::findLightKeyframeAt(int index) const
{
    return m_lightMotion.frameAt(index);
}

IModelKeyframe *Motion::findModelKeyframe(const IKeyframe::TimeIndex & /* timeIndex */,
                                          const IKeyframe::LayerIndex & /* layerIndex */) const
{
    return 0;
}

IModelKeyframe *Motion::findModelKeyframeAt(int /* index */) const
{
    return 0;
}

IMorphKeyframe *Motion::findMorphKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                          const IString *name,
                                          const IKeyframe::LayerIndex &layerIndex) const
{
    return layerIndex == 0 ? m_morphMotion.findKeyframe(timeIndex, name) : 0;
}

IMorphKeyframe *Motion::findMorphKeyframeAt(int index) const
{
    return m_morphMotion.keyframeAt(index);
}

IProjectKeyframe *Motion::findProjectKeyframe(const IKeyframe::TimeIndex & /* timeIndex */,
                                              const IKeyframe::LayerIndex & /* layerIndex */) const
{
    return 0;
}

IProjectKeyframe *Motion::findProjectKeyframeAt(int /* index */) const
{
    return 0;
}

void Motion::deleteKeyframe(IKeyframe *&value)
{
    /* prevent deleting a null keyframe and timeIndex() of the keyframe is zero */
    if (!value || value->timeIndex() == 0)
        return;
    switch (value->type()) {
    case IKeyframe::kBoneKeyframe:
        m_boneMotion.deleteKeyframe(value);
        update(IKeyframe::kBoneKeyframe);
        value = 0;
        break;
    case IKeyframe::kCameraKeyframe:
        m_cameraMotion.deleteKeyframe(value);
        update(IKeyframe::kCameraKeyframe);
        value = 0;
        break;
    case IKeyframe::kLightKeyframe:
        m_lightMotion.deleteKeyframe(value);
        update(IKeyframe::kLightKeyframe);
        value = 0;
        break;
    case IKeyframe::kMorphKeyframe:
        m_morphMotion.deleteKeyframe(value);
        update(IKeyframe::kMorphKeyframe);
        value = 0;
        break;
    default:
        break;
    }
}

void Motion::update(IKeyframe::Type type)
{
    switch (type) {
    case IKeyframe::kBoneKeyframe:
        m_boneMotion.setParentModel(m_parentModelRef);
        break;
    case IKeyframe::kCameraKeyframe:
        m_cameraMotion.update();
        break;
    case IKeyframe::kLightKeyframe:
        m_lightMotion.update();
        break;
    case IKeyframe::kMorphKeyframe:
        m_morphMotion.setParentModel(m_parentModelRef);
        break;
    default:
        break;
    }
}

IMotion *Motion::clone() const
{
    IMotion *dest = m_motionPtr = new Motion(m_parentModelRef, m_encodingRef);
    const int nbkeyframes = m_boneMotion.countKeyframes();
    for (int i = 0; i < nbkeyframes; i++) {
        BoneKeyframe *keyframe = m_boneMotion.keyframeAt(i);
        dest->addKeyframe(keyframe->clone());
    }
    const int nckeyframes = m_cameraMotion.countKeyframes();
    for (int i = 0; i < nckeyframes; i++) {
        CameraKeyframe *keyframe = m_cameraMotion.frameAt(i);
        dest->addKeyframe(keyframe->clone());
    }
    const int nlkeyframes = m_lightMotion.countKeyframes();
    for (int i = 0; i < nlkeyframes; i++) {
        LightKeyframe *keyframe = m_lightMotion.frameAt(i);
        dest->addKeyframe(keyframe->clone());
    }
    const int nmkeyframes = m_morphMotion.countKeyframes();
    for (int i = 0; i < nmkeyframes; i++) {
        MorphKeyframe *keyframe = m_morphMotion.keyframeAt(i);
        dest->addKeyframe(keyframe->clone());
    }
    m_motionPtr = 0;
    return dest;
}

void Motion::parseHeader(const DataInfo &info)
{
    m_name = m_encodingRef->toString(info.namePtr, IString::kShiftJIS, kNameSize);
}

void Motion::parseBoneFrames(const DataInfo &info)
{
    m_boneMotion.read(info.boneKeyframePtr, info.boneKeyframeCount);
    m_boneMotion.setParentModel(m_parentModelRef);
}

void Motion::parseMorphFrames(const DataInfo &info)
{
    m_morphMotion.read(info.morphKeyframePtr, info.morphKeyframeCount);
    m_morphMotion.setParentModel(m_parentModelRef);
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
    delete m_motionPtr;
    m_parentSceneRef = 0;
    m_motionPtr = 0;
    m_error = kNoError;
    m_active = false;
}

}
}

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

#include "vpvl2/mvd/AssetSection.h"
#include "vpvl2/mvd/BoneSection.h"
#include "vpvl2/mvd/CameraSection.h"
#include "vpvl2/mvd/EffectSection.h"
#include "vpvl2/mvd/LightSection.h"
#include "vpvl2/mvd/ModelKeyframe.h"
#include "vpvl2/mvd/ModelSection.h"
#include "vpvl2/mvd/MorphSection.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/mvd/NameListSection.h"
#include "vpvl2/mvd/ProjectSection.h"

namespace
{

using namespace vpvl2;

#pragma pack(push, 1)

struct Header {
    vpvl2::uint8_t signature[30];
    float32_t version;
    vpvl2::uint8_t encoding;
};

#pragma pack(pop)

template<typename S, typename K>
static void AddAllKeyframes(const S *section, IMotion *motion)
{
    int nkeyframes = section->countKeyframes();
    for (int i = 0; i < nkeyframes; i++) {
        const K *keyframe = section->findKeyframeAt(i);
        motion->addKeyframe(keyframe->clone());
    }
}

}

namespace vpvl2
{
namespace mvd
{

const uint8_t *Motion::kSignature = reinterpret_cast<const uint8_t *>("Motion Vector Data file");

struct Motion::PrivateContext {
    PrivateContext(IModel *modelRef, IEncoding *encodingRef, Motion *self)
        : motionPtr(0),
          selfPtr(self),
          assetSection(0),
          boneSection(0),
          cameraSection(0),
          effectSection(0),
          lightSection(0),
          modelSection(0),
          morphSection(0),
          nameListSection(0),
          projectSection(0),
          parentSceneRef(0),
          parentModelRef(modelRef),
          encodingRef(encodingRef),
          name(0),
          name2(0),
          reserved(0),
          error(kNoError),
          active(true)
    {
        nameListSection = new NameListSection(encodingRef);
    }
    ~PrivateContext() {
        release();
    }

    void initialize() {
        assetSection = new AssetSection(selfPtr);
        boneSection = new BoneSection(selfPtr, parentModelRef);
        cameraSection = new CameraSection(selfPtr);
        effectSection = new EffectSection(selfPtr);
        lightSection = new LightSection(selfPtr);
        modelSection = new ModelSection(selfPtr, parentModelRef, 0);
        morphSection = new MorphSection(selfPtr, parentModelRef);
        projectSection = new ProjectSection(selfPtr);
        type2sectionRefs.insert(IKeyframe::kAssetKeyframe, assetSection);
        type2sectionRefs.insert(IKeyframe::kBoneKeyframe, boneSection);
        type2sectionRefs.insert(IKeyframe::kCameraKeyframe, cameraSection);
        type2sectionRefs.insert(IKeyframe::kEffectKeyframe, effectSection);
        type2sectionRefs.insert(IKeyframe::kLightKeyframe, lightSection);
        type2sectionRefs.insert(IKeyframe::kModelKeyframe, modelSection);
        type2sectionRefs.insert(IKeyframe::kMorphKeyframe, morphSection);
        type2sectionRefs.insert(IKeyframe::kProjectKeyframe, projectSection);
    }
    void parseHeader(const Motion::DataInfo &info) {
        IEncoding *encoding = info.encoding;
        internal::setStringDirect(encoding->toString(info.namePtr, info.nameSize, info.codec), name);
        internal::setStringDirect(encoding->toString(info.name2Ptr, info.name2Size, info.codec), name2);
        internal::setStringDirect(encoding->toString(info.reservedPtr, info.reservedSize, info.codec), reserved);
        nameListSection = new NameListSection(encodingRef);
        nameListSection->read(info.nameListSectionPtr, info.codec);
    }
    void parseAssetSections(const Motion::DataInfo &info) {
        const Array<uint8_t *> &sections = info.assetSectionPtrs;
        const int nsections = sections.count();
        assetSection = new AssetSection(selfPtr);
        type2sectionRefs.insert(IKeyframe::kAssetKeyframe, assetSection);
        for (int i = 0; i < nsections; i++) {
            const uint8_t *ptr = sections[i];
            assetSection->read(ptr);
        }
    }
    void parseBoneSections(const Motion::DataInfo &info) {
        const Array<uint8_t *> &sections = info.boneSectionPtrs;
        const int nsections = sections.count();
        boneSection = new BoneSection(selfPtr, parentModelRef);
        type2sectionRefs.insert(IKeyframe::kBoneKeyframe, boneSection);
        for (int i = 0; i < nsections; i++) {
            const uint8_t *ptr = sections[i];
            boneSection->read(ptr);
        }
    }
    void parseCameraSections(const Motion::DataInfo &info) {
        const Array<uint8_t *> &sections = info.cameraSectionPtrs;
        const int nsections = sections.count();
        cameraSection = new CameraSection(selfPtr);
        type2sectionRefs.insert(IKeyframe::kCameraKeyframe, cameraSection);
        for (int i = 0; i < nsections; i++) {
            const uint8_t *ptr = sections[i];
            cameraSection->read(ptr);
        }
    }
    void parseEffectSections(const Motion::DataInfo &info) {
        const Array<uint8_t *> &sections = info.effectSectionPtrs;
        const int nsections = sections.count();
        effectSection = new EffectSection(selfPtr);
        type2sectionRefs.insert(IKeyframe::kEffectKeyframe, effectSection);
        for (int i = 0; i < nsections; i++) {
            const uint8_t *ptr = sections[i];
            effectSection->read(ptr);
        }
    }
    void parseLightSections(const Motion::DataInfo &info) {
        const Array<uint8_t *> &sections = info.lightSectionPtrs;
        const int nsections = sections.count();
        lightSection = new LightSection(selfPtr);
        type2sectionRefs.insert(IKeyframe::kLightKeyframe, lightSection);
        for (int i = 0; i < nsections; i++) {
            const uint8_t *ptr = sections[i];
            lightSection->read(ptr);
        }
    }
    void parseModelSections(const Motion::DataInfo &info) {
        const Array<uint8_t *> &sections = info.modelSectionPtrs;
        const int nsections = sections.count();
        modelSection = new ModelSection(selfPtr, parentModelRef, info.adjustAlignment);
        type2sectionRefs.insert(IKeyframe::kModelKeyframe, modelSection);
        for (int i = 0; i < nsections; i++) {
            const uint8_t *ptr = sections[i];
            modelSection->read(ptr);
        }
    }
    void parseMorphSections(const Motion::DataInfo &info) {
        const Array<uint8_t *> &sections = info.morphSectionPtrs;
        const int nsections = sections.count();
        morphSection = new MorphSection(selfPtr, parentModelRef);
        type2sectionRefs.insert(IKeyframe::kMorphKeyframe, morphSection);
        for (int i = 0; i < nsections; i++) {
            const uint8_t *ptr = sections[i];
            morphSection->read(ptr);
        }
    }
    void parseProjectSections(const Motion::DataInfo &info) {
        const Array<uint8_t *> &sections = info.projectSectionPtrs;
        const int nsections = sections.count();
        projectSection = new ProjectSection(selfPtr);
        type2sectionRefs.insert(IKeyframe::kProjectKeyframe, projectSection);
        for (int i = 0; i < nsections; i++) {
            const uint8_t *ptr = sections[i];
            projectSection->read(ptr);
        }
    }
    void release() {
        for (int i = 0; i < IKeyframe::kMaxKeyframeType; i++) {
            type2sectionRefs.remove(i);
        }
        delete motionPtr;
        motionPtr = 0;
        delete assetSection;
        assetSection = 0;
        delete boneSection;
        boneSection = 0;
        delete cameraSection;
        cameraSection = 0;
        delete effectSection;
        effectSection = 0;
        delete lightSection;
        lightSection = 0;
        delete modelSection;
        modelSection = 0;
        delete morphSection;
        morphSection = 0;
        delete nameListSection;
        nameListSection = 0;
        delete projectSection;
        projectSection = 0;
        delete name;
        name = 0;
        delete name2;
        name2 = 0;
        delete reserved;
        reserved = 0;
        parentSceneRef = 0;
        error = kNoError;
        active = false;
    }

    mutable IMotion *motionPtr;
    Motion *selfPtr;
    AssetSection *assetSection;
    BoneSection *boneSection;
    CameraSection *cameraSection;
    EffectSection *effectSection;
    LightSection *lightSection;
    ModelSection *modelSection;
    MorphSection *morphSection;
    NameListSection *nameListSection;
    ProjectSection *projectSection;
    Scene *parentSceneRef;
    IModel *parentModelRef;
    IEncoding *encodingRef;
    IString *name;
    IString *name2;
    IString *reserved;
    Motion::DataInfo info;
    Hash<HashInt, BaseSection *> type2sectionRefs;
    Motion::Error error;
    bool active;
};

//
// implemented
// - Bone
// - Camera
// - Morph
//
// NOT implemented
// - Asset
// - Effect
// - Light
// - Model
// - Project

Motion::Motion(IModel *modelRef, IEncoding *encodingRef)
    : m_context(0)
{
    m_context = new PrivateContext(modelRef, encodingRef, this);
    m_context->initialize();
}

Motion::~Motion()
{
    m_context->release();
    delete m_context;
    m_context = 0;
}

bool Motion::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    // Header(30)
    Header header;
    if (!data || sizeof(header) > rest) {
        VPVL2_LOG(WARNING, "Data is null or MVD header not satisfied: " << size);
        m_context->error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    info.basePtr = ptr;
    VPVL2_VLOG(1, "MVDBasePtr: ptr=" << static_cast<const void*>(ptr) << " size=" << size);

    // Check the signature is valid
    internal::getData(ptr, header);
    if (memcmp(header.signature, kSignature, sizeof(kSignature) - 1) != 0) {
        VPVL2_LOG(WARNING, "Invalid MVD signature detected: " << header.signature);
        m_context->error = kInvalidSignatureError;
        return false;
    }
    if (header.version != 1.0) {
        VPVL2_LOG(WARNING, "Invalid MVD version detected: " << header.version);
        m_context->error = kInvalidVersionError;
        return false;
    }
    if (header.encoding != 0 && header.encoding != 1) {
        VPVL2_LOG(WARNING, "Invalid MVD encoding detected: " << header.encoding);
        m_context->error = kInvalidEncodingError;
        return false;
    }
    info.codec = header.encoding == 0 ? IString::kUTF16 : IString::kUTF8;
    ptr += sizeof(header);
    rest -= sizeof(header);
    VPVL2_VLOG(1, "MVDHeader: encoding=" << int(header.encoding));

    /* object name */
    if (!internal::getText(ptr, rest, info.namePtr, info.nameSize)) {
        VPVL2_LOG(WARNING, "Invalid size of MVD object name detected: " << info.nameSize);
        return false;
    }
    /* object name2 */
    if (!internal::getText(ptr, rest, info.name2Ptr, info.name2Size)) {
        VPVL2_LOG(WARNING, "Invalid size of MVD object name 2 detected: " << info.name2Size);
        return false;
    }
    /* scene FPS (not support yet) */
    if (!internal::validateSize(ptr, sizeof(float32_t), rest)) {
        VPVL2_LOG(WARNING, "FPS not satisfied: " << rest);
        return false;
    }
    info.fpsPtr = ptr;
    internal::getData(ptr, info.fps);
    /* reserved */
    if (!internal::getText(ptr, rest, info.reservedPtr, info.reservedSize)) {
        VPVL2_LOG(WARNING, "Invalid size of MVD header reserved area detected: " << info.reservedSize);
        return false;
    }
    info.sectionStartPtr = ptr;
    VPVL2_VLOG(1, "MVDSectionBegin: ptr=" << static_cast<const void*>(ptr) << " size=" << size);

    /* sections */
    bool ret = false;
    SectionTag sectionHeader;
    while (rest > 0) {
        if (!internal::getTyped<SectionTag>(ptr, rest, sectionHeader)) {
            VPVL2_LOG(WARNING, "Invalid section header detected: rest=" << rest);
            return false;
        }
        VPVL2_VLOG(3, "MVDSectionHeader: type=" << int(sectionHeader.type) << " minor=" << int(sectionHeader.minor));
        uint8_t *startPtr = ptr;
        switch (static_cast<SectionType>(sectionHeader.type)) {
        case kNameListSection: {
            VPVL2_VLOG(1, "MVDNameListSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            if (!NameListSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.nameListSectionPtr = startPtr;
            break;
        }
        case kBoneSection: {
            VPVL2_VLOG(3, "MVDBoneSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            if (!BoneSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.boneSectionPtrs.append(startPtr);
            break;
        }
        case kMorphSection: {
            VPVL2_VLOG(3, "MVDMorphSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            if (!MorphSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.morphSectionPtrs.append(startPtr);
            break;
        }
        case kModelSection: {
            VPVL2_VLOG(3, "MVDModelSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            info.adjustAlignment = sectionHeader.minor == 1 ? 4 : 0;
            if (!ModelSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.modelSectionPtrs.append(startPtr);
            break;
        }
        case kAssetSection: {
            VPVL2_VLOG(3, "MVDAssetSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            if (!AssetSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.assetSectionPtrs.append(startPtr);
            break;
        }
        case kEffectSection: {
            VPVL2_VLOG(3, "MVDEffectSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            if (!EffectSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.effectSectionPtrs.append(startPtr);
            break;
        }
        case kCameraSection: {
            VPVL2_VLOG(3, "MVDCameraSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            if (!CameraSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.cameraSectionPtrs.append(startPtr);
            break;
        }
        case kLightSection: {
            VPVL2_VLOG(3, "MVDLightSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            if (!LightSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.lightSectionPtrs.append(startPtr);
            break;
        }
        case kProjectSection: {
            VPVL2_VLOG(3, "MVDProjectSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            if (!ProjectSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.projectSectionPtrs.append(startPtr);
            break;
        }
        case kEndOfFile: {
            VPVL2_VLOG(1, "MVDEndOfDataSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            ret = true;
            rest = 0;
            info.encoding = m_context->encodingRef;
            info.endPtr = ptr;
            break;
        }
        default:
            VPVL2_LOG(WARNING, "MVDUnknownSection: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);
            rest = 0;
            info.endPtr = 0;
            break;
        }
    }
    return ret;
}

bool Motion::load(const uint8_t *data, size_t size)
{
    DataInfo info;
    internal::zerofill(&info, sizeof(info));
    if (preparse(data, size, info)) {
        m_context->release();
        m_context->parseHeader(info);
        m_context->parseAssetSections(info);
        m_context->parseBoneSections(info);
        m_context->parseCameraSections(info);
        m_context->parseEffectSections(info);
        m_context->parseLightSections(info);
        m_context->parseModelSections(info);
        m_context->parseMorphSections(info);
        m_context->parseProjectSections(info);
        m_context->info.copy(info);
        return true;
    }
    return false;
}

void Motion::save(uint8_t *data) const
{
    Header header;
    IString::Codec codec = m_context->info.codec;
    internal::zerofill(&header, sizeof(header));
    memcpy(header.signature, kSignature, sizeof(header.signature) - 1);
    header.version = 1.0;
    header.encoding = 1;
    internal::writeBytes(&header, sizeof(header), data);
    internal::writeString(m_context->name, codec, data);
    internal::writeString(m_context->name2, codec, data);
    float32_t fps = 30;
    internal::writeBytes(&fps, sizeof(fps), data);
    internal::writeString(m_context->reserved, codec, data);
    m_context->nameListSection->write(data, m_context->info);
    data += m_context->nameListSection->estimateSize(m_context->info);
    m_context->boneSection->write(data);
    data += m_context->boneSection->estimateSize();
    m_context->morphSection->write(data);
    data += m_context->morphSection->estimateSize();
    /* to update IK bones enable/disable state */
    m_context->modelSection->setParentModel(m_context->parentModelRef);
    m_context->modelSection->write(data);
    data += m_context->modelSection->estimateSize();
    m_context->assetSection->write(data);
    data += m_context->assetSection->estimateSize();
    m_context->effectSection->write(data);
    data += m_context->effectSection->estimateSize();
    m_context->cameraSection->write(data);
    data += m_context->cameraSection->estimateSize();
    m_context->lightSection->write(data);
    data += m_context->lightSection->estimateSize();
    m_context->projectSection->write(data);
    data += m_context->projectSection->estimateSize();
    SectionTag eof;
    eof.type = kEndOfFile;
    eof.minor = 0;
    internal::writeBytes(&eof, sizeof(eof), data);
}

size_t Motion::estimateSize() const
{
    size_t size = 0;
    IString::Codec codec = m_context->info.codec;
    size += sizeof(Header);
    size += internal::estimateSize(m_context->name, codec);
    size += internal::estimateSize(m_context->name2, codec);
    size += sizeof(float32_t);
    size += internal::estimateSize(m_context->reserved, codec);
    size += m_context->nameListSection->estimateSize(m_context->info);
    size += m_context->boneSection->estimateSize();
    size += m_context->morphSection->estimateSize();
    size += m_context->modelSection->estimateSize();
    size += m_context->assetSection->estimateSize();
    size += m_context->effectSection->estimateSize();
    size += m_context->cameraSection->estimateSize();
    size += m_context->lightSection->estimateSize();
    size += m_context->projectSection->estimateSize();
    size += sizeof(SectionTag);
    return size;
}

void Motion::setParentSceneRef(Scene *value)
{
    m_context->parentSceneRef = value;
}

void Motion::setParentModelRef(IModel *value)
{
    m_context->parentModelRef = value;
    m_context->boneSection->setParentModel(value);
    m_context->modelSection->setParentModel(value);
    m_context->morphSection->setParentModel(value);
}

void Motion::seek(const IKeyframe::TimeIndex &timeIndex)
{
    m_context->assetSection->seek(timeIndex);
    m_context->boneSection->seek(timeIndex);
    m_context->modelSection->seek(timeIndex);
    m_context->morphSection->seek(timeIndex);
    m_context->active = duration() > timeIndex;
}

void Motion::seekScene(const IKeyframe::TimeIndex &timeIndex, Scene *scene)
{
    if (m_context->cameraSection->countKeyframes() > 0) {
        m_context->cameraSection->seek(timeIndex);
        ICamera *camera = scene->camera();
        camera->setLookAt(m_context->cameraSection->position());
        camera->setAngle(m_context->cameraSection->angle());
        camera->setFov(m_context->cameraSection->fov());
        camera->setDistance(m_context->cameraSection->distance());
    }
    if (m_context->lightSection->countKeyframes() > 0) {
        m_context->lightSection->seek(timeIndex);
        ILight *light = scene->light();
        light->setColor(m_context->lightSection->color());
        light->setDirection(m_context->lightSection->direction());
    }
}

void Motion::advance(const IKeyframe::TimeIndex &deltaTimeIndex)
{
    m_context->assetSection->advance(deltaTimeIndex);
    m_context->boneSection->advance(deltaTimeIndex);
    m_context->effectSection->advance(deltaTimeIndex);
    m_context->modelSection->advance(deltaTimeIndex);
    m_context->morphSection->advance(deltaTimeIndex);
    if (deltaTimeIndex > 0) {
        m_context->active = !isReachedTo(duration());
    }
}

void Motion::advanceScene(const IKeyframe::TimeIndex &deltaTimeIndex, Scene *scene)
{
    if (m_context->cameraSection->countKeyframes() > 0) {
        m_context->cameraSection->advance(deltaTimeIndex);
        ICamera *camera = scene->camera();
        camera->setLookAt(m_context->cameraSection->position());
        camera->setAngle(m_context->cameraSection->angle());
        camera->setFov(m_context->cameraSection->fov());
        camera->setDistance(m_context->cameraSection->distance());
    }
    if (m_context->lightSection->countKeyframes() > 0) {
        m_context->lightSection->advance(deltaTimeIndex);
        ILight *light = scene->light();
        light->setColor(m_context->lightSection->color());
        light->setDirection(m_context->lightSection->direction());
    }
}

void Motion::reload()
{
}

void Motion::reset()
{
    m_context->assetSection->seek(0);
    m_context->boneSection->seek(0);
    m_context->cameraSection->seek(0);
    m_context->effectSection->seek(0);
    m_context->lightSection->seek(0);
    m_context->modelSection->seek(0);
    m_context->morphSection->seek(0);
    m_context->projectSection->seek(0);
    m_context->active = true;
}

IKeyframe::TimeIndex Motion::duration() const
{
    IKeyframe::TimeIndex maxTimeIndex = 0;
    btSetMax(maxTimeIndex, m_context->assetSection->maxTimeIndex());
    btSetMax(maxTimeIndex, m_context->boneSection->maxTimeIndex());
    btSetMax(maxTimeIndex, m_context->cameraSection->maxTimeIndex());
    btSetMax(maxTimeIndex, m_context->effectSection->maxTimeIndex());
    btSetMax(maxTimeIndex, m_context->lightSection->maxTimeIndex());
    btSetMax(maxTimeIndex, m_context->modelSection->maxTimeIndex());
    btSetMax(maxTimeIndex, m_context->morphSection->maxTimeIndex());
    btSetMax(maxTimeIndex, m_context->projectSection->maxTimeIndex());
    return maxTimeIndex;
}

bool Motion::isReachedTo(const IKeyframe::TimeIndex &atEnd) const
{
    if (m_context->active) {
        return internal::isReachedToMax(*m_context->assetSection, atEnd) &&
                internal::isReachedToMax(*m_context->boneSection, atEnd) &&
                internal::isReachedToMax(*m_context->cameraSection, atEnd) &&
                internal::isReachedToMax(*m_context->effectSection, atEnd) &&
                internal::isReachedToMax(*m_context->lightSection, atEnd) &&
                internal::isReachedToMax(*m_context->modelSection, atEnd) &&
                internal::isReachedToMax(*m_context->morphSection, atEnd) &&
                internal::isReachedToMax(*m_context->projectSection, atEnd);
    }
    return true;
}

bool Motion::isNullFrameEnabled() const
{
    return false;
}

void Motion::setNullFrameEnable(bool /* value */)
{
}

void Motion::addKeyframe(IKeyframe *value)
{
    if (!value) {
        return;
    }
    if (BaseSection *const *sectionPtr = m_context->type2sectionRefs.find(value->type())) {
        BaseSection *section = *sectionPtr;
        section->addKeyframe(value);
    }
}

void Motion::replaceKeyframe(IKeyframe *value)
{
    if (!value) {
        return;
    }
    switch (value->type()) {
    case IKeyframe::kAssetKeyframe: {
        break;
    }
    case IKeyframe::kBoneKeyframe: {
        IKeyframe *prev = m_context->boneSection->findKeyframe(value->timeIndex(), value->name(), value->layerIndex());
        m_context->boneSection->deleteKeyframe(prev);
        m_context->boneSection->addKeyframe(value);
        break;
    }
    case IKeyframe::kCameraKeyframe: {
        IKeyframe *prev = m_context->cameraSection->findKeyframe(value->timeIndex(), value->layerIndex());
        m_context->cameraSection->deleteKeyframe(prev);
        m_context->cameraSection->addKeyframe(value);
        break;
    }
    case IKeyframe::kEffectKeyframe: {
        IKeyframe *prev = m_context->effectSection->findKeyframe(value->timeIndex(), value->name(), value->layerIndex());
        m_context->effectSection->deleteKeyframe(prev);
        m_context->effectSection->addKeyframe(value);
        break;
    }
    case IKeyframe::kLightKeyframe: {
        IKeyframe *prev = m_context->lightSection->findKeyframe(value->timeIndex(), value->layerIndex());
        m_context->lightSection->deleteKeyframe(prev);
        m_context->lightSection->addKeyframe(value);
        break;
    }
    case IKeyframe::kModelKeyframe: {
        IKeyframe *prev = m_context->modelSection->findKeyframe(value->timeIndex(), value->layerIndex());
        m_context->modelSection->deleteKeyframe(prev);
        m_context->modelSection->addKeyframe(value);
        break;
    }
    case IKeyframe::kMorphKeyframe: {
        IKeyframe *prev = m_context->morphSection->findKeyframe(value->timeIndex(), value->name(), value->layerIndex());
        m_context->morphSection->deleteKeyframe(prev);
        m_context->morphSection->addKeyframe(value);
        break;
    }
    case IKeyframe::kProjectKeyframe: {
        IKeyframe *prev = m_context->projectSection->findKeyframe(value->timeIndex(), value->layerIndex());
        m_context->projectSection->deleteKeyframe(prev);
        m_context->projectSection->addKeyframe(value);
        break;
    }
    default:
        break;
    }
}

int Motion::countKeyframes(IKeyframe::Type value) const
{
    int count = 0;
    if (const BaseSection *const *sectionPtr = m_context->type2sectionRefs.find(value)) {
        const BaseSection *section = *sectionPtr;
        count = section->countKeyframes();
    }
    return count;
}

IKeyframe::LayerIndex Motion::countLayers(const IString *name,
                                          IKeyframe::Type type) const
{
    switch (type) {
    case IKeyframe::kBoneKeyframe:
        return m_context->boneSection->countLayers(name);
    case IKeyframe::kCameraKeyframe:
        return m_context->cameraSection->countLayers();
    default:
        return 1;
    }
}

void Motion::getKeyframeRefs(const IKeyframe::TimeIndex &timeIndex,
                          const IKeyframe::LayerIndex &layerIndex,
                          IKeyframe::Type type,
                          Array<IKeyframe *> &keyframes)
{
    if (const BaseSection *const *sectionPtr = m_context->type2sectionRefs.find(type)) {
        const BaseSection *section = *sectionPtr;
        section->getKeyframes(timeIndex, layerIndex, keyframes);
    }
}

IBoneKeyframe *Motion::findBoneKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                        const IString *name,
                                        const IKeyframe::LayerIndex &layerIndex) const
{
    return m_context->boneSection->findKeyframe(timeIndex, name, layerIndex);
}

IBoneKeyframe *Motion::findBoneKeyframeRefAt(int index) const
{
    return m_context->boneSection->findKeyframeAt(index);
}

ICameraKeyframe *Motion::findCameraKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                            const IKeyframe::LayerIndex &layerIndex) const
{
    return m_context->cameraSection->findKeyframe(timeIndex, layerIndex);
}

ICameraKeyframe *Motion::findCameraKeyframeRefAt(int index) const
{
    return m_context->cameraSection->findKeyframeAt(index);
}

IEffectKeyframe *Motion::findEffectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                            const IString *name,
                                            const IKeyframe::LayerIndex &layerIndex) const
{
    return m_context->effectSection->findKeyframe(timeIndex, name, layerIndex);
}

IEffectKeyframe *Motion::findEffectKeyframeRefAt(int index) const
{
    return m_context->effectSection->findKeyframeAt(index);
}

ILightKeyframe *Motion::findLightKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                          const IKeyframe::LayerIndex &layerIndex) const
{
    return m_context->lightSection->findKeyframe(timeIndex, layerIndex);
}

ILightKeyframe *Motion::findLightKeyframeRefAt(int index) const
{
    return m_context->lightSection->findKeyframeAt(index);
}

IModelKeyframe *Motion::findModelKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                          const IKeyframe::LayerIndex &layerIndex) const
{
    return m_context->modelSection->findKeyframe(timeIndex, layerIndex);
}

IModelKeyframe *Motion::findModelKeyframeRefAt(int index) const
{
    return m_context->modelSection->findKeyframeAt(index);
}

IMorphKeyframe *Motion::findMorphKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                          const IString *name,
                                          const IKeyframe::LayerIndex &layerIndex) const
{
    return m_context->morphSection->findKeyframe(timeIndex, name, layerIndex);
}

IMorphKeyframe *Motion::findMorphKeyframeRefAt(int index) const
{
    return m_context->morphSection->findKeyframeAt(index);
}

IProjectKeyframe *Motion::findProjectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                              const IKeyframe::LayerIndex &layerIndex) const
{
    return m_context->projectSection->findKeyframe(timeIndex, layerIndex);
}

IProjectKeyframe *Motion::findProjectKeyframeRefAt(int index) const
{
    return m_context->projectSection->findKeyframeAt(index);
}

void Motion::deleteKeyframe(IKeyframe *&value)
{
    /* prevent deleting a null keyframe and timeIndex() of the keyframe is zero */
    if (!value || value->timeIndex() == 0) {
        return;
    }
    if (BaseSection *const *sectionPtr = m_context->type2sectionRefs.find(value->type())) {
        BaseSection *section = *sectionPtr;
        section->deleteKeyframe(value);
        value = 0;
    }
}

void Motion::update(IKeyframe::Type /* type */)
{
}

void Motion::getAllKeyframeRefs(Array<IKeyframe *> &value, IKeyframe::Type type)
{
    if (BaseSection *const *sectionPtr = m_context->type2sectionRefs.find(type)) {
        BaseSection *section = *sectionPtr;
        section->getAllKeyframes(value);
    }
}

void Motion::setAllKeyframes(const Array<IKeyframe *> &value, IKeyframe::Type type)
{
    if (BaseSection *const *sectionPtr = m_context->type2sectionRefs.find(type)) {
        BaseSection *section = *sectionPtr;
        section->setAllKeyframes(value);
    }
}

IMotion *Motion::clone() const
{
    IMotion *motion = m_context->motionPtr = new Motion(m_context->parentModelRef, m_context->encodingRef);
    AddAllKeyframes<BoneSection, IBoneKeyframe>(m_context->boneSection, motion);
    AddAllKeyframes<CameraSection, ICameraKeyframe>(m_context->cameraSection, motion);
    AddAllKeyframes<EffectSection, IEffectKeyframe>(m_context->effectSection, motion);
    AddAllKeyframes<LightSection, ILightKeyframe>(m_context->lightSection, motion);
    AddAllKeyframes<ModelSection, IModelKeyframe>(m_context->modelSection, motion);
    AddAllKeyframes<MorphSection, IMorphKeyframe>(m_context->morphSection, motion);
    AddAllKeyframes<ProjectSection, IProjectKeyframe>(m_context->projectSection, motion);
    m_context->motionPtr = 0;
    return motion;
}

ModelKeyframe *Motion::createModelKeyframe() const
{
    return new mvd::ModelKeyframe(m_context->modelSection);
}

const IString *Motion::name() const
{
    return m_context->name;
}

Scene *Motion::parentSceneRef() const
{
    return m_context->parentSceneRef;
}

IModel *Motion::parentModelRef() const
{
    return m_context->parentModelRef;
}

Motion::Error Motion::error() const
{
    return m_context->error;
}

const Motion::DataInfo &Motion::result() const
{
    return m_context->info;
}

NameListSection *Motion::nameListSection() const
{
    return m_context->nameListSection;
}

bool Motion::isActive() const
{
    return m_context->active;
}

IMotion::Type Motion::type() const
{
    return kMVDMotion;
}

} /* namespace mvd */
} /* namespace vpvl2 */


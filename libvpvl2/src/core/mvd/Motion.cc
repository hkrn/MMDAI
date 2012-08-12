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

#include "vpvl2/mvd/AssetSection.h"
#include "vpvl2/mvd/BoneSection.h"
#include "vpvl2/mvd/CameraSection.h"
#include "vpvl2/mvd/EffectSection.h"
#include "vpvl2/mvd/LightSection.h"
#include "vpvl2/mvd/ModelSection.h"
#include "vpvl2/mvd/MorphSection.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/mvd/NameListSection.h"
#include "vpvl2/mvd/ProjectSection.h"

namespace vpvl2
{
namespace mvd
{

const uint8_t *Motion::kSignature = reinterpret_cast<const uint8_t *>("Motion Vector Data file");

#pragma pack(push, 1)

struct Header {
    uint8_t signature[30];
    float version;
    uint8_t encoding;
};

struct SectionHeader {
    uint8_t type;
    uint8_t minor;
};

#pragma pack(pop)

const QuadWord Motion::InterpolationTable::kDefaultParameter = QuadWord(20, 20, 107, 107);

Motion::InterpolationTable::InterpolationTable()
    : parameter(kDefaultParameter),
      linear(true),
      size(0)
{
}

Motion::InterpolationTable::~InterpolationTable()
{
    parameter = kDefaultParameter;
    linear = true;
    size = 0;
}

const QuadWord Motion::InterpolationTable::toQuadWord(const InterpolationPair &pair)
{
    return QuadWord(pair.first.x, pair.first.y, pair.second.x, pair.second.y);
}

void Motion::InterpolationTable::build(const QuadWord &value, int s)
{
    if (!btFuzzyZero(value.x() - value.y()) || !btFuzzyZero(value.z() - value.w())) {
        table.resize(s);
        const IKeyframe::SmoothPrecision &x1 = value.x() / 127.0, &x2 = value.z() / 127.0;
        const IKeyframe::SmoothPrecision &y1 = value.y() / 127.0, &y2 = value.w() / 127.0;
        IKeyframe::SmoothPrecision *ptr = &table[0];
        internal::buildInterpolationTable(x1, x2, y1, y2, s, ptr);
        linear = false;
    }
    else {
        table.clear();
        linear = true;
    }
    parameter = value;
    size = s;
}

void Motion::InterpolationTable::reset()
{
    table.clear();
    linear = true;
    parameter = kDefaultParameter;
}

Motion::Motion(IModel *modelRef, IEncoding *encodingRef)
    : m_assetSection(0),
      m_boneSection(0),
      m_cameraSection(0),
      m_effectSection(0),
      m_lightSection(0),
      m_modelSection(0),
      m_morphSection(0),
      m_nameListSection(0),
      m_projectSection(0),
      m_modelRef(modelRef),
      m_encodingRef(encodingRef),
      m_name(0),
      m_name2(0),
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
    // Header(30)
    Header header;
    if (sizeof(header) > rest) {
        m_error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    info.basePtr = ptr;

    // Check the signature is valid
    header = *reinterpret_cast<const Header *>(ptr);
    if (memcmp(header.signature, kSignature, sizeof(kSignature) - 1) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }
    if (header.version != 1.0) {
        m_error = kInvalidVersionError;
        return false;
    }
    if (header.encoding != 0 && header.encoding != 1) {
        m_error = kInvalidEncodingError;
        return false;
    }
    info.codec = header.encoding == 0 ? IString::kUTF16 : IString::kUTF8;
    ptr += sizeof(header);
    rest -= sizeof(header);

    /* object name */
    if (!internal::sizeText(ptr, rest, info.namePtr, info.nameSize)) {
        return false;
    }
    /* object name2 */
    if (!internal::sizeText(ptr, rest, info.name2Ptr, info.name2Size)) {
        return false;
    }
    /* scene FPS */
    if (!internal::validateSize(ptr, sizeof(float), rest)) {
        return false;
    }
    /* reserved */
    if (!internal::sizeText(ptr, rest, info.reservedPtr, info.reservedSize)) {
        return false;
    }
    info.sectionStartPtr = ptr;

    /* sections */
    bool ret = false;
    while (rest > 0) {
        const SectionHeader &sectionHeader = *reinterpret_cast<const SectionHeader *>(ptr);
        if (!internal::validateSize(ptr, sizeof(sectionHeader), rest)) {
            return false;
        }
        uint8_t *startPtr = ptr;
        switch (static_cast<SectionType>(sectionHeader.type)) {
        case kNameListSection: {
            if (!NameListSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.nameListSectionPtr = startPtr;
            break;
        }
        case kBoneSection: {
            if (!BoneSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.boneSectionPtrs.add(startPtr);
            break;
        }
        case kMorphSection: {
            if (!MorphSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.morphSectionPtrs.add(startPtr);
            break;
        }
        case kModelSection: {
            info.adjustAlignment = sectionHeader.minor == 1 ? 4 : 0;
            if (!ModelSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.modelSectionPtrs.add(startPtr);
            break;
        }
        case kAssetSection: {
            if (!AssetSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.assetSectionPtrs.add(startPtr);
            break;
        }
        case kEffectSection: {
            if (!EffectSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.effectSectionPtrs.add(startPtr);
            break;
        }
        case kCameraSection: {
            if (!CameraSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.cameraSectionPtrs.add(startPtr);
            break;
        }
        case kLightSection: {
            if (!LightSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.lightSectionPtrs.add(startPtr);
            break;
        }
        case kProjectSection: {
            if (!ProjectSection::preparse(ptr, rest, info)) {
                return false;
            }
            info.projectSectionPtrs.add(startPtr);
            break;
        }
        case kEndOfFile: {
            ret = true;
            rest = 0;
            info.encoding = m_encodingRef;
            info.endPtr = ptr;
            break;
        }
        default:
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
        release();
        parseHeader(info);
        parseAssetSections(info);
        parseBoneSections(info);
        parseCameraSections(info);
        parseEffectSections(info);
        parseLightSections(info);
        parseModelSections(info);
        parseMorphSections(info);
        parseProjectSections(info);
        return true;
    }
    return false;
}

void Motion::save(uint8_t *data) const
{
    Header header;
    memcpy(header.signature, kSignature, sizeof(header.signature));
    header.version = 1.0;
    header.encoding = 1;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&header), sizeof(header), data);
}

size_t Motion::estimateSize() const
{
    return sizeof(Header);
}

void Motion::setParentModel(IModel *model)
{
    m_modelRef = model;
}

void Motion::seek(const IKeyframe::TimeIndex &timeIndex)
{
    m_assetSection->seek(timeIndex);
    m_boneSection->seek(timeIndex);
    m_cameraSection->seek(timeIndex);
    m_effectSection->seek(timeIndex);
    m_lightSection->seek(timeIndex);
    m_modelSection->seek(timeIndex);
    m_morphSection->seek(timeIndex);
    m_projectSection->seek(timeIndex);
    m_active = maxTimeIndex() > timeIndex;
}

void Motion::advance(const IKeyframe::TimeIndex &deltaTimeIndex)
{
    m_assetSection->advance(deltaTimeIndex);
    m_boneSection->advance(deltaTimeIndex);
    m_cameraSection->advance(deltaTimeIndex);
    m_effectSection->advance(deltaTimeIndex);
    m_lightSection->advance(deltaTimeIndex);
    m_modelSection->advance(deltaTimeIndex);
    m_morphSection->advance(deltaTimeIndex);
    m_projectSection->advance(deltaTimeIndex);
    if (deltaTimeIndex > 0)
        m_active = isReachedTo(maxTimeIndex());
}

void Motion::reload()
{
}

void Motion::reset()
{
    m_active = true;
}

const IKeyframe::TimeIndex &Motion::maxTimeIndex() const
{
    return btMax(m_assetSection->maxTimeIndex(),
                 btMax(m_boneSection->maxTimeIndex(),
                       btMax(m_cameraSection->maxTimeIndex(),
                             btMax(m_effectSection->maxTimeIndex(),
                                   btMax(m_lightSection->maxTimeIndex(),
                                         btMax(m_modelSection->maxTimeIndex(),
                                               btMax(m_morphSection->maxTimeIndex(),
                                                     m_projectSection->maxTimeIndex())))))));
}

bool Motion::isReachedTo(const IKeyframe::TimeIndex &atEnd) const
{
    // force inactive motion is reached
    return !m_active  || (m_assetSection->currentTimeIndex() >= atEnd &&
                          m_boneSection->currentTimeIndex() >= atEnd &&
                          m_cameraSection->currentTimeIndex() >= atEnd &&
                          m_effectSection->currentTimeIndex() >= atEnd &&
                          m_lightSection->currentTimeIndex() >= atEnd &&
                          m_modelSection->currentTimeIndex() >= atEnd &&
                          m_morphSection->currentTimeIndex() >= atEnd &&
                          m_projectSection->currentTimeIndex() >= atEnd);
}

bool Motion::isNullFrameEnabled() const
{
    return false;
}

void Motion::setNullFrameEnable(bool value)
{
}

void Motion::addKeyframe(IKeyframe *value)
{
    if (!value)
        return;
    switch (value->type()) {
    case IKeyframe::kAsset:
        break;
    case IKeyframe::kBone:
        break;
    case IKeyframe::kCamera:
        break;
    case IKeyframe::kEffect:
        break;
    case IKeyframe::kLight:
        break;
    case IKeyframe::kModel:
        break;
    case IKeyframe::kMorph:
        break;
    case IKeyframe::kProject:
        break;
    default:
        break;
    }
}

void Motion::replaceKeyframe(IKeyframe *value)
{
    if (!value)
        return;
    switch (value->type()) {
    case IKeyframe::kAsset: {
        break;
    }
    case IKeyframe::kBone: {
        break;
    }
    case IKeyframe::kCamera: {
        break;
    }
    case IKeyframe::kEffect: {
        break;
    }
    case IKeyframe::kLight: {
        break;
    }
    case IKeyframe::kModel: {
        break;
    }
    case IKeyframe::kMorph: {
        break;
    }
    case IKeyframe::kProject: {
        break;
    }
    default:
        break;
    }
}

int Motion::countKeyframes(IKeyframe::Type value) const
{
    switch (value) {
    case IKeyframe::kAsset:
        return m_assetSection->countKeyframes();
    case IKeyframe::kBone:
        return m_boneSection->countKeyframes();
    case IKeyframe::kCamera:
        return m_cameraSection->countKeyframes();
    case IKeyframe::kEffect:
        return m_effectSection->countKeyframes();
    case IKeyframe::kLight:
        return m_lightSection->countKeyframes();
    case IKeyframe::kModel:
        return m_modelSection->countKeyframes();
    case IKeyframe::kMorph:
        return m_morphSection->countKeyframes();
    case IKeyframe::kProject:
        return m_projectSection->countKeyframes();
    default:
        return 0;
    }
}

IKeyframe::LayerIndex Motion::countLayers(const IKeyframe::TimeIndex &timeIndex,
                                          const IString *name,
                                          IKeyframe::Type type) const
{
    switch (type) {
    case IKeyframe::kBone:
        return 1;
    case IKeyframe::kCamera:
        return 1;
    default:
        return 1;
    }
}

IBoneKeyframe *Motion::findBoneKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                        const IString *name,
                                        const IKeyframe::LayerIndex &layerIndex) const
{
    return m_boneSection->findKeyframe(timeIndex, name, layerIndex);
}

IBoneKeyframe *Motion::findBoneKeyframeAt(int index) const
{
    return m_boneSection->findKeyframeAt(index);
}

ICameraKeyframe *Motion::findCameraKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                            const IKeyframe::LayerIndex &layerIndex) const
{
    return m_cameraSection->findKeyframe(timeIndex, layerIndex);
}

ICameraKeyframe *Motion::findCameraKeyframeAt(int index) const
{
    return m_cameraSection->findKeyframeAt(index);
}

ILightKeyframe *Motion::findLightKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                          const IKeyframe::LayerIndex &layerIndex) const
{
    return m_lightSection->findKeyframe(timeIndex, layerIndex);
}

ILightKeyframe *Motion::findLightKeyframeAt(int index) const
{
    return m_lightSection->findKeyframeAt(index);
}

IMorphKeyframe *Motion::findMorphKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                          const IString *name,
                                          const IKeyframe::LayerIndex &layerIndex) const
{
    return m_morphSection->findKeyframe(timeIndex, name, layerIndex);
}

IMorphKeyframe *Motion::findMorphKeyframeAt(int index) const
{
    return m_morphSection->findKeyframeAt(index);
}

void Motion::deleteKeyframe(IKeyframe *&value)
{
    if (!value)
        return;
    switch (value->type()) {
    case IKeyframe::kAsset:
        value = 0;
        break;
    case IKeyframe::kBone:
        value = 0;
        break;
    case IKeyframe::kCamera:
        value = 0;
        break;
    case IKeyframe::kEffect:
        value = 0;
        break;
    case IKeyframe::kLight:
        value = 0;
        break;
    case IKeyframe::kModel:
        value = 0;
        break;
    case IKeyframe::kMorph:
        value = 0;
        break;
    case IKeyframe::kProject:
        value = 0;
        break;
    default:
        break;
    }
}

void Motion::deleteKeyframes(const IKeyframe::TimeIndex &timeIndex, IKeyframe::Type type)
{
    switch (type) {
    case IKeyframe::kAsset:
        break;
    case IKeyframe::kBone:
        break;
    case IKeyframe::kCamera:
        break;
    case IKeyframe::kEffect:
        break;
    case IKeyframe::kLight:
        break;
    case IKeyframe::kModel:
        break;
    case IKeyframe::kMorph:
        break;
    case IKeyframe::kProject:
        break;
    default:
        break;
    }
}

void Motion::update(IKeyframe::Type type)
{
    switch (type) {
    case IKeyframe::kAsset:
        break;
    case IKeyframe::kBone:
        break;
    case IKeyframe::kCamera:
        break;
    case IKeyframe::kEffect:
        break;
    case IKeyframe::kLight:
        break;
    case IKeyframe::kModel:
        break;
    case IKeyframe::kMorph:
        break;
    case IKeyframe::kProject:
        break;
    default:
        break;
    }
}

void Motion::parseHeader(const DataInfo &info)
{
    IEncoding *encoding = info.encoding;
    internal::setStringDirect(encoding->toString(info.namePtr, info.nameSize, info.codec), m_name);
    internal::setStringDirect(encoding->toString(info.name2Ptr, info.name2Size, info.codec), m_name2);
    m_nameListSection = new NameListSection(m_encodingRef);
    m_nameListSection->read(info.nameListSectionPtr, info.codec);
}

void Motion::parseAssetSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.assetSectionPtrs;
    const int nsections = sections.count();
    m_assetSection = new AssetSection(m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_assetSection->read(ptr);
    }
}

void Motion::parseBoneSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.boneSectionPtrs;
    const int nsections = sections.count();
    m_boneSection = new BoneSection(m_modelRef, m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_boneSection->read(ptr);
    }
}

void Motion::parseCameraSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.cameraSectionPtrs;
    const int nsections = sections.count();
    m_cameraSection = new CameraSection(m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_cameraSection->read(ptr);
    }
}

void Motion::parseEffectSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.effectSectionPtrs;
    const int nsections = sections.count();
    m_effectSection = new EffectSection(m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_effectSection->read(ptr);
    }
}

void Motion::parseLightSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.lightSectionPtrs;
    const int nsections = sections.count();
    m_lightSection = new LightSection(m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_lightSection->read(ptr);
    }
}

void Motion::parseModelSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.modelSectionPtrs;
    const int nsections = sections.count();
    m_modelSection = new ModelSection(m_modelRef, m_nameListSection, info.adjustAlignment);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_modelSection->read(ptr);
    }
}

void Motion::parseMorphSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.morphSectionPtrs;
    const int nsections = sections.count();
    m_morphSection = new MorphSection(m_modelRef, m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_morphSection->read(ptr);
    }
}

void Motion::parseProjectSections(const DataInfo &info)
{
    const Array<uint8_t *> &sections = info.projectSectionPtrs;
    const int nsections = sections.count();
    m_projectSection = new ProjectSection(m_nameListSection);
    for (int i = 0; i < nsections; i++) {
        const uint8_t *ptr = sections[i];
        m_projectSection->read(ptr);
    }
}

void Motion::release()
{
    delete m_assetSection;
    m_assetSection = 0;
    delete m_boneSection;
    m_boneSection = 0;
    delete m_cameraSection;
    m_cameraSection = 0;
    delete m_effectSection;
    m_effectSection = 0;
    delete m_lightSection;
    m_lightSection = 0;
    delete m_modelSection;
    m_modelSection = 0;
    delete m_morphSection;
    m_morphSection = 0;
    delete m_nameListSection;
    m_nameListSection = 0;
    delete m_projectSection;
    m_projectSection = 0;
    delete m_name;
    m_name = 0;
    delete m_name2;
    m_name2 = 0;
    m_error = kNoError;
    m_active = false;
}

} /* namespace mvd */
} /* namespace vpvl2 */


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

#include "vpvl2/vmd/BoneKeyframe.h"
#include "vpvl2/vmd/CameraKeyframe.h"
#include "vpvl2/vmd/LightKeyframe.h"
#include "vpvl2/vmd/MorphKeyframe.h"
#include "vpvl2/mvd/Motion.h"

namespace vpvl2
{
namespace mvd
{

const uint8_t *Motion::kSignature = reinterpret_cast<const uint8_t *>("Motion Vector Data file");

enum SectionType {
    kNameListSection = 0,
    kBoneSection     = 16,
    kMorphSection    = 32,
    kModelSection    = 64,
    kAssetSection    = 80,
    kEffectSection   = 88,
    kCameraSection   = 96,
    kLightSection    = 112,
    kProjectSection  = 128,
    kEndOfFile       = 255
};

enum EffectParameterType {
    kNoneEffectParameter,
    kBoolEffectParameter,
    kIntegerEffectParameter,
    kStringEffectParameter,
    kFloatEffectParameter,
    kFloat2EffectParameter,
    kFloat3EffectParameter,
    kFloat4EffectParameter
};

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

struct NameSectionHeader {
    int reserved;
    int reserved2;
    int count;
    int reserved3;
};

struct BoneSectionHeader {
    int key;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int sizeOfLayer;
};

struct Interpolation {
    uint8_t x;
    uint8_t y;
};

struct InterpolationPair {
    Interpolation first;
    Interpolation second;
};

struct BoneKeyframe {
    int layerIndex;
    uint64_t timeIndex;
    float position[3];
    float rotation[4];
    InterpolationPair x;
    InterpolationPair y;
    InterpolationPair z;
    InterpolationPair r;
};

struct MorphSecionHeader {
    int key;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int reserved;
};

struct MorphKeyframe {
    uint64_t timeIndex;
    float weight;
    InterpolationPair weightIP;
};

struct ModelSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int sizeOfIK;
    int countOfIK;
};

struct ModelKeyframe {
    uint64_t timeIndex;
    uint8_t visible;
    uint8_t shadow;
    uint8_t addBlend;
    uint8_t physics;
    uint8_t physicsStillMode;
    uint8_t reserved[3];
    float edgeWidth;
    uint8_t edgeColor[4];
};

struct AssetSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int reserved2;
};

struct AssetKeyframe {
    uint64_t timeIndex;
    uint8_t visible;
    uint8_t shadow;
    uint8_t addBlend;
    uint8_t reserved;
    float scaleFactor;
    float opacity;
    int modelID;
    int boneID;
};

struct EffectSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int parameterSize;
    int parameterCount;
};

struct EffectParameter {
    int pid;
    int type;
};

struct EffectKeyframe {
    uint64_t timeIndex;
    uint8_t visible;
    uint8_t addBlend;
    uint8_t shadow;
    uint8_t reserved;
    float scaleFactor;
    float opacity;
    int modelID;
    int boneID;
};

struct CameraSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int layerSize;
};

struct CameraKeyframe {
    int layerIndex;
    uint64_t timeIndex;
    float radius;
    float position[3];
    float rotation[3];
    float fov;
    uint8_t perspective;
    InterpolationPair positionIP;
    InterpolationPair rotationIP;
    InterpolationPair distanceIP;
    InterpolationPair fovIP;
};

struct LightSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int reserved2;
};

struct LightKeyframe {
    uint64_t timeIndex;
    float position[3];
    float direction[3];
    uint8_t enabled;
};

struct ProjectSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int reserved2;
};

struct ProjectKeyframe {
    uint64_t timeIndex;
    float gravityFactor;
    float gravityDirection[3];
    int shadowMode;
    float shadowDistance;
    float shadowDepth;
};

#pragma pack(pop)

Motion::Motion(IModel *model, IEncoding *encoding)
    : m_model(model),
      m_encoding(encoding),
      m_name(0),
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
    ptr += sizeof(header);
    rest -= sizeof(header);

    uint8_t *namePtr;
    size_t nNameSize;
    /* object name */
    if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
        return false;
    }
    /* object name2 */
    if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
        return false;
    }
    /* scene FPS */
    if (!internal::validateSize(ptr, sizeof(float), rest)) {
        return false;
    }
    /* reserved */
    if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
        return false;
    }
    /* sections */
    bool ret = false;
    while (rest > 0) {
        const SectionHeader &sectionHeader = *reinterpret_cast<const SectionHeader *>(ptr);
        if (!internal::validateSize(ptr, sizeof(sectionHeader), rest)) {
            return false;
        }
        switch (static_cast<SectionType>(sectionHeader.type)) {
        case kNameListSection: {
            const NameSectionHeader &nameSectionHeader = *reinterpret_cast<const NameSectionHeader *>(ptr);
            if (!internal::validateSize(ptr, sizeof(nameSectionHeader), rest)) {
                return false;
            }
            if (!internal::validateSize(ptr, nameSectionHeader.reserved3, rest)) {
                return false;
            }
            static int keyIndex;
            const int nkeyframes = nameSectionHeader.count;
            for (int i = 0; i < nkeyframes; i++) {
                if (!internal::validateSize(ptr, sizeof(keyIndex), rest)) {
                    return false;
                }
                if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
                    return false;
                }
            }
            break;
        }
        case kBoneSection: {
            const BoneSectionHeader &boneSectionHeader = *reinterpret_cast<const BoneSectionHeader *>(ptr);
            if (!internal::validateSize(ptr, sizeof(boneSectionHeader), rest)) {
                return false;
            }
            if (!internal::validateSize(ptr, boneSectionHeader.sizeOfLayer, rest)) {
                return false;
            }
            static BoneKeyframe keyframe;
            const int nkeyframes = boneSectionHeader.countOfKeyframes;
            const size_t reserved = boneSectionHeader.sizeOfKeyframe - sizeof(keyframe);
            for (int i = 0; i < nkeyframes; i++) {
                if (!internal::validateSize(ptr, sizeof(keyframe), rest)) {
                    return false;
                }
                if (!internal::validateSize(ptr, reserved, rest)) {
                    return false;
                }
            }
            break;
        }
        case kMorphSection: {
            const MorphSecionHeader &morphSectionHeader = *reinterpret_cast<const MorphSecionHeader *>(ptr);
            if (!internal::validateSize(ptr, sizeof(morphSectionHeader), rest)) {
                return false;
            }
            if (!internal::validateSize(ptr, morphSectionHeader.reserved, rest)) {
                return false;
            }
            static MorphKeyframe keyframe;
            const int nkeyframes = morphSectionHeader.countOfKeyframes;
            const size_t reserved = morphSectionHeader.sizeOfKeyframe - sizeof(keyframe);
            for (int i = 0; i < nkeyframes; i++) {
                if (!internal::validateSize(ptr, sizeof(keyframe), rest)) {
                    return false;
                }
                if (!internal::validateSize(ptr, reserved, rest)) {
                    return false;
                }
            }
            break;
        }
        case kModelSection: {
            const ModelSectionHeader &modelSectionHeader = *reinterpret_cast<const ModelSectionHeader *>(ptr);
            if (!internal::validateSize(ptr, sizeof(modelSectionHeader), rest)) {
                return false;
            }
            const int countOfIK = modelSectionHeader.countOfIK;
            if (!internal::validateSize(ptr, countOfIK * sizeof(int), rest)) {
                return false;
            }
            const int sizeOfIK = modelSectionHeader.sizeOfIK;
            if (!internal::validateSize(ptr, sizeOfIK - 4 * (countOfIK + 1), rest)) {
                return false;
            }
            static ModelKeyframe keyframe;
            const int nkeyframes = modelSectionHeader.countOfKeyframes;
            const size_t adjust = sectionHeader.minor == 1 ? 4 : 0;
            const size_t reserved = modelSectionHeader.sizeOfKeyframe - ((sizeof(keyframe) - adjust) + countOfIK);
            for (int i = 0; i < nkeyframes; i++) {
                if (!internal::validateSize(ptr, sizeof(keyframe), rest)) {
                    return false;
                }
                if (!internal::validateSize(ptr, sizeof(uint8_t), countOfIK, rest)) {
                    return false;
                }
                if (!internal::validateSize(ptr, reserved, rest)) {
                    return false;
                }
            }
            break;
        }
        case kAssetSection: {
            const AssetSectionHeader &assetSectionHeader = *reinterpret_cast<const AssetSectionHeader *>(ptr);
            if (!internal::validateSize(ptr, sizeof(assetSectionHeader), rest)) {
                return false;
            }
            if (!internal::validateSize(ptr, assetSectionHeader.reserved2, rest)) {
                return false;
            }
            static AssetKeyframe keyframe;
            const int nkeyframes = assetSectionHeader.countOfKeyframes;
            const size_t reserved = assetSectionHeader.sizeOfKeyframe - sizeof(keyframe);
            for (int i = 0; i < nkeyframes; i++) {
                if (!internal::validateSize(ptr, sizeof(keyframe), rest)) {
                    return false;
                }
                if (!internal::validateSize(ptr, reserved, rest)) {
                    return false;
                }
            }
            break;
        }
        case kEffectSection: {
            const EffectSectionHeader &effectSectionHeader = *reinterpret_cast<const EffectSectionHeader *>(ptr);
            if (!internal::validateSize(ptr, sizeof(effectSectionHeader), rest)) {
                return false;
            }
            const int nparameters = effectSectionHeader.parameterCount;
            const size_t parameterArraySize = nparameters * sizeof(int);
            if (!internal::validateSize(ptr, parameterArraySize, rest)) {
                return false;
            }
            if (!internal::validateSize(ptr, effectSectionHeader.parameterSize - 8 * nparameters - 4, rest)) {
                return false;
            }
            static EffectKeyframe keyframe;
            const int nkeyframes = effectSectionHeader.countOfKeyframes;
            const size_t reserved = effectSectionHeader.sizeOfKeyframe - (sizeof(keyframe) + parameterArraySize);
            for (int i = 0; i < nkeyframes; i++) {
                if (!internal::validateSize(ptr, sizeof(keyframe), rest)) {
                    return false;
                }
                if (!internal::validateSize(ptr, reserved, rest)) {
                    return false;
                }
            }
            break;
        }
        case kCameraSection: {
            const CameraSectionHeader &cameraSectionHeader = *reinterpret_cast<const CameraSectionHeader *>(ptr);
            if (!internal::validateSize(ptr, sizeof(cameraSectionHeader), rest)) {
                return false;
            }
            if (!internal::validateSize(ptr, cameraSectionHeader.layerSize, rest)) {
                return false;
            }
            static CameraKeyframe keyframe;
            const int nkeyframes = cameraSectionHeader.countOfKeyframes;
            const size_t reserved = cameraSectionHeader.sizeOfKeyframe - sizeof(keyframe);
            for (int i = 0; i < nkeyframes; i++) {
                if (!internal::validateSize(ptr, sizeof(keyframe), rest)) {
                    return false;
                }
                if (!internal::validateSize(ptr, reserved, rest)) {
                    return false;
                }
            }
            break;
        }
        case kLightSection: {
            const LightSectionHeader lightSectionHeader = *reinterpret_cast<const LightSectionHeader *>(ptr);
            if (!internal::validateSize(ptr, sizeof(lightSectionHeader), rest)) {
                return false;
            }
            if (!internal::validateSize(ptr, lightSectionHeader.reserved2, rest)) {
                return false;
            }
            static LightKeyframe keyframe;
            const int nkeyframes = lightSectionHeader.countOfKeyframes;
            const size_t reserved = lightSectionHeader.sizeOfKeyframe - sizeof(keyframe);
            for (int i = 0; i < nkeyframes; i++) {
                if (!internal::validateSize(ptr, sizeof(keyframe), rest)) {
                    return false;
                }
                if (!internal::validateSize(ptr, reserved, rest)) {
                    return false;
                }
            }
            break;
        }
        case kProjectSection: {
            const ProjectSectionHeader &projectSectionHeader = *reinterpret_cast<const ProjectSectionHeader *>(ptr);
            if (!internal::validateSize(ptr, sizeof(projectSectionHeader), rest)) {
                return false;
            }
            if (!internal::validateSize(ptr, projectSectionHeader.reserved2, rest)) {
                return false;
            }
            static EffectKeyframe keyframe;
            const int nkeyframes = projectSectionHeader.countOfKeyframes;
            const size_t reserved = projectSectionHeader.sizeOfKeyframe - sizeof(keyframe);
            for (int i = 0; i < nkeyframes; i++) {
                if (!internal::validateSize(ptr, sizeof(keyframe), rest)) {
                    return false;
                }
                if (!internal::validateSize(ptr, reserved, rest)) {
                    return false;
                }
            }
            break;
        }
        case kEndOfFile: {
            ret = true;
            rest = 0;
            break;
        }
        default:
            rest = 0;
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
    m_model = model;
}

void Motion::seek(const IKeyframe::TimeIndex &timeIndex)
{
    m_active = maxTimeIndex() > timeIndex;
}

void Motion::advance(const IKeyframe::TimeIndex &delta)
{
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
    return 0;
}

bool Motion::isReachedTo(const IKeyframe::TimeIndex &atEnd) const
{
    // force inactive motion is reached
    return !m_active;
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
    case IKeyframe::kBone:
        break;
    case IKeyframe::kCamera:
        break;
    case IKeyframe::kLight:
        break;
    case IKeyframe::kMorph:
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
    case IKeyframe::kBone: {
        break;
    }
    case IKeyframe::kCamera: {
        break;
    }
    case IKeyframe::kLight: {
        break;
    }
    case IKeyframe::kMorph: {
        break;
    }
    default:
        break;
    }
}

int Motion::countKeyframes(IKeyframe::Type value) const
{
    switch (value) {
    case IKeyframe::kBone:
        return 0;
    case IKeyframe::kCamera:
        return 0;
    case IKeyframe::kLight:
        return 0;
    case IKeyframe::kMorph:
        return 0;
    default:
        return 0;
    }
}

IBoneKeyframe *Motion::findBoneKeyframe(const IKeyframe::TimeIndex &timeIndex, const IString *name) const
{
    return 0;
}

IBoneKeyframe *Motion::findBoneKeyframeAt(int index) const
{
    return 0;
}

ICameraKeyframe *Motion::findCameraKeyframe(const IKeyframe::TimeIndex &timeIndex) const
{
    return 0;
}

ICameraKeyframe *Motion::findCameraKeyframeAt(int index) const
{
    return 0;
}

ILightKeyframe *Motion::findLightKeyframe(const IKeyframe::TimeIndex &timeIndex) const
{
    return 0;
}

ILightKeyframe *Motion::findLightKeyframeAt(int index) const
{
    return 0;
}

IMorphKeyframe *Motion::findMorphKeyframe(const IKeyframe::TimeIndex &timeIndex, const IString *name) const
{
    return 0;
}

IMorphKeyframe *Motion::findMorphKeyframeAt(int index) const
{
    return 0;
}

void Motion::deleteKeyframe(IKeyframe *&value)
{
    if (!value)
        return;
    switch (value->type()) {
    case IKeyframe::kBone:
        value = 0;
        break;
    case IKeyframe::kCamera:
        value = 0;
        break;
    case IKeyframe::kLight:
        value = 0;
        break;
    case IKeyframe::kMorph:
        value = 0;
        break;
    default:
        break;
    }
}

void Motion::deleteKeyframes(const IKeyframe::TimeIndex &timeIndex, IKeyframe::Type type)
{
    switch (type) {
    case IKeyframe::kBone:
        break;
    case IKeyframe::kCamera:
        break;
    case IKeyframe::kLight:
        break;
    case IKeyframe::kMorph:
        break;
    default:
        break;
    }
}

void Motion::update(IKeyframe::Type type)
{
    switch (type) {
    case IKeyframe::kBone:
        break;
    case IKeyframe::kCamera:
        break;
    case IKeyframe::kLight:
        break;
    case IKeyframe::kMorph:
        break;
    default:
        break;
    }
}

void Motion::parseHeader(const DataInfo &info)
{
}

void Motion::parseBoneFrames(const DataInfo &info)
{
}

void Motion::parseMorphFrames(const DataInfo &info)
{
}

void Motion::parseCameraFrames(const DataInfo &info)
{
}

void Motion::parseLightFrames(const DataInfo &info)
{
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


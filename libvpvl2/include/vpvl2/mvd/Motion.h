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

#ifndef VPVL2_MVD_MOTION_H_
#define VPVL2_MVD_MOTION_H_

#include "vpvl2/IEncoding.h"
#include "vpvl2/IMotion.h"

namespace vpvl2
{
namespace mvd
{

struct Interpolation {
    uint8_t x;
    uint8_t y;
};

struct InterpolationPair {
    Interpolation first;
    Interpolation second;
};

class AssetSection;
class BoneSection;
class CameraSection;
class EffectSection;
class LightSection;
class ModelSection;
class MorphSection;
class NameListSection;
class ProjectSection;

class VPVL2_API Motion : public IMotion
{
public:

    /**
      * Type of parsing errors.
      */
    enum Error
    {
        kNoError,
        kInvalidHeaderError,
        kInvalidSignatureError,
        kInvalidVersionError,
        kInvalidEncodingError,
        kMaxErrors
    };

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
    struct DataInfo
    {
        IEncoding *encoding;
        IString::Codec codec;
        uint8_t *basePtr;
        uint8_t *namePtr;
        size_t nameSize;
        uint8_t *name2Ptr;
        size_t name2Size;
        uint8_t *reservedPtr;
        size_t reservedSize;
        uint8_t *sectionStartPtr;
        uint8_t *nameListSectionPtr;
        Array<uint8_t *> assetSectionPtrs;
        Array<uint8_t *> boneSectionPtrs;
        Array<uint8_t *> cameraSectionPtrs;
        Array<uint8_t *> effectSectionPtrs;
        Array<uint8_t *> lightSectionPtrs;
        Array<uint8_t *> modelSectionPtrs;
        Array<uint8_t *> morphSectionPtrs;
        Array<uint8_t *> projectSectionPtrs;
        uint8_t *endPtr;
    };
    static const uint8_t *kSignature;

    Motion(IModel *modelRef, IEncoding *encodingRef);
    ~Motion();

    bool preparse(const uint8_t *data, size_t size, DataInfo &info);
    bool load(const uint8_t *data, size_t size);
    void save(uint8_t *data) const;
    size_t estimateSize() const;
    void setParentModel(IModel *model);
    void seek(const IKeyframe::TimeIndex &timeIndex);
    void advance(const IKeyframe::TimeIndex &delta);
    void reload();
    void reset();
    const IKeyframe::TimeIndex &maxTimeIndex() const;
    bool isReachedTo(const IKeyframe::TimeIndex &atEnd) const;
    bool isNullFrameEnabled() const;
    void setNullFrameEnable(bool value);

    void addKeyframe(IKeyframe *value);
    int countKeyframes(IKeyframe::Type value) const;
    IBoneKeyframe *findBoneKeyframe(const IKeyframe::TimeIndex &timeIndex, const IString *name) const;
    IBoneKeyframe *findBoneKeyframeAt(int index) const;
    ICameraKeyframe *findCameraKeyframe(const IKeyframe::TimeIndex &timeIndex) const;
    ICameraKeyframe *findCameraKeyframeAt(int index) const;
    ILightKeyframe *findLightKeyframe(const IKeyframe::TimeIndex &timeIndex) const;
    ILightKeyframe *findLightKeyframeAt(int index) const;
    IMorphKeyframe *findMorphKeyframe(const IKeyframe::TimeIndex &timeIndex, const IString *name) const;
    IMorphKeyframe *findMorphKeyframeAt(int index) const;
    void replaceKeyframe(IKeyframe *value);
    void deleteKeyframe(IKeyframe *&value);
    void deleteKeyframes(const IKeyframe::TimeIndex &timeIndex, IKeyframe::Type type);
    void update(IKeyframe::Type type);

    const IString *name() const {
        return m_name;
    }
    IModel *parentModel() const {
        return m_modelRef;
    }
    Error error() const {
        return m_error;
    }
    const DataInfo &result() const {
        return m_result;
    }
    bool isActive() const {
        return m_active;
    }

private:
    void parseHeader(const DataInfo &info);
    void parseAssetSections(const DataInfo &info);
    void parseBoneSections(const DataInfo &info);
    void parseCameraSections(const DataInfo &info);
    void parseEffectSections(const DataInfo &info);
    void parseLightSections(const DataInfo &info);
    void parseModelSections(const DataInfo &info);
    void parseMorphSections(const DataInfo &info);
    void parseProjectSections(const DataInfo &info);
    void release();

    AssetSection *m_assetSection;
    BoneSection *m_boneSection;
    CameraSection *m_cameraSection;
    EffectSection *m_effectSection;
    LightSection *m_lightSection;
    ModelSection *m_modelSection;
    MorphSection *m_morphSection;
    NameListSection *m_nameListSection;
    ProjectSection *m_projectSection;
    IModel *m_modelRef;
    IEncoding *m_encodingRef;
    IString *m_name;
    IString *m_name2;
    DataInfo m_result;
    Error m_error;
    bool m_active;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Motion)
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif


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

#pragma once
#ifndef VPVL2_MVD_MOTION_H_
#define VPVL2_MVD_MOTION_H_

#include "vpvl2/IEncoding.h"
#include "vpvl2/IMotion.h"

namespace vpvl2
{
namespace mvd
{

class AssetSection;
class BaseSection;
class BoneSection;
class CameraSection;
class EffectSection;
class LightSection;
class ModelKeyframe;
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
    struct DataInfo {
        DataInfo()
            : encoding(0),
              codec(IString::kUTF8),
              basePtr(0),
              namePtr(0),
              nameSize(0),
              name2Ptr(0),
              name2Size(0),
              reservedPtr(0),
              reservedSize(0),
              adjustAlignment(0),
              sectionStartPtr(0),
              nameListSectionPtr(0),
              endPtr(0)
        {
        }
        void copy(const DataInfo &other) {
            encoding = other.encoding;
            codec = other.codec;
            namePtr = other.namePtr;
            nameSize = other.nameSize;
            name2Ptr = other.name2Ptr;
            name2Size = other.name2Size;
            reservedPtr = other.reservedPtr;
            reservedSize = other.reservedSize;
            adjustAlignment = other.adjustAlignment;
            sectionStartPtr = other.sectionStartPtr;
            nameListSectionPtr = other.nameListSectionPtr;
            assetSectionPtrs.copy(other.assetSectionPtrs);
            boneSectionPtrs.copy(other.boneSectionPtrs);
            cameraSectionPtrs.copy(other.cameraSectionPtrs);
            effectSectionPtrs.copy(other.effectSectionPtrs);
            lightSectionPtrs.copy(other.lightSectionPtrs);
            modelSectionPtrs.copy(other.modelSectionPtrs);
            morphSectionPtrs.copy(other.morphSectionPtrs);
            projectSectionPtrs.copy(other.projectSectionPtrs);
            endPtr = other.endPtr;
        }
        IEncoding *encoding;
        IString::Codec codec;
        uint8 *basePtr;
        uint8 *namePtr;
        int32 nameSize;
        uint8 *name2Ptr;
        int32 name2Size;
        uint8 *fpsPtr;
        float32 fps;
        uint8 *reservedPtr;
        int32 reservedSize;
        vsize adjustAlignment;
        uint8 *sectionStartPtr;
        uint8 *nameListSectionPtr;
        Array<uint8 *> assetSectionPtrs;
        Array<uint8 *> boneSectionPtrs;
        Array<uint8 *> cameraSectionPtrs;
        Array<uint8 *> effectSectionPtrs;
        Array<uint8 *> lightSectionPtrs;
        Array<uint8 *> modelSectionPtrs;
        Array<uint8 *> morphSectionPtrs;
        Array<uint8 *> projectSectionPtrs;
        uint8 *endPtr;
    };
    static const uint8 *kSignature;

#pragma pack(push, 1)

    struct SectionTag {
        uint8 type;
        uint8 minor;
    };

#pragma pack(pop)

    Motion(IModel *modelRef, IEncoding *encodingRef);
    ~Motion();

    bool preparse(const uint8 *data, vsize size, DataInfo &info);
    bool load(const uint8 *data, vsize size);
    void save(uint8 *data) const;
    vsize estimateSize() const;
    void setParentSceneRef(Scene *value);
    void setParentModelRef(IModel *value);
    void seek(const IKeyframe::TimeIndex &timeIndex);
    void seekScene(const IKeyframe::TimeIndex &timeIndex, Scene *scene);
    void advance(const IKeyframe::TimeIndex &deltaTimeIndex);
    void advanceScene(const IKeyframe::TimeIndex &deltaTimeIndex, Scene *scene);
    void reload();
    void reset();
    IKeyframe::TimeIndex duration() const;
    bool isReachedTo(const IKeyframe::TimeIndex &atEnd) const;
    bool isNullFrameEnabled() const;
    void setNullFrameEnable(bool value);

    void addKeyframe(IKeyframe *value);
    int countKeyframes(IKeyframe::Type value) const;
    void getKeyframeRefs(const IKeyframe::TimeIndex &timeIndex,
                      const IKeyframe::LayerIndex &layerIndex,
                      IKeyframe::Type type,
                      Array<IKeyframe *> &keyframes);
    IKeyframe::LayerIndex countLayers(const vpvl2::IString *name,
                                      IKeyframe::Type type) const;
    IBoneKeyframe *findBoneKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                    const IString *name,
                                    const IKeyframe::LayerIndex &layerIndex) const;
    IBoneKeyframe *findBoneKeyframeRefAt(int index) const;
    ICameraKeyframe *findCameraKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                        const IKeyframe::LayerIndex &layerIndex) const;
    ICameraKeyframe *findCameraKeyframeRefAt(int index) const;
    IEffectKeyframe *findEffectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                        const IString *name,
                                        const IKeyframe::LayerIndex &layerIndex) const;
    IEffectKeyframe *findEffectKeyframeRefAt(int index) const;
    ILightKeyframe *findLightKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                      const IKeyframe::LayerIndex &layerIndex) const;
    ILightKeyframe *findLightKeyframeRefAt(int index) const;
    IModelKeyframe *findModelKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                      const IKeyframe::LayerIndex &layerIndex) const;
    IModelKeyframe *findModelKeyframeRefAt(int index) const;
    IMorphKeyframe *findMorphKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                      const IString *name,
                                      const IKeyframe::LayerIndex &layerIndex) const;
    IMorphKeyframe *findMorphKeyframeRefAt(int index) const;
    IProjectKeyframe *findProjectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                          const IKeyframe::LayerIndex &layerIndex) const;
    IProjectKeyframe *findProjectKeyframeRefAt(int index) const;
    void replaceKeyframe(IKeyframe *value);
    void deleteKeyframe(IKeyframe *&value);
    void deleteKeyframes(const IKeyframe::TimeIndex &timeIndex, IKeyframe::Type type);
    void update(IKeyframe::Type type);
    void getAllKeyframeRefs(Array<IKeyframe *> &value, IKeyframe::Type type);
    void setAllKeyframes(const Array<IKeyframe *> &value, IKeyframe::Type type);
    IMotion *clone() const;

    ModelKeyframe *createModelKeyframe() const;

    const IString *name() const;
    Scene *parentSceneRef() const;
    IModel *parentModelRef() const;
    Error error() const;
    const DataInfo &result() const;
    NameListSection *nameListSection() const;
    bool isActive() const;
    Type type() const;

private:
    struct PrivateContext;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Motion)
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif


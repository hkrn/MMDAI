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

#ifndef VPVL2_VMD_VMDMOTION_H_
#define VPVL2_VMD_VMDMOTION_H_

#include "vpvl2/IEncoding.h"
#include "vpvl2/IMotion.h"
#include "vpvl2/vmd/BoneAnimation.h"
#include "vpvl2/vmd/CameraAnimation.h"
#include "vpvl2/vmd/LightAnimation.h"
#include "vpvl2/vmd/MorphAnimation.h"

namespace vpvl2
{
namespace vmd
{

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Bone class represents a Vocaloid Motion Data object, set of bone, face and camera motion.
 */

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
        kBoneKeyFramesSizeError,
        kBoneKeyFramesError,
        kMorphKeyFramesSizeError,
        kMorphKeyFramesError,
        kCameraKeyFramesSizeError,
        kCameraKeyFramesError,
        kLightKeyFramesSizeError,
        kLightKeyFramesError,
        kMaxErrors
    };

    struct DataInfo
    {
        const uint8_t *basePtr;
        const uint8_t *namePtr;
        const uint8_t *boneKeyframePtr;
        size_t boneKeyframeCount;
        const uint8_t *morphKeyframePtr;
        size_t morphKeyframeCount;
        const uint8_t *cameraKeyframePtr;
        size_t cameraKeyframeCount;
        const uint8_t *lightKeyframePtr;
        size_t lightKeyframeCount;
        const uint8_t *selfShadowKeyframePtr;
        size_t selfShadowKeyframeCount;
    };

    static const uint8_t *kSignature;
    static const int kSignatureSize = 30;
    static const int kNameSize = 20;

    Motion(IModel *model, IEncoding *encoding);
    ~Motion();

    bool preparse(const uint8_t *data, size_t size, DataInfo &info);
    bool load(const uint8_t *data, size_t size);
    void save(uint8_t *data) const;
    size_t estimateSize() const;
    void setParentModel(IModel *model);
    void seek(float frameIndex);
    void advance(float delta);
    void reload();
    void reset();
    float maxFrameIndex() const;
    bool isReachedTo(float frameIndex) const;
    bool isNullFrameEnabled() const;
    void setNullFrameEnable(bool value);

    void addKeyframe(IKeyframe *value);
    int countKeyframes(IKeyframe::Type value) const;
    IBoneKeyframe *findBoneKeyframe(int frameIndex, const IString *name) const;
    ICameraKeyframe *findCameraKeyframe(int frameIndex) const;
    ILightKeyframe *findLightKeyframe(int frameIndex) const;
    IMorphKeyframe *findMorphKeyframe(int frameIndex, const IString *name) const;
    void replaceKeyframe(IKeyframe *value);
    void deleteKeyframe(IKeyframe *&value);
    void deleteKeyframes(int frameIndex, IKeyframe::Type type);
    void update(IKeyframe::Type type);

    const IString *name() const {
        return m_name;
    }
    IModel *parentModel() const {
        return m_model;
    }
    Error error() const {
        return m_error;
    }
    const BoneAnimation &boneAnimation() const {
        return m_boneMotion;
    }
    const CameraAnimation &cameraAnimation() const {
        return m_cameraMotion;
    }
    const MorphAnimation &morphAnimation() const {
        return m_morphMotion;
    }
    const LightAnimation &lightAnimation() const {
        return m_lightMotion;
    }
    const DataInfo &result() const {
        return m_result;
    }
    BoneAnimation *mutableBoneAnimation() {
        return &m_boneMotion;
    }
    CameraAnimation *mutableCameraAnimation() {
        return &m_cameraMotion;
    }
    MorphAnimation *mutableMorphAnimation() {
        return &m_morphMotion;
    }
    LightAnimation *mutableLightAnimation() {
        return &m_lightMotion;
    }
    bool isActive() const {
        return m_active;
    }

private:
    void parseHeader(const DataInfo &info);
    void parseBoneFrames(const DataInfo &info);
    void parseMorphFrames(const DataInfo &info);
    void parseCameraFrames(const DataInfo &info);
    void parseLightFrames(const DataInfo &info);
    void parseSelfShadowFrames(const DataInfo &info);
    void release();

    IModel *m_model;
    IEncoding *m_encoding;
    IString *m_name;
    DataInfo m_result;
    BoneAnimation m_boneMotion;
    CameraAnimation m_cameraMotion;
    MorphAnimation m_morphMotion;
    LightAnimation m_lightMotion;
    Error m_error;
    bool m_active;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Motion)
};

}
}

#endif

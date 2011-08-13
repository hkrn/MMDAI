/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef VPVL_VMDMOTION_H_
#define VPVL_VMDMOTION_H_

#include <LinearMath/btHashMap.h>
#include "vpvl/BoneAnimation.h"
#include "vpvl/CameraAnimation.h"
#include "vpvl/FaceAnimation.h"

class PMDModel;

namespace vpvl
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

class VPVL_EXPORT VMDMotion
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
        kFaceKeyFramesSizeError,
        kFaceKeyFramesError,
        kCameraKeyFramesSizeError,
        kCameraKeyFramesError,
        kMaxErrors
    };

    /**
     * Status of current motion.
     */
    enum MotionStatus
    {
        kRunning,
        kLooped,
        kDeleted
    };

    struct DataInfo
    {
        const uint8_t *basePtr;
        const uint8_t *namePtr;
        const uint8_t *boneKeyFramePtr;
        size_t boneKeyFrameCount;
        const uint8_t *faceKeyFramePtr;
        size_t faceKeyFrameCount;
        const uint8_t *cameraKeyFramePtr;
        size_t cameraKeyFrameCount;
        const uint8_t *lightKeyFramePtr;
        size_t lightKeyFrameCount;
        const uint8_t *selfShadowKeyFramePtr;
        size_t selfShadowKeyFrameCount;
    };

    static const float kDefaultPriority;
    static const float kDefaultLoopAtFrame;

    VMDMotion();
    ~VMDMotion();

    bool preparse(const uint8_t *data, size_t size, DataInfo &info);
    bool load(const uint8_t *data, size_t size);
    size_t estimateSize();
    void save(uint8_t *data);
    void attachModel(PMDModel *model);
    void seek(float frameIndex);
    void update(float deltaFrame);

    const uint8_t *name() const {
        return m_name;
    }
    PMDModel *attachedModel() const {
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
    const FaceAnimation &faceAnimation() const {
        return m_faceMotion;
    }
    const MotionStatus &status() const {
        return m_status;
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
    FaceAnimation *mutableFaceAnimation() {
        return &m_faceMotion;
    }
    float loopAt() const {
        return m_loopAt;
    }
    float priority() const {
        return m_priority;
    }
    bool enableSmooth() const {
        return m_enableSmooth;
    }
    bool enableRelocation() const {
        return m_enableRelocation;
    }
    bool isActive() const {
        return m_active;
    }
    void setLoop(bool value) {
        m_onEnd = value ? 1 : 2;
    }
    void setFull(bool value) {
        m_ignoreStatic = !value;
    }
    void setEnableSmooth(bool value) {
        m_enableSmooth = value;
    }
    void setEnableRelocation(bool value) {
        m_enableRelocation = value;
    }

private:
    void parseHeader(const DataInfo &info);
    void parseBoneFrames(const DataInfo &info);
    void parseFaceFrames(const DataInfo &info);
    void parseCameraFrames(const DataInfo &info);
    void parseLightFrames(const DataInfo &info);
    void parseSelfShadowFrames(const DataInfo &info);
    void release();

    uint8_t m_name[20];
    PMDModel *m_model;
    DataInfo m_result;
    BoneAnimation m_boneMotion;
    CameraAnimation m_cameraMotion;
    FaceAnimation m_faceMotion;
    MotionStatus m_status;
    Error m_error;
    uint8_t m_onEnd;
    float m_loopAt;
    float m_priority;
    float m_endingBoneBlend;
    float m_endingFaceBlend;
    float m_endingBoneBlendFrames;
    float m_endingFaceBlendFrames;
    float m_motionBlendRate;
    float m_beginningNonControlledBlend;
    bool m_active;
    bool m_enableSmooth;
    bool m_enableRelocation;
    bool m_ignoreStatic;

    VPVL_DISABLE_COPY_AND_ASSIGN(VMDMotion)
};

}

#endif

/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
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

#include "LinearMath/btHashMap.h"
#include "vpvl/BoneMotion.h"
#include "vpvl/CameraMotion.h"
#include "vpvl/FaceMotion.h"

namespace vpvl
{

struct VMDMotionDataInfo
{
    const char *basePtr;
    const char *namePtr;
    const char *boneKeyFramePtr;
    size_t boneKeyFrameCount;
    const char *faceKeyFramePtr;
    size_t faceKeyFrameCount;
    const char *cameraKeyFramePtr;
    size_t cameraKeyFrameCount;
    const char *lightKeyFramePtr;
    size_t lightKeyFrameCount;
    const char *selfShadowKeyFramePtr;
    size_t selfShadowKeyFrameCount;
};

class VMDMotion
{
public:
    VMDMotion(const char *data, size_t size);
    ~VMDMotion();

    bool preparse();
    bool parse();

    const char *name() const {
        return m_name;
    }
    const char *data() const {
        return m_data;
    }
    size_t size() const {
        return m_size;
    }
    const BoneMotion &bone() const {
        return m_boneMotion;
    }
    const CameraMotion &camera() const {
        return m_cameraMotion;
    }
    const FaceMotion &face() const {
        return m_faceMotion;
    }
    const VMDMotionDataInfo &result() const {
        return m_result;
    }

private:
    void parseHeader();
    void parseBoneFrames();
    void parseFaceFrames();
    void parseCameraFrames();
    void parseLightFrames();
    void parseSelfShadowFrames();

    char m_name[20];
    VMDMotionDataInfo m_result;
    BoneMotion m_boneMotion;
    CameraMotion m_cameraMotion;
    FaceMotion m_faceMotion;
    const char *m_data;
    const size_t m_size;
};

}

#endif

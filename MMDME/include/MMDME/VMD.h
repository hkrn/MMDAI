/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

#ifndef MMDME_VMD_H_
#define MMDME_VMD_H_

#include "MMDME/Common.h"
#include "MMDME/PTree.h"
#include "MMDME/IMotionLoader.h"

namespace MMDAI {

typedef struct _BoneKeyFrame {
    float keyFrame;
    btVector3 pos;
    btQuaternion rot;
    bool linear[4];
    float *interpolationTable[4];
} BoneKeyFrame;


typedef struct _BoneMotion {
    char *name;
    unsigned long numKeyFrame;
    BoneKeyFrame *keyFrameList;
} BoneMotion;


typedef struct _BoneMotionLink {
    BoneMotion boneMotion;
    struct _BoneMotionLink *next;
} BoneMotionLink;


typedef struct _FaceKeyFrame {
    float keyFrame;
    float weight;
} FaceKeyFrame;


typedef struct _FaceMotion {
    char *name;
    unsigned long numKeyFrame;
    FaceKeyFrame *keyFrameList;
} FaceMotion;


typedef struct _FaceMotionLink {
    FaceMotion faceMotion;
    struct _FaceMotionLink *next;
} FaceMotionLink;


class VMD
{
public:
    static const int kInterpolationTableSize = 64;

    VMD();
    ~VMD();

    bool load(IMotionLoader *loader);
    bool parse(unsigned char *data, size_t size);

    inline const unsigned int getTotalKeyFrame() const {
        return m_numTotalBoneKeyFrame + m_numTotalFaceKeyFrame;
    }
    inline BoneMotionLink * getBoneMotionLink() const {
        return m_boneLink;
    }
    inline FaceMotionLink * getFaceMotionLink() const {
        return m_faceLink;
    }
    inline const uint32_t countBoneKind() const {
        return m_numBoneKind;
    }
    inline const uint32_t countFaceKind() const {
        return m_numFaceKind;
    }
    inline const float getMaxFrame() const {
        return m_maxFrame;
    }

private:
    void addBoneMotion(const char *name);
    void addFaceMotion(const char *name);
    BoneMotion * getBoneMotion(const char *name);
    FaceMotion * getFaceMotion(const char *name);
    void setInterpolationTable(BoneKeyFrame *bf, char ip[]);
    void release();

    PTree m_name2bone;
    PTree m_name2face;
    BoneMotionLink *m_boneLink;
    FaceMotionLink *m_faceLink;
    uint32_t m_numBoneKind;
    uint32_t m_numFaceKind;
    uint32_t m_numTotalBoneKeyFrame;
    uint32_t m_numTotalFaceKeyFrame;
    float m_maxFrame;

    MMDME_DISABLE_COPY_AND_ASSIGN(VMD);
};

} /* namespace */

#endif


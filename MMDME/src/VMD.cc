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

#include "MMDME/MMDME.h"

namespace MMDAI {

static int compareBoneKeyFrame(const void *x, const void *y)
{
    const BoneKeyFrame *a = static_cast<const BoneKeyFrame *>(x);
    const BoneKeyFrame *b = static_cast<const BoneKeyFrame *>(y);
    return static_cast<int>(a->keyFrame - b->keyFrame);
}

static int compareFaceKeyFrame(const void *x, const void *y)
{
    const FaceKeyFrame *a = static_cast<const FaceKeyFrame *>(x);
    const FaceKeyFrame *b = static_cast<const FaceKeyFrame *>(y);
    return static_cast<int>(a->keyFrame - b->keyFrame);
}

static int compareCameraKeyFrame(const void *x, const void *y)
{
    const CameraKeyFrame *a = static_cast<const CameraKeyFrame *>(x);
    const CameraKeyFrame *b = static_cast<const CameraKeyFrame *>(y);
    return static_cast<int>(a->keyFrame - b->keyFrame);
}

static float ipfunc(const float t, const float p1,  const float p2)
{
    return ((1 + 3 * p1 - 3 * p2) * t * t * t + (3 * p2 - 6 * p1) * t * t + 3 * p1 * t);
}

static float ipfuncd(const float t, const float p1, const float p2)
{
    return ((3 + 9 * p1 - 9 * p2) * t * t + (6 * p2 - 12 * p1) * t + 3 * p1);
}

VMD::VMD()
    : m_boneLink(NULL),
      m_faceLink(NULL),
      m_cameraMotion(NULL),
      m_numBoneKind(0),
      m_numFaceKind(0),
      m_numTotalBoneKeyFrame(0),
      m_numTotalFaceKeyFrame(0),
      m_numTotalCameraKeyFrame(0),
      m_maxFrame(0.0f)
{
}

VMD::~VMD()
{
    release();
}

void VMD::release()
{
    m_name2bone.release();
    m_name2face.release();

    BoneMotionLink *bl = m_boneLink;
    while (bl) {
        if (bl->boneMotion.keyFrameList) {
            for (uint32_t i = 0; i < bl->boneMotion.numKeyFrame; i++)
                for (int j = 0; j < 4; j++)
                    if (bl->boneMotion.keyFrameList[i].linear[j] == false)
                        MMDAIMemoryRelease(bl->boneMotion.keyFrameList[i].interpolationTable[j]);
            MMDAIMemoryRelease(bl->boneMotion.keyFrameList);
        }
        if(bl->boneMotion.name)
            MMDAIMemoryRelease(bl->boneMotion.name);
        BoneMotionLink *bl_tmp = bl->next;
        MMDAIMemoryRelease(bl);
        bl = bl_tmp;
    }

    FaceMotionLink *fl = m_faceLink;
    while (fl) {
        if (fl->faceMotion.keyFrameList)
            MMDAIMemoryRelease(fl->faceMotion.keyFrameList);
        if(fl->faceMotion.name)
            MMDAIMemoryRelease(fl->faceMotion.name);
        FaceMotionLink *fl_tmp = fl->next;
        MMDAIMemoryRelease(fl);
        fl = fl_tmp;
    }

    if (m_cameraMotion != NULL) {
        if (m_cameraMotion->keyFrameList != NULL) {
            for (uint32_t i = 0; i < m_cameraMotion->numKeyFrame; i++) {
                for (int j = 0; j < 6; j++)
                    if (!m_cameraMotion->keyFrameList[i].linear[j])
                        MMDAIMemoryRelease(m_cameraMotion->keyFrameList[i].interpolationTable[j]);
            }
            MMDAIMemoryRelease(m_cameraMotion->keyFrameList);
        }
        MMDAIMemoryRelease(m_cameraMotion);
    }

    m_boneLink = NULL;
    m_faceLink = NULL;
    m_numBoneKind = 0;
    m_numFaceKind = 0;
    m_numTotalBoneKeyFrame = 0;
    m_numTotalFaceKeyFrame = 0;
    m_maxFrame = 0.0f;
}

void VMD::addBoneMotion(const char *name)
{
    assert(name != NULL);

    BoneMotionLink *link = static_cast<BoneMotionLink *>(MMDAIMemoryAllocate(sizeof(BoneMotionLink)));
    if (link == NULL) {
        MMDAILogWarnString("cannot allocate memory");
        return;
    }

    BoneMotion *bmNew = &(link->boneMotion);
    bmNew->name = MMDAIStringClone(name);
    bmNew->numKeyFrame = 1;
    bmNew->keyFrameList = NULL;

    link->next = m_boneLink;
    m_boneLink = link;

    BoneMotion *bmMatch = static_cast<BoneMotion *>(m_name2bone.findNearest(name));
    if (!bmMatch || !MMDAIStringEquals(bmMatch->name, name))
        m_name2bone.add(name, bmNew, (bmMatch) ? bmMatch->name : NULL);
}

void VMD::addFaceMotion(const char *name)
{
    assert(name != NULL);

    FaceMotionLink *link = static_cast<FaceMotionLink *>(MMDAIMemoryAllocate(sizeof(FaceMotionLink)));
    if (link == NULL) {
        MMDAILogWarnString("cannot allocate memory");
        return;
    }

    FaceMotion *fmNew = &(link->faceMotion);
    fmNew->name = MMDAIStringClone(name);
    fmNew->numKeyFrame = 1;
    fmNew->keyFrameList = NULL;

    link->next = m_faceLink;
    m_faceLink = link;

    FaceMotion *fmMatch = static_cast<FaceMotion *>(m_name2face.findNearest(name));
    if (!fmMatch || !MMDAIStringEquals(fmMatch->name, name))
        m_name2face.add(name, fmNew, (fmMatch) ? fmMatch->name : NULL);
}

BoneMotion* VMD::getBoneMotion(const char *name)
{
    if (name == NULL)
        return NULL;

    BoneMotion *bm = static_cast<BoneMotion *>(m_name2bone.findNearest(name));
    if (bm && MMDAIStringEquals(bm->name, name))
        return bm;

    return NULL;
}

FaceMotion* VMD::getFaceMotion(const char *name)
{
    if(name == NULL)
        return NULL;

    FaceMotion *fm = static_cast<FaceMotion *>(m_name2face.findNearest(name));
    if (fm && MMDAIStringEquals(fm->name, name))
        return fm;

    return NULL;
}

void VMD::setBoneInterpolationTable(BoneKeyFrame *bf, char ip[])
{
    /* check if they are just a linear function */
    for (int i = 0; i < 4; i++)
        bf->linear[i] = (ip[0+i] == ip[4+i] && ip[8+i] == ip[12+i]) ? true : false;

    /* make X (0.0 - 1.0) -> Y (0.0 - 1.0) mapping table */
    for (int i = 0; i < 4; i++) {
        if (bf->linear[i]) {
            /* table not needed */
            bf->interpolationTable[i] = NULL;
            continue;
        }
        bf->interpolationTable[i] = static_cast<float *>(MMDAIMemoryAllocate(sizeof(float) * (kBoneInterpolationTableSize + 1)));
        if (bf->interpolationTable[i] == NULL)
            return;
        float x1 = ip[   i] / 127.0f;
        float y1 = ip[ 4+i] / 127.0f;
        float x2 = ip[ 8+i] / 127.0f;
        float y2 = ip[12+i] / 127.0f;
        for (int d = 0; d < kBoneInterpolationTableSize; d++) {
            float inval = static_cast<float>(d) / kBoneInterpolationTableSize;
            /* get Y value for given inval */
            float t = inval;
            while (1) {
                float v = ipfunc(t, x1, x2) - inval;
                if (fabsf(v) < 0.0001f)
                    break;
                float tt = ipfuncd(t, x1, x2);
                if (tt == 0.0f)
                    break;
                t -= v / tt;
            }
            bf->interpolationTable[i][d] = ipfunc(t, y1, y2);
        }
        bf->interpolationTable[i][kBoneInterpolationTableSize] = 1.0f;
    }
}

void VMD::setCameraInterpolationTable(CameraKeyFrame *cf, char ip[])
{
    /* check if they are just a linear function */
    for (int i = 0; i < 6; i++)
        cf->linear[i] = (ip[i*4] == ip[i*4+2] && ip[i*4+1] == ip[i*4+3]) ? true : false;

    /* make X (0.0 - 1.0) -> Y (0.0 - 1.0) mapping table */
    for (int i = 0; i < 6; i++) {
        if (cf->linear[i]) {
            /* table not needed */
            cf->interpolationTable[i] = NULL;
            continue;
        }
        cf->interpolationTable[i] = static_cast<float *>(MMDAIMemoryAllocate(sizeof(float) * (kCameraInterpolationTableSize + 1)));
        float x1 = ip[i*4  ] / 127.0f;
        float y1 = ip[i*4+2] / 127.0f;
        float x2 = ip[i*4+1] / 127.0f;
        float y2 = ip[i*4+3] / 127.0f;
        for (int d = 0; d < kCameraInterpolationTableSize; d++) {
            float inval = static_cast<float>(d) / kCameraInterpolationTableSize;
            /* get Y value for given inval */
            float t = inval;
            while (1) {
                float v = ipfunc(t, x1, x2) - inval;
                if (fabsf(v) < 0.0001f)
                    break;
                float tt = ipfuncd(t, x1, x2);
                if (tt == 0.0f)
                    break;
                t -= v / tt;
            }
            cf->interpolationTable[i][d] = ipfunc(t, y1, y2);
        }
        cf->interpolationTable[i][kCameraInterpolationTableSize] = 1.0f;
    }
}

bool VMD::load(IMotionLoader *loader)
{
    unsigned char *ptr = NULL;
    size_t size = 0;
    if (!loader->loadMotionData(&ptr, &size))
        return false;
    bool ret = parse(ptr, size);
    loader->unloadMotionData(ptr);
    return ret;
}

bool VMD::parse(unsigned char *data, size_t size)
{
    uint32_t i = 0;

    char name[16];
    size_t rest = size;

    /* free VMD */
    release();

    /* header */
    VMDFile_Header *header = reinterpret_cast<VMDFile_Header *>(data);
    if (sizeof(VMDFile_Header) > rest || !MMDAIStringEqualsIn(header->header, "Vocaloid Motion Data 0002", sizeof(header->header)))
        return false;

    data += sizeof(VMDFile_Header);
    rest -= sizeof(VMDFile_Header);

    /* bone motions */
    m_numTotalBoneKeyFrame = *reinterpret_cast<uint32_t *>(data);
    data += sizeof(uint32_t);
    rest -= sizeof(uint32_t);

    if (m_numTotalBoneKeyFrame * sizeof(VMDFile_BoneFrame) > rest) {
        MMDAILogWarnString("size of bones exceeds size of VMD");
        return false;
    }

    VMDFile_BoneFrame *boneFrame = reinterpret_cast<VMDFile_BoneFrame *>(data);

    /* list bones that exists in the data and count the number of defined key frames for each */
    for (i = 0; i < m_numTotalBoneKeyFrame; i++) {
        MMDAIStringCopySafe(name, boneFrame[i].name, sizeof(name));
        BoneMotion *bm = getBoneMotion(name);
        if (bm)
            bm->numKeyFrame++;
        else
            addBoneMotion(name);
    }
    /* allocate memory to store the key frames, and reset count again */
    for (BoneMotionLink *bl = m_boneLink; bl; bl = bl->next) {
        bl->boneMotion.keyFrameList = static_cast<BoneKeyFrame *>(MMDAIMemoryAllocate(sizeof(BoneKeyFrame) * bl->boneMotion.numKeyFrame));
        bl->boneMotion.numKeyFrame = 0;
    }
    /* store the key frames, parsing the data again. also compute max frame */
    for (i = 0; i < m_numTotalBoneKeyFrame; i++) {
        MMDAIStringCopySafe(name, boneFrame[i].name, sizeof(name));
        BoneMotion *bm = getBoneMotion(name);
        if (bm) {
            bm->keyFrameList[bm->numKeyFrame].keyFrame = static_cast<float>(boneFrame[i].keyFrame);
            if (m_maxFrame < bm->keyFrameList[bm->numKeyFrame].keyFrame)
                m_maxFrame = bm->keyFrameList[bm->numKeyFrame].keyFrame;
            /* convert from left-hand coordinates to right-hand coordinates */
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
            bm->keyFrameList[bm->numKeyFrame].pos = btVector3(boneFrame[i].pos[0], boneFrame[i].pos[1], -boneFrame[i].pos[2]);
            bm->keyFrameList[bm->numKeyFrame].rot = btQuaternion(-boneFrame[i].rot[0], -boneFrame[i].rot[1], boneFrame[i].rot[2], boneFrame[i].rot[3]);
#else
            bm->keyFrameList[bm->numKeyFrame].pos = btVector3(boneFrame[i].pos[0], boneFrame[i].pos[1], boneFrame[i].pos[2]);
            bm->keyFrameList[bm->numKeyFrame].rot = btQuaternion(boneFrame[i].rot[0], boneFrame[i].rot[1], boneFrame[i].rot[2], boneFrame[i].rot[3]);
#endif
            /* set interpolation table */
            setBoneInterpolationTable(&(bm->keyFrameList[bm->numKeyFrame]), boneFrame[i].interpolation);
            bm->numKeyFrame++;
        }
    }
    /* sort the key frames in each boneMotion by frame */
    for (BoneMotionLink *bl = m_boneLink; bl; bl = bl->next)
        qsort(bl->boneMotion.keyFrameList, bl->boneMotion.numKeyFrame, sizeof(BoneKeyFrame), compareBoneKeyFrame);
    /* count number of bones appear in this vmd */
    m_numBoneKind = 0;
    for (BoneMotionLink *bl = m_boneLink; bl; bl = bl->next)
        m_numBoneKind++;

    data += sizeof(VMDFile_BoneFrame) * m_numTotalBoneKeyFrame;
    rest -= sizeof(VMDFile_BoneFrame) * m_numTotalBoneKeyFrame;

    /* face motions */
    m_numTotalFaceKeyFrame = *reinterpret_cast<uint32_t *>(data);
    data += sizeof(uint32_t);
    rest -= sizeof(uint32_t);

    if (m_numTotalFaceKeyFrame * sizeof(VMDFile_FaceFrame) > rest) {
        MMDAILogWarnString("size of faces exceeds size of VMD");
        return false;
    }

    VMDFile_FaceFrame *faceFrame = reinterpret_cast<VMDFile_FaceFrame *>(data);

    /* list faces that exists in the data and count the number of defined key frames for each */
    for (i = 0; i < m_numTotalFaceKeyFrame; i++) {
        MMDAIStringCopySafe(name, faceFrame[i].name, sizeof(name));
        FaceMotion *fm = getFaceMotion(name);
        if (fm)
            fm->numKeyFrame++;
        else
            addFaceMotion(name);
    }
    /* allocate memory to store the key frames, and reset count again */
    for (FaceMotionLink *fl = m_faceLink; fl; fl = fl->next) {
        fl->faceMotion.keyFrameList = static_cast<FaceKeyFrame *>(MMDAIMemoryAllocate(sizeof(FaceKeyFrame) * fl->faceMotion.numKeyFrame));
        fl->faceMotion.numKeyFrame = 0;
    }
    /* store the key frames, parsing the data again. also compute max frame */
    for (i = 0; i < m_numTotalFaceKeyFrame; i++) {
        MMDAIStringCopySafe(name, faceFrame[i].name, sizeof(name));
        FaceMotion *fm = getFaceMotion(name);
        if (fm) {
            fm->keyFrameList[fm->numKeyFrame].keyFrame = static_cast<float>(faceFrame[i].keyFrame);
            if (m_maxFrame < fm->keyFrameList[fm->numKeyFrame].keyFrame)
                m_maxFrame = fm->keyFrameList[fm->numKeyFrame].keyFrame;
            fm->keyFrameList[fm->numKeyFrame].weight = faceFrame[i].weight;
            fm->numKeyFrame++;
        }
    }
    /* sort the key frames in each faceMotion by frame */
    for (FaceMotionLink *fl = m_faceLink; fl; fl = fl->next)
        qsort(fl->faceMotion.keyFrameList, fl->faceMotion.numKeyFrame, sizeof(FaceKeyFrame), compareFaceKeyFrame);

    /* count number of faces appear in this vmd */
    m_numFaceKind = 0;
    for (FaceMotionLink *fl = m_faceLink; fl; fl = fl->next)
        m_numFaceKind++;

    data += sizeof(VMDFile_FaceFrame) * m_numTotalFaceKeyFrame;
    rest -= sizeof(VMDFile_FaceFrame) * m_numTotalFaceKeyFrame;

    m_numTotalCameraKeyFrame = *reinterpret_cast<uint32_t *>(data);
    data += sizeof(uint32_t);
    rest -= sizeof(uint32_t);

    if (m_numTotalCameraKeyFrame > 0 && rest >= m_numTotalCameraKeyFrame * sizeof(VMDFile_CameraFrame)) {
        VMDFile_CameraFrame *cameraFrame = reinterpret_cast<VMDFile_CameraFrame *>(data);
        m_cameraMotion = static_cast<CameraMotion *>(MMDAIMemoryAllocate(sizeof(CameraMotion)));
        m_cameraMotion->numKeyFrame = m_numTotalCameraKeyFrame;
        m_cameraMotion->keyFrameList = static_cast<CameraKeyFrame *>(MMDAIMemoryAllocate(sizeof(CameraKeyFrame) * m_cameraMotion->numKeyFrame));
        for (i = 0; i < m_cameraMotion->numKeyFrame; i++) {
            m_cameraMotion->keyFrameList[i].keyFrame = (float) cameraFrame[i].keyFrame;
            m_cameraMotion->keyFrameList[i].distance = - cameraFrame[i].distance;
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
            m_cameraMotion->keyFrameList[i].pos = btVector3(cameraFrame[i].pos[0], cameraFrame[i].pos[1], - cameraFrame[i].pos[2]);
            m_cameraMotion->keyFrameList[i].angle = btVector3(- MMDAIMathDegree(cameraFrame[i].angle[0]), -MMDAIMathDegree(cameraFrame[i].angle[1]),
                                                              MMDAIMathDegree(cameraFrame[i].angle[2]));
#else
            m_cameraMotion->keyFrameList[i].pos = btVector3(cameraFrame[i].pos[0], cameraFrame[i].pos[1], cameraFrame[i].pos[2]);
            m_cameraMotion->keyFrameList[i].angle = btVector3(MMDAIMathDegree(cameraFrame[i].angle[0]), MMDAIMathDegree(cameraFrame[i].angle[1]),
                                                              MMDAIMathDegree(cameraFrame[i].angle[2]));
#endif
            m_cameraMotion->keyFrameList[i].fovy = (float) cameraFrame[i].viewAngle;
            m_cameraMotion->keyFrameList[i].noPerspective = cameraFrame[i].noPerspective;
            setCameraInterpolationTable(&(m_cameraMotion->keyFrameList[i]), cameraFrame[i].interpolation);
        }
        qsort(m_cameraMotion->keyFrameList, m_cameraMotion->numKeyFrame, sizeof(CameraKeyFrame), compareCameraKeyFrame);
        data += m_numTotalCameraKeyFrame * sizeof(VMDFile_CameraFrame);
        rest -= m_numTotalCameraKeyFrame * sizeof(VMDFile_CameraFrame);
    }

    int numLightFrame = 0, numSelfShadowFrame = 0;
    if (rest > 0) {
        numLightFrame = *reinterpret_cast<uint32_t *>(data);
        data += sizeof(uint32_t);
        rest -= sizeof(uint32_t);
        if (rest >= numLightFrame * sizeof(VMDFile_LightFrame)) {
            data += numLightFrame * sizeof(VMDFile_LightFrame);
            rest -= numLightFrame * sizeof(VMDFile_LightFrame);
        }
    }
    if (rest > 0) {
        numSelfShadowFrame = *reinterpret_cast<uint32_t *>(data);
        data += sizeof(uint32_t);
        rest -= sizeof(uint32_t);
        if (rest >= numSelfShadowFrame * sizeof(VMDFile_SelfShadowFrame)) {
            data += numSelfShadowFrame * sizeof(VMDFile_SelfShadowFrame);
            rest -= numSelfShadowFrame * sizeof(VMDFile_SelfShadowFrame);
        }
    }

    MMDAILogDebug("camera:%d light:%d selfShadow:%d", m_numTotalCameraKeyFrame, numLightFrame, numSelfShadowFrame);
    MMDAILogDebug("rest of VMD: %d (data size is %d)", rest, size);

    return true;
}

} /* namespace */


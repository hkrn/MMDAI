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

#include "vpvl/vpvl.h"
#include "vpvl/internal/BoneKeyFrame.h"
#include "vpvl/internal/CameraKeyFrame.h"
#include "vpvl/internal/FaceKeyFrame.h"
#include "vpvl/internal/VMDMotion.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

VMDMotion::VMDMotion(const char *data, size_t size)
    : m_data(data),
      m_size(size)
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_result, 0, sizeof(m_result));
}

VMDMotion::~VMDMotion()
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_result, 0, sizeof(m_result));
    m_data = 0;
}

bool VMDMotion::preparse()
{
    size_t rest = m_size;
    /* header + name */
    if (50 > rest)
        return false;

    char *ptr = const_cast<char *>(m_data);
    m_result.basePtr = ptr;

    if (strcmp(ptr, "Vocaloid Motion Data 0002") != 0)
        return false;
    ptr += 30;
    m_result.namePtr = ptr;
    ptr += 20;
    rest -= 50;

    /* bone key frame */
    size_t nBoneKeyFrames, nFaceKeyFrames, nCameraKeyFrames;
    if (!size32(ptr, rest, nBoneKeyFrames))
        return false;
    m_result.boneKeyFramePtr = ptr;
    if (!validateSize(ptr, BoneKeyFrame::stride(ptr), nBoneKeyFrames, rest))
        return false;
    m_result.boneKeyFrameCount = nBoneKeyFrames;

    /* face key frame */
    if (!size32(ptr, rest, nFaceKeyFrames))
        return false;
    m_result.faceKeyFramePtr = ptr;
    if (!validateSize(ptr, FaceKeyFrame::stride(ptr), nFaceKeyFrames, rest))
        return false;
    m_result.faceKeyFrameCount = nFaceKeyFrames;

    /* camera key frame */
    if (!size32(ptr, rest, nCameraKeyFrames))
        return false;
    m_result.cameraKeyFramePtr = ptr;
    if (!validateSize(ptr, CameraKeyFrame::stride(ptr), nCameraKeyFrames, rest))
        return false;
    m_result.cameraKeyFrameCount = nCameraKeyFrames;

    return true;
}

bool VMDMotion::load()
{
    if (preparse()) {
        parseHeader();
        parseBoneFrames();
        parseFaceFrames();
        parseCameraFrames();
        parseLightFrames();
        parseSelfShadowFrames();
        return true;
    }
    return false;
}

void VMDMotion::parseHeader()
{
    stringCopySafe(m_name, m_result.namePtr, sizeof(m_name));
}

void VMDMotion::parseBoneFrames()
{
    m_boneMotion.read(m_result.boneKeyFramePtr, m_result.boneKeyFrameCount);
    m_boneMotion.sort();
}

void VMDMotion::parseFaceFrames()
{
    m_faceMotion.read(m_result.faceKeyFramePtr, m_result.faceKeyFrameCount);
    m_faceMotion.sort();
}

void VMDMotion::parseCameraFrames()
{
    m_cameraMotion.read(m_result.cameraKeyFramePtr, m_result.cameraKeyFrameCount);
    m_cameraMotion.sort();
}

void VMDMotion::parseLightFrames()
{
}

void VMDMotion::parseSelfShadowFrames()
{
}

}

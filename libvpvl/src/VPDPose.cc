/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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
#include "vpvl/internal/util.h"

namespace vpvl
{

enum VPDPoseInternalParseState
{
    kNone,
    kPosition,
    kQuaternion,
    kEnd
};

VPDPose::VPDPose()
    : m_error(kNoError)
{
}

VPDPose::~VPDPose()
{
    release();
}

bool VPDPose::preparse(const uint8_t *data, size_t size)
{
    static const uint8_t signature[] = "Vocaloid Pose Data file";
    uint8_t *ptr = const_cast<uint8_t *>(data);
    if (sizeof(signature) > size) {
        m_error = kInvalidHeaderError;
        return false;
    }

    if (!internal::stringEquals(ptr, signature, sizeof(signature) - 1)) {
        m_error = kInvalidSignatureError;
        return false;
    }

    return true;
}

bool VPDPose::load(const uint8_t *data, size_t size)
{
    if (!preparse(data, size))
        return false;
    release();

    char *buffer = new char[size + 1];
    memcpy(buffer, data, size);
    buffer[size] = 0;

    Array<char *> tokens;
    char *p = 0, *token = 0, *ptr = buffer + 23;
    internal::stringToken(ptr, ";\r\n\t", &p); // rest
    internal::stringToken(NULL, ";\r\n\t", &p); // file
    size_t expected = internal::stringToInt(internal::stringToken(NULL, " ;\r\n\t", &p)); // size
    while ((token = internal::stringToken(NULL, "\r\n\t", &p))) {
        if (internal::stringEquals(token, "//", 2))
            continue;
        tokens.add(token);
    }

    try {
        VPDPoseInternalParseState state = kNone;
        Bone *bone = 0;
        size_t nTokens = tokens.count();
        for (uint32_t i = 0; i < nTokens; i++) {
            token = tokens[i];
            switch (state) {
            case kNone: {
                if (strlen(token) > 6 && internal::stringEquals(reinterpret_cast<const uint8_t *>(token),
                                                                reinterpret_cast<const uint8_t *>("Bone"), 4)) {
                    token += 4;
                    char *s = internal::stringToken(token, "0123456789{", &p);
                    size_t len = strlen(s) + 1;
                    bone = new Bone();
                    bone->name = new uint8_t[len];
                    bone->position.setZero();
                    bone->rotation.setZero();
                    m_bones.add(bone);
                    copyBytesSafe(bone->name, reinterpret_cast<const uint8_t *>(s), len);
                    state = kPosition;
                }
                else {
                    throw kBoneNameError;
                }
                break;
            }
            case kPosition: {
                char *x = internal::stringToken(token, ",", &p);
                char *y = internal::stringToken(NULL, ",", &p);
                char *z = internal::stringToken(NULL, ",;", &p);
                if (x && y && z) {
#ifdef VPVL_COORDINATE_OPENGL
                    bone->position.setValue(internal::stringToFloat(x),
                                            internal::stringToFloat(y),
                                            -internal::stringToFloat(z));
#else
                    bone->position.setValue(internal::stringToFloat(x),
                                            internal::stringToFloat(y),
                                            internal::stringToFloat(z));
#endif
                    state = kQuaternion;
                }
                else {
                    throw kPositionError;
                }
                break;
            }
            case kQuaternion:
            {
                char *x = internal::stringToken(token, ",", &p);
                char *y = internal::stringToken(NULL, ",", &p);
                char *z = internal::stringToken(NULL, ",", &p);
                char *w = internal::stringToken(NULL, ",;", &p);
                if (x && y && z) {
#ifdef VPVL_COORDINATE_OPENGL
                    bone->rotation.setValue(-internal::stringToFloat(x),
                                            -internal::stringToFloat(y),
                                            internal::stringToFloat(z),
                                            internal::stringToFloat(w));
#else
                    bone->rotation.setValue(internal::stringToFloat(x),
                                            internal::stringToFloat(y),
                                            internal::stringToFloat(z),
                                            internal::stringToFloat(w));
#endif
                    state = kEnd;
                }
                else {
                    throw kQuaternionError;
                }
                break;
            }
            case kEnd:
            {
                if (*token == '}')
                    state = kNone;
                else
                    throw kEndError;
            }
            }
        }
    } catch (Error e) {
        m_error = e;
    }
    delete[] buffer;

    if (expected != m_bones.count())
        return false;

    return true;
}

void VPDPose::makePose(vpvl::PMDModel *model)
{
    const uint32_t nBones = m_bones.count();
    for (uint32_t i = 0; i < nBones; i++) {
        Bone *b = m_bones[i];
        vpvl::Bone *bone = model->findBone(b->name);
        if (bone) {
            btVector3 pos = b->position;
            btVector4 rot = b->rotation;
            const btQuaternion rotation(rot.x(), rot.y(), rot.z(), rot.w());
            bone->setPosition(pos);
            bone->setRotation(rotation);
        }
    }
}

void VPDPose::release()
{
    uint32_t size = m_bones.count();
    for (uint32_t i = 0; i < size; i++) {
        Bone *bone = m_bones[i];
        delete[] bone->name;
        delete bone;
    }
    m_error = kNoError;
}

}


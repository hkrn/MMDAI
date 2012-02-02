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

namespace vpvl2
{
namespace pmx
{

#pragma pack(push, 1)

struct BoneUnit {
    float position[3];
};

#pragma pack(pop)

Bone::Bone()
{
}

Bone::~Bone()
{
}

bool Bone::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.bonesPtr = ptr;
    /* BoneUnit + boneIndexSize + hierarcy + flags */
    size_t baseSize = sizeof(BoneUnit) + info.boneIndexSize + sizeof(int) + sizeof(uint16_t);
    for (size_t i = 0; i < size; i++) {
        size_t nNameSize;
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        /* name in English */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        if (baseSize > rest) {
            return false;
        }
        internal::drain(baseSize, ptr, rest);
        uint16_t flags = *reinterpret_cast<uint16_t *>(ptr - 2);
        /* bone has destination */
        bool isRelative = flags & 0x0001 == 1;
        if (isRelative) {
            if (info.boneIndexSize > rest) {
                return false;
            }
            internal::drain(info.boneIndexSize, ptr, rest);
        }
        else {
            if (sizeof(BoneUnit) > rest) {
                return false;
            }
            internal::drain(sizeof(BoneUnit), ptr, rest);
        }
        /* bone is IK */
        if (flags & 0x0020) {
            /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
            size_t extraSize = info.boneIndexSize + sizeof(int) + sizeof(float) + sizeof(int);
            if (extraSize > rest) {
                return false;
            }
            internal::drain(extraSize, ptr, rest);
            int nlinks = *reinterpret_cast<int *>(ptr - sizeof(int));
            for (int i = 0; i < nlinks; i++) {
                if (info.boneIndexSize > rest) {
                    return false;
                }
                internal::drain(info.boneIndexSize, ptr, rest);
                size_t hasAngleConstraint;
                if (!internal::size8(ptr, rest, hasAngleConstraint)) {
                    return false;
                }
                if (hasAngleConstraint == 1) {
                    extraSize = sizeof(BoneUnit) * 2;
                    if (extraSize > rest) {
                        return false;
                    }
                    internal::drain(extraSize, ptr, rest);
                }
            }
        }
        /* bone is additional rotation */
        if (flags & 0x0100 || flags & 0x200) {
            size_t extraSize = info.boneIndexSize + sizeof(float);
            if (extraSize > rest) {
                return false;
            }
            internal::drain(extraSize, ptr, rest);
        }
        /* axis of bone is fixed */
        if (flags & 0x0400) {
            if (sizeof(BoneUnit) > rest) {
                return false;
            }
            internal::drain(sizeof(BoneUnit), ptr, rest);
        }
        /* axis of bone is local */
        if (flags & 0x0800) {
            size_t extraSize = sizeof(BoneUnit) * 2;
            if (extraSize > rest) {
                return false;
            }
            internal::drain(extraSize, ptr, rest);
        }
        /* bone is transformed after external parent bone transformation */
        if (flags & 0x2000) {
            if (sizeof(int) > rest) {
                return false;
            }
            internal::drain(sizeof(int), ptr, rest);
        }
    }
    info.bonesCount = size;
    return true;
}

void Bone::read(const uint8_t *data)
{
}

void Bone::write(uint8_t *data) const
{
}

} /* namespace pmx */
} /* namespace vpvl2 */


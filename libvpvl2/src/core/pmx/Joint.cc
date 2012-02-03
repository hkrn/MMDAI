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

struct JointUnit
{
    float position[3];
    float rotation[3];
    float positionLowerLimit[3];
    float positionUpperLimit[3];
    float rotationLowerLimit[3];
    float rotationUpperLimit[3];
    float positionStiffness[3];
    float rotationStiffness[3];
};

#pragma pack(pop)

Joint::Joint()
{
}

Joint::~Joint()
{
}

bool Joint::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.jointsPtr = ptr;
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
        size_t type;
        if (!internal::size8(ptr, rest, type)) {
            return false;
        }
        switch (type) {
        case 0:
            if (!internal::validateSize(ptr, info.rigidBodyIndexSize * 2 + sizeof(JointUnit), rest)) {
                return false;
            }
            break;
        default:
            assert(0);
            return false;
        }
    }
    info.jointsCount = size;
    return true;
}

void Joint::read(const uint8_t *data)
{
}

void Joint::write(uint8_t *data) const
{
}

} /* namespace pmx */
} /* namespace vpvl2 */


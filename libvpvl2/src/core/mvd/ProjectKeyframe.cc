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

#include "vpvl2/mvd/ProjectKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct ProjectKeyframeChunk {
    ProjectKeyframeChunk() {}
    uint64_t timeIndex;
    float gravityFactor;
    float gravityDirection[3];
    int shadowMode;
    float shadowDistance;
    float shadowDepth;
};

#pragma pack(pop)

ProjectKeyframe::ProjectKeyframe(IEncoding *encoding)
    : BaseKeyframe(),
      m_encoding(encoding)
{
}

ProjectKeyframe::~ProjectKeyframe()
{
}

size_t ProjectKeyframe::size()
{
    static const ProjectKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool ProjectKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        return false;
    }
    return true;
}

void ProjectKeyframe::read(const uint8_t * /* data */)
{
}

void ProjectKeyframe::write(uint8_t * /* data */) const
{
}

size_t ProjectKeyframe::estimateSize() const
{
    return size();
}

/*
IProjectKeyframe *ProjectKeyframe::clone() const
{
    return 0;
}
*/

void ProjectKeyframe::setName(const IString * /* value */)
{
}

IKeyframe::Type ProjectKeyframe::type() const
{
    return kProject;
}

} /* namespace mvd */
} /* namespace vpvl2 */

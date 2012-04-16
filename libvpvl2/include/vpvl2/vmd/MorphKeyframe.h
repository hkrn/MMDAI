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

#ifndef VPVL2_VMD_MORPHKEYFRAME_H_
#define VPVL2_VMD_MORPHKEYFRAME_H_

#include "vpvl2/IMorphKeyframe.h"
#include "vpvl2/vmd/BaseKeyframe.h"

namespace vpvl2
{
class IEncoding;

namespace vmd
{

class VPVL2_API MorphKeyframe : public BaseKeyframe, public IMorphKeyframe
{
public:
    MorphKeyframe(IEncoding *encoding);
    ~MorphKeyframe();

    static const int kNameSize = 15;

    static size_t strideSize();

    size_t stride() const;
    void read(const uint8_t *data);
    void write(uint8_t *data) const;
    BaseKeyframe *clone() const;

    void setName(const IString *value);

    /**
     * Get the face weight of this keyframe.
     *
     * @return A value of face weight
     */
    float weight() const {
        return m_weight;
    }

    /**
     * Set weight of the face of this keyframe.
     *
     * @param A value of weight of the face
     */
    void setWeight(float value);

    Type type() const { return IKeyframe::kMorph; }

private:
    IEncoding *m_encoding;
    float m_weight;

    VPVL2_DISABLE_COPY_AND_ASSIGN(MorphKeyframe)
};

}
}

#endif

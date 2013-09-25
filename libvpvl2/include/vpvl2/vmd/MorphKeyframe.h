/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_VMD_MORPHKEYFRAME_H_
#define VPVL2_VMD_MORPHKEYFRAME_H_

#include "vpvl2/IMorphKeyframe.h"
#include "vpvl2/internal/Keyframe.h"

namespace vpvl2
{
class IEncoding;

namespace vmd
{

class VPVL2_API MorphKeyframe VPVL2_DECL_FINAL : public IMorphKeyframe
{
public:
    static vsize strideSize();

    MorphKeyframe(IEncoding *encoding);
    ~MorphKeyframe();

    static const int kNameSize = 15;

    void read(const uint8 *data);
    void write(uint8 *data) const;
    vsize estimateSize() const;
    IMorphKeyframe *clone() const;

    VPVL2_KEYFRAME_DEFINE_METHODS()
    IMorph::WeightPrecision weight() const {  return m_weight; }
    Type type() const { return IKeyframe::kMorphKeyframe; }

    void setName(const IString *value);
    void setWeight(const IMorph::WeightPrecision &value);

private:
    VPVL2_KEYFRAME_DEFINE_FIELDS()
    IEncoding *m_encodingRef;
    IMorph::WeightPrecision m_weight;

    VPVL2_DISABLE_COPY_AND_ASSIGN(MorphKeyframe)
};

}
}

#endif

/**

 Copyright (c) 2010-2013  hkrn

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
#ifndef VPVL2_PMD_MORPH_H_
#define VPVL2_PMD_MORPH_H_

#include "vpvl2/Common.h"
#include "vpvl2/IMorph.h"

#include "vpvl/Face.h"

namespace vpvl2
{

class IEncoding;
class IModel;
class IString;

namespace pmd
{

class VPVL2_API Morph : public IMorph
{
public:
    Morph(IModel *modelRef, vpvl::Face *morph, IEncoding *encoding);
    ~Morph();

    const IString *name() const { return m_name; }
    int index() const { return m_index; }
    IModel *parentModelRef() const { return m_modelRef; }
    Category category() const;
    Type type() const;
    bool hasParent() const;
    WeightPrecision weight() const;
    void resetTransform();
    void setWeight(const WeightPrecision &value);
    void setIndex(int value);

    IModel *m_modelRef;
    IEncoding *m_encodingRef;
    IString *m_name;
    vpvl::Face *m_morphRef;
    Category m_category;
    WeightPrecision m_weight;
    int m_index;
};

}
}

#endif

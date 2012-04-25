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
#include "vpvl2/pmd/Morph.h"

namespace vpvl2
{
namespace pmd
{

Morph::Morph(vpvl::Face *morph, IEncoding *encoding)
    : m_encoding(encoding),
      m_name(0),
      m_morph(morph),
      m_category(kReserved),
      m_weight(0),
      m_index(-1)
{
    m_name = encoding->toString(morph->name(), IString::kShiftJIS, vpvl::Face::kNameSize);
}

Morph::~Morph()
{
    delete m_name;
    m_name = 0;
    m_morph = 0;
    m_category = kReserved;
    m_weight = 0;
    m_index = -1;
}

IMorph::Category Morph::category() const
{
    switch (m_morph->type()) {
    case vpvl::Face::kEye:
        return kEye;
    case vpvl::Face::kEyeblow:
        return kEyeblow;
    case vpvl::Face::kLip:
        return kLip;
    case vpvl::Face::kOther:
        return kOther;
    case vpvl::Face::kBase:
    default:
        return kReserved;
    }
}

void Morph::setWeight(const Scalar &value)
{
    m_morph->setWeight(value);
}

void Morph::setIndex(int value)
{
    m_index = value;
}

}
}

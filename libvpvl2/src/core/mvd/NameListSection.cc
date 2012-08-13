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

#include "vpvl2/mvd/NameListSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct NameSectionHeader {
    int reserved;
    int reserved2;
    int count;
    int reserved3;
};

#pragma pack(pop)

NameListSection::NameListSection(IEncoding *encoding)
    : m_encoding(encoding)
{
}

NameListSection::~NameListSection()
{
    m_names.releaseAll();
    m_encoding = 0;
}

bool NameListSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo & /* info */)
{
    const NameSectionHeader &header = *reinterpret_cast<const NameSectionHeader *>(ptr);
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, header.reserved3, rest)) {
        return false;
    }
    static int keyIndex;
    uint8_t *namePtr;
    size_t nNameSize;
    const int nkeyframes = header.count;
    for (int i = 0; i < nkeyframes; i++) {
        if (!internal::validateSize(ptr, sizeof(keyIndex), rest)) {
            return false;
        }
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
    }
    return true;
}

void NameListSection::read(const uint8_t *data, const IString::Codec &codec)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    const NameSectionHeader &header = *reinterpret_cast<const NameSectionHeader *>(ptr);
    size_t rest = SIZE_MAX;
    uint8_t *namePtr;
    size_t nNameSize;
    const int nkeyframes = header.count;
    m_names.reserve(nkeyframes);
    for (int i = 0; i < nkeyframes; i++) {
        int keyIndex = internal::readUnsignedIndex(ptr, 4);
        internal::sizeText(ptr, rest, namePtr, nNameSize);
        m_names.add(m_encoding->toString(namePtr, codec, nNameSize));
        IString *s = m_names[i];
        m_key2values.insert(keyIndex, s);
        m_value2keys.insert(s, keyIndex);
    }
}

void NameListSection::write(uint8_t * /* data */) const
{
}

size_t NameListSection::estimateSize() const
{
    return 0;
}

int NameListSection::key(const IString *value)
{
    const int *key = m_value2keys.find(value);
    return key ? *key : -1;
}

const IString *NameListSection::value(int key)
{
    const IString *const *value = m_key2values.find(key);
    return value ? *value : 0;
}

} /* namespace mvd */
} /* namespace vpvl2 */

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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/mvd/NameListSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct NameSectionHeader {
    int32 reserved;
    int32 reserved2;
    int32 count;
    int32 reserved3;
};

#pragma pack(pop)

const int NameListSection::kNotFound = -1;

NameListSection::NameListSection(IEncoding *encoding)
    : m_encoding(encoding)
{
}

NameListSection::~NameListSection()
{
    setParentModel(0);
    m_encoding = 0;
}

bool NameListSection::preparse(uint8 *&ptr, vsize &rest, Motion::DataInfo & /* info */)
{
    NameSectionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVDNameListSection header detected: " << rest);
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    if (!internal::validateSize(ptr, header.reserved3, rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVDNameListSection reserved detected: size=" << header.reserved3 << " rest=" << rest);
        return false;
    }
    VPVL2_VLOG(2, "MVDNameListSection(Header): size=" << header.count);
    VPVL2_VLOG(2, "MVDNameListSection(Header): reserved1=" << header.reserved);
    VPVL2_VLOG(2, "MVDNameListSection(Header): reserved2=" << header.reserved2);
    VPVL2_VLOG(2, "MVDNameListSection(Header): reserved3=" << header.reserved3);
    uint8 *namePtr = 0;
    int32 size = 0;
    const int nkeyframes = header.count;
    for (int i = 0; i < nkeyframes; i++) {
        if (!internal::validateSize(ptr, sizeof(size), rest)) {
            VPVL2_LOG(WARNING, "Invalid size of MVDNameListSection key detected: index=" << i << " rest=" << rest);
            return false;
        }
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of MVDNameListSection value detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
    }
    return true;
}

void NameListSection::read(const uint8 *data, const IString::Codec &codec)
{
    uint8 *ptr = const_cast<uint8 *>(data);
    NameSectionHeader header;
    internal::getData(ptr, header);
    vsize rest = SIZE_MAX;
    uint8 *namePtr = 0;
    const int nnames = header.count;
    int32 size = 0;
    m_strings.reserve(nnames);
    ptr += sizeof(header) + header.reserved3;
    for (int i = 0; i < nnames; i++) {
        int keyIndex = internal::readUnsignedIndex(ptr, sizeof(i));
        internal::getText(ptr, rest, namePtr, size);
        m_strings.append(m_encoding->toString(namePtr, codec, size));
        const IString *s = m_strings[i];
        m_key2StringRefs.insert(keyIndex, s);
        m_string2Keys.insert(s->toHashString(), keyIndex);
    }
}

void NameListSection::write(uint8 *data, const Motion::DataInfo &info) const
{
    Motion::SectionTag tag;
    tag.type = Motion::kNameListSection;
    tag.minor = 0;
    internal::writeBytes(&tag, sizeof(tag), data);
    NameSectionHeader header;
    const int nnames = m_strings.count();
    header.count = nnames;
    header.reserved = header.reserved2 = header.reserved3 = 0;
    internal::writeBytes(&header, sizeof(header), data);
    const IString::Codec codec = info.codec;
    for (int32 i = 0; i < nnames; i++) {
        const IString *name = m_strings[i];
        internal::writeBytes(&i, sizeof(i), data);
        internal::writeString(name, codec, data);
    }
}

vsize NameListSection::estimateSize(const Motion::DataInfo &info) const
{
    vsize size = 0;
    size += sizeof(Motion::SectionTag);
    size += sizeof(NameSectionHeader);
    const int nnames = m_strings.count();
    const IString::Codec codec = info.codec;
    for (int32 i = 0; i < nnames; i++) {
        const IString *name = m_strings[i];
        size += sizeof(i);
        size += internal::estimateSize(name, codec);
    }
    return size;
}

int NameListSection::key(const IString *value) const
{
    if (value) {
        const int *key = m_string2Keys.find(value->toHashString());
        return key ? *key : kNotFound;
    }
    return kNotFound;
}

const IString *NameListSection::value(int key) const
{
    const IString *const *value = m_key2StringRefs.find(key);
    return value ? *value : 0;
}

void NameListSection::getNames(Array<const IString *> &names) const
{
    const int nnames = m_strings.count();
    for (int i = 0; i < nnames; i++) {
        names.append(m_strings[i]);
    }
}

void NameListSection::addName(const IString *name)
{
    const HashString &s = name->toHashString();
    if (!m_string2Keys.find(s)) {
        int key = m_strings.count();
        m_strings.append(name->clone());
        m_key2StringRefs.insert(key, name);
        m_string2Keys.insert(s, key);
    }
}

void NameListSection::setParentModel(const IModel *value)
{
    m_strings.releaseAll();
    m_key2StringRefs.clear();
    m_string2Keys.clear();
    if (value) {
        Array<IBone *> boneRefs;
        value->getBoneRefs(boneRefs);
        const int nbones = boneRefs.count();
        for (int i = 0; i < nbones; i++) {
            const IBone *bone = boneRefs[i];
            addName(bone->name(IEncoding::kDefaultLanguage));
        }
        Array<IMorph *> morphRefs;
        value->getMorphRefs(morphRefs);
        const int nmorphs = morphRefs.count();
        for (int i = 0; i < nmorphs; i++) {
            const IMorph *morph = morphRefs[i];
            addName(morph->name(IEncoding::kDefaultLanguage));
        }
    }
}

IEncoding *NameListSection::encodingRef() const
{
    return m_encoding;
}

} /* namespace mvd */
} /* namespace vpvl2 */

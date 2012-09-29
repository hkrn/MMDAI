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

#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Morph.h"

namespace
{

#pragma pack(push, 1)

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

struct Label::Pair {
    int id;
    int type;
    IBone *bone;
    IMorph *morph;
};

Label::Label()
    : m_name(0),
      m_englishName(0),
      m_index(-1),
      m_special(false)
{
}

Label::~Label()
{
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    m_pairs.releaseAll();
    m_index = -1;
    m_special = false;
}

bool Label::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t nDisplayNames;
    if (!internal::size32(ptr, rest, nDisplayNames)) {
        return false;
    }
    info.labelsPtr = ptr;
    for (size_t i = 0; i < nDisplayNames; i++) {
        size_t nNameSize;
        uint8_t *namePtr;
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        if (!internal::validateSize(ptr, sizeof(uint8_t), rest)) {
            return false;
        }
        size_t size;
        if (!internal::size32(ptr, rest, size)) {
            return false;
        }
        for (size_t j = 0; j < size; j++) {
            size_t type;
            if (!internal::size8(ptr, rest, type)) {
                return false;
            }
            switch (type) {
            case 0:
                if (!internal::validateSize(ptr, info.boneIndexSize, rest)) {
                    return false;
                }
                break;
            case 1:
                if (!internal::validateSize(ptr, info.morphIndexSize, rest)) {
                    return false;
                }
                break;
            default:
                return false;
            }
        }
    }
    info.labelsCount = nDisplayNames;
    return true;
}

bool Label::loadLabels(const Array<Label *> &labels, const Array<Bone *> &bones, const Array<Morph *> &morphs)
{
    const int nlabels = labels.count();
    const int nbones = bones.count();
    const int nmorphs = morphs.count();
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        const Array<Pair *> &pairs = label->m_pairs;
        const int npairs = pairs.count();
        for (int j = 0; j < npairs; j++) {
            Pair *pair = pairs[j];
            switch (pair->type) {
            case 0: {
                const int boneIndex = pair->id;
                if (boneIndex >= 0) {
                    if (boneIndex >= nbones)
                        return false;
                    else
                        pair->bone = bones[boneIndex];
                }
                break;
            }
            case 1: {
                const int morphIndex = pair->id;
                if (morphIndex >= 0) {
                    if (morphIndex >= nmorphs)
                        return false;
                    else
                        pair->morph = morphs[morphIndex];
                }
                break;
            }
            default:
                assert(0);
                return false;
            }
        }
        label->m_index = i;
    }
    return true;
}

void Label::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    IEncoding *encoding = info.encoding;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_name);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_englishName);
    internal::size8(ptr, rest, nNameSize);
    m_special = nNameSize == 1;
    internal::size32(ptr, rest, nNameSize);
    for (size_t i = 0; i < nNameSize; i++) {
        size_t type;
        internal::size8(ptr, rest, type);
        Pair *pair = new Pair();
        m_pairs.add(pair);
        pair->bone = 0;
        pair->morph = 0;
        pair->type = type;
        switch (type) {
        case 0:
            pair->id = internal::readSignedIndex(ptr, info.boneIndexSize);
            break;
        case 1:
            pair->id = internal::readSignedIndex(ptr, info.morphIndexSize);
            break;
        default:
            assert(0);
            return;
        }
    }
    size = ptr - start;
}

void Label::write(uint8_t *data, const Model::DataInfo &info) const
{
    internal::writeString(m_name, info.codec, data);
    internal::writeString(m_englishName, info.codec, data);
    int npairs = m_pairs.count();
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_special), sizeof(uint8_t), data);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&npairs), sizeof(npairs), data);
    for (int i = 0; i < npairs; i++) {
        const Pair *pair = m_pairs[i];
        const uint8_t type = pair->type;
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&type), sizeof(type), data);
        switch (pair->type) {
        case 0:
            internal::writeSignedIndex(pair->id, info.boneIndexSize, data);
            break;
        case 1:
            internal::writeSignedIndex(pair->id, info.morphIndexSize, data);
            break;
        default:
            return;
        }
    }
}

size_t Label::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0;
    size += internal::estimateSize(m_name, info.codec);
    size += internal::estimateSize(m_englishName, info.codec);
    int npairs = m_pairs.count();
    for (int i = 0; i < npairs; i++) {
        const Pair *pair = m_pairs[i];
        size += sizeof(uint8_t);
        switch (pair->type) {
        case 0:
            size += info.boneIndexSize;
            break;
        case 1:
            size += info.morphIndexSize;
            break;
        default:
            return 0;
        }
    }
    return size;
}

IBone *Label::bone(int index) const
{
    if (index >= 0 && index < m_pairs.count())
        return m_pairs[index]->bone;
    return 0;
}

IMorph *Label::morph(int index) const
{
    if (index >= 0 && index < m_pairs.count())
        return m_pairs[index]->morph;
    return 0;
}

int Label::count() const
{
    return m_pairs.count();
}

void Label::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void Label::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
}

void Label::setSpecial(bool value)
{
    m_special = value;
}

void Label::addBone(IBone *value)
{
    if (value) {
        Pair *pair = new Pair();
        pair->bone = value;
        pair->id = value->index();
        pair->morph = 0;
        pair->type = 0;
        m_pairs.add(pair);
    }
}

void Label::addMorph(IMorph *value)
{
    if (value) {
        Pair *pair = new Pair();
        pair->bone = 0;
        pair->id = value->index();
        pair->morph = value;
        pair->type = 1;
        m_pairs.add(pair);
    }
}

void Label::removeBone(IBone *value)
{
    const int npairs = m_pairs.count();
    for (int i = 0; i < npairs; i++) {
        Pair *pair = m_pairs[i];
        if (pair->bone == value) {
            m_pairs.remove(pair);
            delete pair;
            break;
        }
    }
}

void Label::removeMorph(IMorph *value)
{
    const int npairs = m_pairs.count();
    for (int i = 0; i < npairs; i++) {
        Pair *pair = m_pairs[i];
        if (pair->morph == value) {
            m_pairs.remove(pair);
            delete pair;
            break;
        }
    }
}

} /* namespace pmx */
} /* namespace vpvl2 */


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
#include "vpvl2/pmd/Bone.h" /* for Boen::kCategoryNameSize */
#include "vpvl2/pmd/Label.h"

namespace
{

#pragma pack(push, 1)

struct BoneLabel
{
    uint8_t categoryIndex;
    uint16_t boneIndex;
};

#pragma pack(pop)
}

namespace vpvl2
{
namespace pmd
{

Label::Label(const uint8_t *name, const Array<IBone *> &bones, IEncoding *encoding, bool special)
    : m_encodingRef(encoding),
      m_name(0),
      m_index(-1),
      m_special(special)
{
    m_name = m_encodingRef->toString(name, IString::kShiftJIS, 50);
    m_boneRefs.copy(bones);
}

Label::~Label()
{
    delete m_name;
    m_name = 0;
    m_encodingRef = 0;
    m_index = -1;
    m_special = false;
}

bool Label::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size8(ptr, rest, size) || size * sizeof(uint16_t) > rest) {
        return false;
    }
    info.morphLabelsCount = size;
    info.morphLabelsPtr = ptr;
    internal::readBytes(size * sizeof(uint16_t), ptr, rest);
    if (!internal::size8(ptr, rest, size) || size * Bone::kCategoryNameSize > rest) {
        return false;
    }
    info.boneCategoryNamesCount = size;
    info.boneCategoryNamesPtr = ptr;
    internal::readBytes(size * Bone::kCategoryNameSize, ptr, rest);
    if (!internal::size32(ptr, rest, size) || size * sizeof(BoneLabel) > rest) {
        return false;
    }
    info.boneLabelsCount = size;
    info.boneCategoryNamesPtr = ptr;
    internal::readBytes(size * sizeof(BoneLabel), ptr, rest);
    return true;
}

bool Label::loadLabels(const Array<Label *> &labels, const Array<Bone *> &bones, const Array<Morph *> &morphs)
{
    const int nlabels = labels.count();
    const int nbones = bones.count();
    const int nmorphs = morphs.count();
    (void) nbones;
    (void) nmorphs;
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        // FIXME: implement this
        /*
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
        */
        label->m_index = i;
    }
    return true;
}

size_t Label::estimateTotalSize(const Array<Label *> &labels, const Model::DataInfo &info)
{
    const int nlabels = labels.count();
    size_t size = 0;
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        size += label->estimateSize(info);
    }
    return size;
}

void Label::read(const uint8_t * /* data */, const Model::DataInfo &/*info*/, size_t &/*size*/)
{
    // FIXME: implement this
}

size_t Label::estimateSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    return size;
}

void Label::write(uint8_t * /* data */, const Model::DataInfo & /* info */) const
{
}

}
}

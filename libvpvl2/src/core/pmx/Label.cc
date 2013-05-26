/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

struct Pair {
    int id;
    int type;
    vpvl2::IBone *boneRef;
    vpvl2::IMorph *morphRef;
};

}

namespace vpvl2
{
namespace pmx
{

struct Label::PrivateContext {
    PrivateContext(IModel *modelRef)
        : modelRef(modelRef),
          name(0),
          englishName(0),
          index(-1),
          special(false)
    {
    }
    ~PrivateContext() {
        delete name;
        name = 0;
        delete englishName;
        englishName = 0;
        modelRef = 0;
        pairs.releaseAll();
        index = -1;
        special = false;
    }

    IModel *modelRef;
    IString *name;
    IString *englishName;
    PointerArray<Pair> pairs;
    int index;
    bool special;
};

Label::Label(IModel *modelRef)
    : m_context(0)
{
    m_context = new PrivateContext(modelRef);
}

Label::~Label()
{
    delete m_context;
    m_context = 0;
}

bool Label::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    int nlabels, size;
    if (!internal::getTyped<int>(ptr, rest, nlabels)) {
        VPVL2_LOG(WARNING, "Invalid size of PMX labels detected: size=" << nlabels << " rest=" << rest);
        return false;
    }
    info.labelsPtr = ptr;
    for (int i = 0; i < nlabels; i++) {
        uint8_t *namePtr;
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX label name in Japanese detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX label name in English detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        if (!internal::validateSize(ptr, sizeof(uint8_t), rest)) {
            VPVL2_LOG(WARNING, "Invalid PMX label special flag detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << "rest=" << rest);
            return false;
        }
        if (!internal::getTyped<int>(ptr, rest, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX child labels detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << "rest=" << rest);
            return false;
        }
        for (int j = 0; j < size; j++) {
            uint8_t type;
            if (!internal::getTyped<uint8_t>(ptr, rest, type)) {
                VPVL2_LOG(WARNING, "Invalid PMX child label type detected: index=" << i << " childIndex=" << j << " ptr=" << static_cast<const void *>(ptr) << "rest=" << rest);
                return false;
            }
            switch (type) {
            case 0:
                if (!internal::validateSize(ptr, info.boneIndexSize, rest)) {
                    VPVL2_LOG(WARNING, "Invalid PMX bone label detected: index=" << i << " childIndex=" << j << " ptr=" << static_cast<const void *>(ptr) << "rest=" << rest);
                    return false;
                }
                break;
            case 1:
                if (!internal::validateSize(ptr, info.morphIndexSize, rest)) {
                    VPVL2_LOG(WARNING, "Invalid PMX morph label detected: index=" << i << " childIndex=" << j << " ptr=" << static_cast<const void *>(ptr) << "rest=" << rest);
                    return false;
                }
                break;
            default:
                return false;
            }
        }
    }
    info.labelsCount = nlabels;
    return true;
}

bool Label::loadLabels(const Array<Label *> &labels, const Array<Bone *> &bones, const Array<Morph *> &morphs)
{
    const int nlabels = labels.count();
    const int nbones = bones.count();
    const int nmorphs = morphs.count();
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        const Array<Pair *> &pairs = label->m_context->pairs;
        const int npairs = pairs.count();
        for (int j = 0; j < npairs; j++) {
            Pair *pair = pairs[j];
            switch (pair->type) {
            case 0: {
                const int boneIndex = pair->id;
                if (boneIndex >= 0) {
                    if (boneIndex >= nbones) {
                        VPVL2_LOG(WARNING, "Invalid PMX label bone specified: index=" << i << " bone=" << boneIndex);
                        return false;
                    }
                    else {
                        pair->boneRef = bones[boneIndex];
                    }
                }
                break;
            }
            case 1: {
                const int morphIndex = pair->id;
                if (morphIndex >= 0) {
                    if (morphIndex >= nmorphs) {
                        VPVL2_LOG(WARNING, "Invalid PMX label morph specified: index=" << i << " morph=" << morphIndex);
                        return false;
                    }
                    else {
                        pair->morphRef = morphs[morphIndex];
                    }
                }
                break;
            }
            default:
                VPVL2_LOG(WARNING, "Invalid PMX label type specified: index=" << i << " type=" << pair->type);
                return false;
            }
        }
        label->setIndex(i);
    }
    return true;
}

void Label::writeLabels(const Array<Label *> &labels, const Model::DataInfo &info, uint8_t *&data)
{
    const int32_t nlabels = labels.count();
    internal::writeBytes(&nlabels, sizeof(nlabels), data);
    for (int32_t i = 0; i < nlabels; i++) {
        const Label *label = labels[i];
        label->write(data, info);
    }
}

size_t Label::estimateTotalSize(const Array<Label *> &labels, const Model::DataInfo &info)
{
    const int32_t nlabels = labels.count();
    size_t size = 0;
    size += sizeof(nlabels);
    for (int32_t i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        size += label->estimateSize(info);
    }
    return size;
}

void Label::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t rest = SIZE_MAX;
    int32_t nNameSize;
    IEncoding *encoding = info.encoding;
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->name);
    VPVL2_VLOG(3, "PMXLabel: name=" << internal::cstr(m_context->name, "(null)"));
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->englishName);
    VPVL2_VLOG(3, "PMXLabel: englishName=" << internal::cstr(m_context->englishName, "(null)"));
    uint8_t type;
    internal::getTyped<uint8_t>(ptr, rest, type);
    m_context->special = type == 1;
    VPVL2_VLOG(3, "PMXLabel: special=" << m_context->special);
    internal::getTyped<int32_t>(ptr, rest, nNameSize);
    for (int32_t i = 0; i < nNameSize; i++) {
        internal::getTyped<uint8_t>(ptr, rest, type);
        Pair *pair = m_context->pairs.append(new Pair());
        pair->boneRef = 0;
        pair->morphRef = 0;
        pair->type = type;
        switch (type) {
        case 0:
            pair->id = internal::readSignedIndex(ptr, info.boneIndexSize);
            VPVL2_VLOG(3, "PMXLabel: index=" << i << " bone=" << pair->id);
            break;
        case 1:
            pair->id = internal::readSignedIndex(ptr, info.morphIndexSize);
            VPVL2_VLOG(3, "PMXLabel: index=" << i << " morph=" << pair->id);
            break;
        default:
            assert(0);
            return;
        }
    }
    size = ptr - start;
}

void Label::write(uint8_t *&data, const Model::DataInfo &info) const
{
    internal::writeString(m_context->name, info.codec, data);
    internal::writeString(m_context->englishName, info.codec, data);
    int32_t npairs = m_context->pairs.count();
    internal::writeBytes(&m_context->special, sizeof(uint8_t), data);
    internal::writeBytes(&npairs, sizeof(npairs), data);
    for (int32_t i = 0; i < npairs; i++) {
        const Pair *pair = m_context->pairs[i];
        const uint8_t type = pair->type;
        internal::writeBytes(&type, sizeof(type), data);
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
    size += internal::estimateSize(m_context->name, info.codec);
    size += internal::estimateSize(m_context->englishName, info.codec);
    size += sizeof(uint8_t);
    int32_t npairs = m_context->pairs.count();
    size += sizeof(npairs);
    for (int32_t i = 0; i < npairs; i++) {
        const Pair *pair = m_context->pairs[i];
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

const IString *Label::name() const
{
    return m_context->name;
}

const IString *Label::englishName() const
{
    return m_context->englishName;
}

IModel *Label::parentModelRef() const
{
    return m_context->modelRef;
}

int Label::index() const
{
    return m_context->index;
}

bool Label::isSpecial() const
{
    return m_context->special;
}

IBone *Label::bone(int index) const
{
    return internal::checkBound(index, 0, m_context->pairs.count()) ? m_context->pairs[index]->boneRef : 0;
}

IMorph *Label::morph(int index) const
{
    return internal::checkBound(index, 0, m_context->pairs.count()) ? m_context->pairs[index]->morphRef : 0;
}

int Label::count() const
{
    return m_context->pairs.count();
}

void Label::setName(const IString *value)
{
    internal::setString(value, m_context->name);
}

void Label::setEnglishName(const IString *value)
{
    internal::setString(value, m_context->englishName);
}

void Label::setSpecial(bool value)
{
    m_context->special = value;
}

void Label::addBone(IBone *value)
{
    if (value) {
        Pair *pair = m_context->pairs.append(new Pair());
        pair->boneRef = value;
        pair->id = value->index();
        pair->morphRef = 0;
        pair->type = 0;
    }
}

void Label::addMorph(IMorph *value)
{
    if (value) {
        Pair *pair = m_context->pairs.append(new Pair());
        pair->boneRef = 0;
        pair->id = value->index();
        pair->morphRef = value;
        pair->type = 1;
    }
}

void Label::removeBone(IBone *value)
{
    const int npairs = m_context->pairs.count();
    for (int i = 0; i < npairs; i++) {
        Pair *pair = m_context->pairs[i];
        if (pair->boneRef == value) {
            m_context->pairs.remove(pair);
            delete pair;
            break;
        }
    }
}

void Label::removeMorph(IMorph *value)
{
    const int npairs = m_context->pairs.count();
    for (int i = 0; i < npairs; i++) {
        Pair *pair = m_context->pairs[i];
        if (pair->morphRef == value) {
            m_context->pairs.remove(pair);
            delete pair;
            break;
        }
    }
}

void Label::setIndex(int value)
{
    m_context->index = value;
}

} /* namespace pmx */
} /* namespace vpvl2 */


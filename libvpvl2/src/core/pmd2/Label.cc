/**

 Copyright (c) 2010-2014  hkrn

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
#include "vpvl2/internal/ModelHelper.h"
#include "vpvl2/pmd2/Bone.h"
#include "vpvl2/pmd2/Label.h"
#include "vpvl2/pmd2/Morph.h"

namespace
{

using namespace vpvl2::VPVL2_VERSION_NS;

#pragma pack(push, 1)

struct BoneLabel
{
    uint16 boneIndex;
    uint8 categoryIndex;
};

#pragma pack(pop)
}

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
namespace pmd2
{

struct Label::PrivateContext {
    PrivateContext(Model *modelRef, IEncoding *encodingRef, const uint8 *name, Type type)
        : modelRef(modelRef),
          encodingRef(encodingRef),
          namePtr(0),
          englishNamePtr(0),
          type(type),
          index(-1)
    {
        namePtr = encodingRef->toString(name, IString::kShiftJIS, Bone::kCategoryNameSize);
    }
    ~PrivateContext() {
        internal::deleteObject(namePtr);
        internal::deleteObject(englishNamePtr);
        const int nbones = boneRefs.count();
        for (int i = 0; i < nbones; i++) {
            boneRefs[i]->setInternalParentLabelRef(0);
        }
        const int nmorphs = morphRefs.count();
        for (int i = 0; i < nmorphs; i++) {
            morphRefs[i]->setInternalParentLabelRef(0);
        }
        encodingRef = 0;
        index = -1;
    }

    Array<Bone *> boneRefs;
    Array<Morph *> morphRefs;
    Array<int> boneIndices;
    Array<int> morphIndices;
    Model *modelRef;
    IEncoding *encodingRef;
    IString *namePtr;
    IString *englishNamePtr;
    Type type;
    int index;
};

Label::Label(Model *modelRef, IEncoding *encodingRef, const uint8 *name, Type type)
    : m_context(new PrivateContext(modelRef, encodingRef, name, type))
{
}

Label::~Label()
{
    internal::deleteObject(m_context);
}

bool Label::preparse(uint8 *&ptr, vsize &rest, Model::DataInfo &info)
{
    uint8 size;
    if (!internal::getTyped<uint8>(ptr, rest, size) || size * sizeof(uint16) > rest) {
        return false;
    }
    info.morphLabelsCount = size;
    info.morphLabelsPtr = ptr;
    internal::drainBytes(size * sizeof(uint16), ptr, rest);
    if (!internal::getTyped<uint8>(ptr, rest, size) || vsize(size * Bone::kCategoryNameSize) > rest) {
        return false;
    }
    info.boneCategoryNamesCount = size;
    info.boneCategoryNamesPtr = ptr;
    internal::drainBytes(size * Bone::kCategoryNameSize, ptr, rest);
    int size32;
    if (!internal::getTyped<int>(ptr, rest, size32) || size32 * sizeof(BoneLabel) > rest) {
        return false;
    }
    info.boneLabelsCount = size32;
    info.boneLabelsPtr = ptr;
    internal::drainBytes(size32 * sizeof(BoneLabel), ptr, rest);
    return true;
}

bool Label::loadLabels(const Array<Label *> &labels, const Array<Bone *> &bones, const Array<Morph *> &morphs)
{
    const int nlabels = labels.count();
    const int nbones = bones.count();
    const int nmorphs = morphs.count();
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        switch (label->type()) {
        case kSpecialBoneCategoryLabel: {
            if (bones.count() > 0) {
                Bone *boneRef = bones[0];
                label->m_context->boneRefs.append(boneRef);
            }
            break;
        }
        case kBoneCategoryLabel: {
            const Array<int> &indices = label->m_context->boneIndices;
            const int nindices = indices.count();
            for (int j = 0; j < nindices; j++) {
                int index = indices[j];
                if (internal::checkBound(index, 0, nbones)) {
                    Bone *boneRef = bones[index];
                    label->m_context->boneRefs.append(boneRef);
                }
            }
            break;
        }
        case kMorphCategoryLabel: {
            const Array<int> &indices = label->m_context->morphIndices;
            const int nindices = indices.count();
            for (int j = 0; j < nindices; j++) {
                int index = indices[j];
                if (internal::checkBound(index, 0, nmorphs)) {
                    Morph *morphRef = morphs[index];
                    label->m_context->morphRefs.append(morphRef);
                }
            }
            break;
        }
        default:
            break;
        }
        label->setIndex(i);
    }
    return true;
}

void Label::writeLabels(const Array<Label *> &labels, const Model::DataInfo &info, uint8 *&data)
{
    const int nlabels = labels.count();
    int nbones = 0, nmorphs = 0, ncategories = 0;
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        switch (label->type()) {
        case kSpecialBoneCategoryLabel:
        case kBoneCategoryLabel: {
            nbones += label->m_context->boneRefs.count();
            ncategories++;
            break;
        }
        case kMorphCategoryLabel: {
            nmorphs += label->m_context->morphRefs.count();
            break;
        }
        default:
            break;
        }
    }
    internal::writeUnsignedIndex(nmorphs, sizeof(uint8), data);
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        Label::Type type = label->type();
        if (type == kMorphCategoryLabel) {
            label->write(data, info);
        }
    }
    const IEncoding *encodingRef = info.encoding;
    internal::writeUnsignedIndex(ncategories, sizeof(uint8), data);
    uint8 categoryName[internal::kPMDBoneCategoryNameSize], *categoryNamePtr = categoryName;
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        Label::Type type = label->type();
        if (type == kSpecialBoneCategoryLabel || type == kBoneCategoryLabel) {
            internal::writeStringAsByteArray(label->name(IEncoding::kJapanese), encodingRef, IString::kShiftJIS, sizeof(categoryName), categoryNamePtr);
            internal::writeBytes(categoryName, sizeof(categoryName), data);
            categoryNamePtr = categoryName;
        }
    }
    internal::writeBytes(&nbones, sizeof(nbones), data);
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        Label::Type type = label->type();
        if (type == kSpecialBoneCategoryLabel || type == kBoneCategoryLabel) {
            label->write(data, info);
        }
    }
}

void Label::writeEnglishNames(const Array<Label *> &labels, const Model::DataInfo &info, uint8 *&data)
{
    const IEncoding *encodingRef = info.encoding;
    const int nlabels = labels.count();
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        Label::Type type = label->type();
        if (type == kSpecialBoneCategoryLabel || type == kBoneCategoryLabel) {
            internal::writeStringAsByteArray(label->name(IEncoding::kEnglish), encodingRef, IString::kShiftJIS, Bone::kCategoryNameSize, data);
        }
    }
}

vsize Label::estimateTotalSize(const Array<Label *> &labels, const Model::DataInfo &info)
{
    const int nlabels = labels.count();
    vsize size = sizeof(int32) + sizeof(uint8) + sizeof(uint8), ncategories = 0;
    for (int i = 0; i < nlabels; i++) {
        Label *label = labels[i];
        size += label->estimateSize(info);
        Type type = label->type();
        if (type == kSpecialBoneCategoryLabel || type == kBoneCategoryLabel) {
            ncategories++;
        }
    }
    size += ncategories * Bone::kCategoryNameSize;
    return size;
}

Label *Label::selectCategory(const Array<Label *> &labels, const uint8 *data)
{
    BoneLabel label;
    internal::getData(data, label);
    int index = label.categoryIndex;
    if (internal::checkBound(index, 0, labels.count())) {
        Label *label = labels[index];
        return label;
    }
    return 0;
}

void Label::read(const uint8 *data, const Model::DataInfo & /* info */, vsize &size)
{
    switch (m_context->type) {
    case kSpecialBoneCategoryLabel:
    case kBoneCategoryLabel: {
        BoneLabel label;
        internal::getData(data, label);
        m_context->boneIndices.append(label.boneIndex);
        size = sizeof(label);
        break;
    }
    case kMorphCategoryLabel: {
        uint16 morphIndex;
        internal::getData(data, morphIndex);
        m_context->morphIndices.append(morphIndex);
        size = sizeof(morphIndex);
        break;
    }
    default:
        size = 0;
        break;
    }
}

void Label::readEnglishName(const uint8 *data, int index)
{
    if (data && index >= 0) {
        internal::setStringDirect(m_context->encodingRef->toString(data + Bone::kCategoryNameSize * index, IString::kShiftJIS, Bone::kCategoryNameSize), m_context->englishNamePtr);
    }
}

vsize Label::estimateSize(const Model::DataInfo & /* info */) const
{
    vsize size = 0;
    switch (m_context->type) {
    case kSpecialBoneCategoryLabel:
    case kBoneCategoryLabel: {
        size += sizeof(BoneLabel) * m_context->boneRefs.count();
        break;
    }
    case kMorphCategoryLabel: {
        size += sizeof(uint16) * m_context->morphRefs.count();
        break;
    }
    default:
        break;
    }
    return size;
}

void Label::write(uint8 *&data, const Model::DataInfo & /* info */) const
{
    switch (m_context->type) {
    case kSpecialBoneCategoryLabel:
    case kBoneCategoryLabel: {
        const int nindices = m_context->boneRefs.count();
        BoneLabel label;
        for (int i = 0; i < nindices; i++) {
            IBone *bone = m_context->boneRefs[i];
            label.boneIndex = bone->index();
            label.categoryIndex = index() + 1;
            internal::writeBytes(&label, sizeof(label), data);
        }
        break;
    }
    case kMorphCategoryLabel: {
        const int nindices = m_context->morphRefs.count();
        uint16 value;
        for (int i = 0; i < nindices; i++) {
            IMorph *morph = m_context->morphRefs[i];
            value = morph->index();
            internal::writeBytes(&value, sizeof(value), data);
        }
        break;
    }
    default:
        break;
    }
}

const IString *Label::name(IEncoding::LanguageType type) const
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        return m_context->namePtr;
    case IEncoding::kEnglish:
        return m_context->englishNamePtr;
    default:
        return 0;
    }
}

bool Label::isSpecial() const
{
    return m_context->type == kSpecialBoneCategoryLabel;
}

int Label::count() const
{
    switch (m_context->type) {
    case kSpecialBoneCategoryLabel:
    case kBoneCategoryLabel:
        return m_context->boneRefs.count();
    case kMorphCategoryLabel:
        return m_context->morphRefs.count();
    default:
        return 0;
    }
}

IBone *Label::boneRef(int index) const
{
    if ((m_context->type == kSpecialBoneCategoryLabel || m_context->type == kBoneCategoryLabel) &&
            internal::checkBound(index, 0, m_context->boneRefs.count())) {
        IBone *bone = m_context->boneRefs[index];
        return bone;
    }
    return 0;
}

IMorph *Label::morphRef(int index) const
{
    if (m_context->type == kMorphCategoryLabel && internal::checkBound(index, 0, m_context->morphRefs.count())) {
        IMorph *morph = m_context->morphRefs[index];
        return morph;
    }
    return 0;
}

IModel *Label::parentModelRef() const
{
    return m_context->modelRef;
}

int Label::index() const
{
    return m_context->index;
}

Label::Type Label::type() const
{
    return m_context->type;
}

void Label::setIndex(int value)
{
    m_context->index = value;
}

void Label::setName(const IString *value, IEncoding::LanguageType type)
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        if (value && !value->equals(m_context->namePtr)) {
            // VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->namePtr);
        }
        else if (!value && value != m_context->namePtr) {
            // VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::deleteObject(m_context->namePtr);
        }
        break;
    case IEncoding::kEnglish:
        if (value && !value->equals(m_context->englishNamePtr)) {
            // VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->englishNamePtr);
        }
        else if (!value && value != m_context->englishNamePtr) {
            // VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::deleteObject(m_context->englishNamePtr);
        }
        break;
    default:
        break;
    }
}

void Label::setSpecial(bool value)
{
    if (value) {
        m_context->type = kSpecialBoneCategoryLabel;
    }
    else if (m_context->boneRefs.count() > 0) {
        m_context->type = kBoneCategoryLabel;
    }
    else if (m_context->morphRefs.count() > 0) {
        m_context->type = kMorphCategoryLabel;
    }
}

void Label::addBoneRef(IBone *value)
{
    if (value && value->parentModelRef() == parentModelRef()) {
        Bone *boneRef = static_cast<Bone *>(value);
        bool found = false;
        const int nbones = m_context->boneRefs.count();
        for (int i = 0; i < nbones; i++) {
            if (m_context->boneRefs[i] == boneRef) {
                found = true;
            }
        }
        if (!found) {
            boneRef->setInternalParentLabelRef(this);
            m_context->boneRefs.append(boneRef);
        }
    }
}

void Label::addMorphRef(IMorph *value)
{
    if (value && value->parentModelRef() == parentModelRef()) {
        Morph *morphRef = static_cast<Morph *>(value);
        bool found = false;
        const int nmorphs = m_context->morphRefs.count();
        for (int i = 0; i < nmorphs; i++) {
            if (m_context->morphRefs[i] == morphRef) {
                found = true;
            }
        }
        if (!found) {
            morphRef->setInternalParentLabelRef(this);
            m_context->morphRefs.append(morphRef);
        }
    }
}

void Label::removeBoneRef(IBone *value)
{
    if (value && value->parentModelRef() == parentModelRef()) {
        Bone *boneRef = static_cast<Bone *>(value);
        boneRef->setInternalParentLabelRef(0);
        m_context->boneRefs.remove(boneRef);
    }
}

void Label::removeMorphRef(IMorph *value)
{
    if (value && value->parentModelRef() == parentModelRef()) {
        Morph *morphRef = static_cast<Morph *>(value);
        morphRef->setInternalParentLabelRef(0);
        m_context->morphRefs.remove(morphRef);
    }
}

} /* namespace pmd2 */
} /* namespace VPVL2_VERSION_NS */
} /* namespace vpvl2 */

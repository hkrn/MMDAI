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
#include "vpvl2/mvd/ModelKeyframe.h"
#include "vpvl2/mvd/ModelSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct ModelSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int sizeOfIKBones;
    int countOfIKBones;
};

#pragma pack(pop)

class ModelSection::PrivateContext : public BaseSectionContext {
public:
    PrivateContext(IModel *m, size_t a)
        : BaseSectionContext(),
          modelRef(m),
          keyframePtr(0),
          adjustAlignment(a),
          sizeOfIKBones(0)
    {
        keyframes = new KeyframeCollection();
    }
    ~PrivateContext() {
        delete keyframePtr;
        keyframePtr = 0;
        modelRef = 0;
        adjustAlignment = 0;
        sizeOfIKBones = 0;
    }
    IModel *modelRef;
    ModelKeyframe *keyframePtr;
    Array<IBone *> bones;
    size_t adjustAlignment;
    int sizeOfIKBones;
};

ModelSection::ModelSection(IModel *model, NameListSection *nameListSectionRef, size_t align)
    : BaseSection(nameListSectionRef),
      m_contextPtr(0)
{
    m_contextPtr = new PrivateContext(model, align);
}

ModelSection::~ModelSection()
{
    release();
}

bool ModelSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    ModelSectionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    const int countOfIK = header.countOfIKBones;
    if (!internal::validateSize(ptr, sizeof(int), countOfIK, rest)) {
        return false;
    }
    const int sizeOfIK = header.sizeOfIKBones;
    if (!internal::validateSize(ptr, sizeOfIK - sizeof(int) * (countOfIK + 1), rest)) {
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const size_t adjust = info.adjustAlignment;
    const size_t reserved = header.sizeOfKeyframe - ((ModelKeyframe::size() - adjust) + countOfIK);
    for (int i = 0; i < nkeyframes; i++) {
        if (!ModelKeyframe::preparse(ptr, rest, reserved, countOfIK, info)) {
            return false;
        }
    }
    return true;
}

void ModelSection::release()
{
    delete m_contextPtr;
    m_contextPtr = 0;
}

void ModelSection::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    ModelSectionHeader header;
    internal::getData(ptr, header);
    const size_t sizeOfKeyframe = header.sizeOfKeyframe + m_contextPtr->adjustAlignment;
    const int nkeyframes = header.countOfKeyframes;
    const int nBonesOfIK = header.countOfIKBones;
    m_contextPtr->bones.reserve(nBonesOfIK);
    ptr += sizeof(header);
    for (int i = 0; i < nBonesOfIK; i++) {
        int key = *reinterpret_cast<int *>(ptr);
        const IString *s = m_nameListSectionRef->value(key);
        if (m_contextPtr->modelRef && s) {
            IBone *bone = m_contextPtr->modelRef->findBone(s);
            m_contextPtr->bones.add(bone);
        }
        ptr += sizeof(int);
    }
    ptr += header.sizeOfIKBones - sizeof(int) * (nBonesOfIK + 1);
    m_contextPtr->keyframes = new PrivateContext::KeyframeCollection();
    m_contextPtr->keyframes->reserve(nkeyframes);
    for (int i = 0; i < nkeyframes; i++) {
        ModelKeyframe *keyframe = m_contextPtr->keyframePtr = new ModelKeyframe(m_nameListSectionRef, nBonesOfIK);
        keyframe->read(ptr);
        addKeyframe0(keyframe, m_contextPtr->keyframes);
        ptr += sizeOfKeyframe;
    }
    m_contextPtr->keyframePtr = 0;
}

void ModelSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    if (m_contextPtr->modelRef) {
    }
    saveCurrentTimeIndex(timeIndex);
}

void ModelSection::setParentModel(const IModel *parentModelRef)
{
    Array<IBone *> allBones, bonesOfIK;
    if (parentModelRef)
        parentModelRef->getBoneRefs(allBones);
    const int nbones = allBones.count();
    for (int i = 0; i < nbones; i++) {
        IBone *bone = allBones[i];
        if (bone->hasInverseKinematics()) {
            bonesOfIK.add(bone);
        }
    }
    BaseSectionContext::KeyframeCollection *keyframes = m_contextPtr->keyframes;
    const int nkeyframes = keyframes->count();
    for (int i = 0; i < nkeyframes; i++) {
        ModelKeyframe *keyframe = reinterpret_cast<ModelKeyframe *>(keyframes->at(i));
        keyframe->setIKBones(bonesOfIK);
    }
    m_contextPtr->bones.copy(bonesOfIK);
}

void ModelSection::write(uint8_t *data) const
{
    const PrivateContext::KeyframeCollection *keyframes = m_contextPtr->keyframes;
    const Array<IBone *> &bones = m_contextPtr->bones;
    const int nkeyframes = keyframes->count();
    const int nbones = bones.count();
    Motion::SectionTag tag;
    tag.type = Motion::kModelSection;
    tag.minor = 1;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&tag), sizeof(tag), data);
    ModelSectionHeader header;
    header.countOfIKBones = nbones;
    header.countOfKeyframes = nkeyframes;
    header.reserved = 0;
    header.sizeOfIKBones = (nbones + 1) * sizeof(int);
    header.sizeOfKeyframe = ModelKeyframe::size() + sizeof(uint8_t) * nbones - m_contextPtr->adjustAlignment;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&header), sizeof(header), data);
    for (int i = 0; i < nbones; i++) {
        const IBone *bone = bones[i];
        int key = m_nameListSectionRef->key(bone->name());
        internal::writeSignedIndex(key, sizeof(key), data);
    }
    for (int i = 0; i < nkeyframes; i++) {
        const IKeyframe *keyframe = keyframes->at(i);
        keyframe->write(data);
        data += keyframe->estimateSize();
    }
}

size_t ModelSection::estimateSize() const
{
    size_t size = 0;
    size += sizeof(Motion::SectionTag);
    size += sizeof(ModelSectionHeader);
    size += m_contextPtr->bones.count() * sizeof(int);
    const PrivateContext::KeyframeCollection *keyframes = m_contextPtr->keyframes;
    const int nkeyframes = keyframes->count();
    for (int i = 0; i < nkeyframes; i++) {
        const IKeyframe *keyframe = keyframes->at(i);
        size += keyframe->estimateSize();
    }
    return size;
}

size_t ModelSection::countKeyframes() const
{
    return m_contextPtr->keyframes->count();
}

void ModelSection::addKeyframe(IKeyframe *keyframe)
{
    addKeyframe0(keyframe, m_contextPtr->keyframes);
}

void ModelSection::deleteKeyframe(IKeyframe *&keyframe)
{
    m_contextPtr->keyframes->remove(keyframe);
    delete keyframe;
    keyframe = 0;
}

void ModelSection::getKeyframes(const IKeyframe::TimeIndex & /* timeIndex */,
                                const IKeyframe::LayerIndex & /* layerIndex */,
                                Array<IKeyframe *> & /* keyframes */)
{
}

IKeyframe::LayerIndex ModelSection::countLayers(const IString * /* name */) const
{
    return 1;
}

IModelKeyframe *ModelSection::findKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                           const IKeyframe::LayerIndex &layerIndex) const
{
    const PrivateContext::KeyframeCollection *keyframes = m_contextPtr->keyframes;
    const int nkeyframes = keyframes->count();
    for (int i = 0; i < nkeyframes; i++) {
        mvd::ModelKeyframe *keyframe = reinterpret_cast<mvd::ModelKeyframe *>(keyframes->at(i));
        if (keyframe->timeIndex() == timeIndex && keyframe->layerIndex() == layerIndex) {
            return keyframe;
        }
    }
    return 0;
}

IModelKeyframe *ModelSection::findKeyframeAt(int index) const
{
    const PrivateContext::KeyframeCollection *keyframes = m_contextPtr->keyframes;
    if (internal::checkBound(index, 0, keyframes->count())) {
        mvd::ModelKeyframe *keyframe = reinterpret_cast<mvd::ModelKeyframe *>(keyframes->at(index));
        return keyframe;
    }
    return 0;
}

} /* namespace mvd */
} /* namespace vpvl2 */

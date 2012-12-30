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
    PrivateContext(IModel *m, NameListSection *n, size_t a)
        : BaseSectionContext(),
          modelRef(m),
          nameListSectionRef(n),
          keyframePtr(0),
          adjustAlignment(a),
          sizeOfIKBones(0)
    {
    }
    ~PrivateContext() {
        delete keyframePtr;
        keyframePtr = 0;
        modelRef = 0;
        nameListSectionRef = 0;
        adjustAlignment = 0;
        sizeOfIKBones = 0;
    }

    void seek(const IKeyframe::TimeIndex &timeIndex) {
        if (keyframes.count() > 0) {
            int fromIndex, toIndex;
            IKeyframe::TimeIndex currentTimeIndex;
            findKeyframeIndices(timeIndex, currentTimeIndex, fromIndex, toIndex);
            const ModelKeyframe *keyframeFrom = reinterpret_cast<const ModelKeyframe *>(keyframes[fromIndex]),
                    *keyframeTo = reinterpret_cast<const ModelKeyframe *>(keyframes[toIndex]), *keyframe = 0;
            const IKeyframe::TimeIndex &timeIndexFrom = keyframeFrom->timeIndex(), &timeIndexTo = keyframeTo->timeIndex();
            const Color &edgeColorFrom = keyframeFrom->edgeColor(), &edgeColorTo = keyframeTo->edgeColor();
            const Scalar &edgeWidthFrom = keyframeFrom->edgeWidth(), &edgeWidthTo = keyframeTo->edgeWidth();
            if (timeIndexFrom != timeIndexTo && timeIndexFrom < currentTimeIndex) {
                if (timeIndexTo <= currentTimeIndex) {
                    modelRef->setEdgeColor(edgeColorTo);
                    modelRef->setEdgeWidth(edgeWidthTo);
                    keyframe = keyframeTo;
                }
                else {
                    const IKeyframe::SmoothPrecision &w = calculateWeight(currentTimeIndex, timeIndexFrom, timeIndexTo);;
                    modelRef->setEdgeColor(edgeColorFrom.lerp(edgeColorFrom, Scalar(w)));
                    modelRef->setEdgeWidth(Scalar(internal::lerp(edgeWidthFrom, edgeWidthTo, w)));
                    keyframe = keyframeFrom;
                }
            }
            else {
                modelRef->setEdgeColor(edgeColorFrom);
                modelRef->setEdgeWidth(edgeWidthFrom);
                keyframe = keyframeFrom;
            }
            modelRef->setVisible(keyframe->isVisible());
            Hash<btHashInt, IBone *> bones;
            const int nbones = boneIDs.count();
            for (int i = 0; i < nbones; i++) {
                const IString *name = nameListSectionRef->value(boneIDs[i]);
                if (IBone *bone = modelRef->findBone(name)) {
                    bones.insert(i, bone);
                }
            }
            keyframe->mergeIKState(bones);
        }
    }
    void getIKBones(Hash<btHashInt, IBone *> &bonesOfIK) const {
        if (modelRef) {
            Array<IBone *> allBones;
            modelRef->getBoneRefs(allBones);
            const int nbones = allBones.count();
            for (int i = 0; i < nbones; i++) {
                IBone *bone = allBones[i];
                if (bone->hasInverseKinematics()) {
                    bonesOfIK.insert(bone->index(), bone);
                }
            }
        }
    }

    IModel *modelRef;
    NameListSection *nameListSectionRef;
    ModelKeyframe *keyframePtr;
    Array<int> boneIDs;
    size_t adjustAlignment;
    int sizeOfIKBones;
};

ModelSection::ModelSection(const Motion *motionRef, IModel *modelRef, size_t align)
    : BaseSection(motionRef),
      m_context(0)
{
    m_context = new PrivateContext(modelRef, motionRef->nameListSection(), align);
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
    delete m_context;
    m_context = 0;
}

void ModelSection::read(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    ModelSectionHeader header;
    internal::getData(ptr, header);
    const size_t sizeOfKeyframe = header.sizeOfKeyframe + m_context->adjustAlignment;
    const int nkeyframes = header.countOfKeyframes;
    const int nBonesOfIK = header.countOfIKBones;
    m_context->boneIDs.reserve(nBonesOfIK);
    ptr += sizeof(header);
    for (int i = 0; i < nBonesOfIK; i++) {
        int key = *reinterpret_cast<int *>(ptr);
        m_context->boneIDs.add(key);
        ptr += sizeof(int);
    }
    ptr += header.sizeOfIKBones - sizeof(int) * (nBonesOfIK + 1);
    m_context->keyframes.reserve(nkeyframes);
    for (int i = 0; i < nkeyframes; i++) {
        ModelKeyframe *keyframe = m_context->keyframePtr = new ModelKeyframe(m_motionRef, nBonesOfIK);
        keyframe->read(ptr);
        addKeyframe0(keyframe, m_context->keyframes);
        ptr += sizeOfKeyframe;
    }
    m_context->keyframePtr = 0;
}

void ModelSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    saveCurrentTimeIndex(timeIndex);
}

void ModelSection::setParentModel(IModel *modelRef)
{
    m_context->modelRef = modelRef;
}

void ModelSection::write(uint8_t *data) const
{
    const PrivateContext::KeyframeCollection &keyframes = m_context->keyframes;
    const int nkeyframes = keyframes.count();
    Motion::SectionTag tag;
    Hash<btHashInt, IBone *> bones;
    m_context->getIKBones(bones);
    const int nbones = bones.count();
    tag.type = Motion::kModelSection;
    tag.minor = 1;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&tag), sizeof(tag), data);
    ModelSectionHeader header;
    header.countOfIKBones = nbones;
    header.countOfKeyframes = nkeyframes;
    header.reserved = 0;
    header.sizeOfIKBones = (nbones + 1) * sizeof(int);
    header.sizeOfKeyframe = ModelKeyframe::size() + sizeof(uint8_t) * nbones - m_context->adjustAlignment;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&header), sizeof(header), data);
    for (int i = 0; i < nbones; i++) {
        const IBone *const *bone = bones.value(i);
        int key = m_nameListSectionRef->key((*bone)->name());
        internal::writeSignedIndex(key, sizeof(key), data);
    }
    for (int i = 0; i < nkeyframes; i++) {
        ModelKeyframe *keyframe = reinterpret_cast<ModelKeyframe *>(keyframes[i]);
        keyframe->setIKState(bones);
        keyframe->write(data);
        data += keyframe->estimateSize();
    }
}

size_t ModelSection::estimateSize() const
{
    size_t size = 0;
    size += sizeof(Motion::SectionTag);
    size += sizeof(ModelSectionHeader);
    Hash<btHashInt, IBone *> bones;
    m_context->getIKBones(bones);
    size += bones.count() * sizeof(int);
    const PrivateContext::KeyframeCollection &keyframes = m_context->keyframes;
    const int nkeyframes = keyframes.count();
    for (int i = 0; i < nkeyframes; i++) {
        const IKeyframe *keyframe = keyframes[i];
        size += keyframe->estimateSize();
    }
    return size;
}

size_t ModelSection::countKeyframes() const
{
    return m_context->keyframes.count();
}

void ModelSection::addKeyframe(IKeyframe *keyframe)
{
    addKeyframe0(keyframe, m_context->keyframes);
}

void ModelSection::deleteKeyframe(IKeyframe *&keyframe)
{
    m_context->keyframes.remove(keyframe);
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
    const PrivateContext::KeyframeCollection &keyframes = m_context->keyframes;
    const int nkeyframes = keyframes.count();
    for (int i = 0; i < nkeyframes; i++) {
        ModelKeyframe *keyframe = reinterpret_cast<ModelKeyframe *>(keyframes[i]);
        if (keyframe->timeIndex() == timeIndex && keyframe->layerIndex() == layerIndex) {
            return keyframe;
        }
    }
    return 0;
}

IModelKeyframe *ModelSection::findKeyframeAt(int index) const
{
    const PrivateContext::KeyframeCollection &keyframes = m_context->keyframes;
    if (internal::checkBound(index, 0, keyframes.count())) {
        ModelKeyframe *keyframe = reinterpret_cast<ModelKeyframe *>(keyframes[index]);
        return keyframe;
    }
    return 0;
}

} /* namespace mvd */
} /* namespace vpvl2 */

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
#include "vpvl2/internal/MotionHelper.h"

#include "vpvl2/mvd/NameListSection.h"
#include "vpvl2/mvd/ModelKeyframe.h"
#include "vpvl2/mvd/ModelSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct ModelSectionHeader {
    int32 reserved;
    int32 sizeOfKeyframe;
    int32 countOfKeyframes;
    int32 sizeOfIKBones;
    int32 countOfIKBones;
};

#pragma pack(pop)

class ModelSection::PrivateContext : public BaseAnimationTrack {
public:
    PrivateContext(IModel *m, NameListSection *n, vsize a)
        : BaseAnimationTrack(),
          modelRef(m),
          nameListSectionRef(n),
          adjustAlignment(a),
          sizeOfIKBones(0)
    {
    }
    ~PrivateContext() {
        modelRef = 0;
        nameListSectionRef = 0;
        adjustAlignment = 0;
        sizeOfIKBones = 0;
    }

    void seek(const IKeyframe::TimeIndex &timeIndex) {
        if (keyframes.count() > 0) {
            int fromIndex, toIndex;
            IKeyframe::TimeIndex currentTimeIndex;
            internal::MotionHelper::findKeyframeIndices(timeIndex, currentTimeIndex, m_lastIndex, fromIndex, toIndex, keyframes);
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
                    const IKeyframe::SmoothPrecision &w = internal::MotionHelper::calculateWeight(currentTimeIndex, timeIndexFrom, timeIndexTo);;
                    const Vector3 &ec = edgeColorFrom.lerp(edgeColorTo, Scalar(w));
                    modelRef->setEdgeColor(Color(ec.x(), ec.y(), ec.z(), edgeColorFrom.w() + edgeColorTo.w() * w));
                    modelRef->setEdgeWidth(Scalar(internal::MotionHelper::lerp(edgeWidthFrom, edgeWidthTo, w)));
                    keyframe = keyframeFrom;
                }
            }
            else {
                modelRef->setEdgeColor(edgeColorFrom);
                modelRef->setEdgeWidth(edgeWidthFrom);
                keyframe = keyframeFrom;
            }
            modelRef->setVisible(keyframe->isVisible());
            Hash<HashInt, IBone *> bones;
            const int nbones = boneNameIndices.count();
            for (int i = 0; i < nbones; i++) {
                const IString *name = nameListSectionRef->value(boneNameIndices[i]);
                if (IBone *bone = modelRef->findBoneRef(name)) {
                    bones.insert(i, bone);
                }
            }
            keyframe->updateInverseKinematicsState();
        }
    }
    void getIKBoneRefs(Hash<HashInt, IBone *> &value) const {
        if (modelRef) {
            Array<IBone *> allBones;
            modelRef->getBoneRefs(allBones);
            const int nbones = allBones.count();
            for (int i = 0; i < nbones; i++) {
                IBone *bone = allBones[i];
                if (bone->hasInverseKinematics()) {
                    value.insert(bone->index(), bone);
                }
            }
        }
    }

    IModel *modelRef;
    NameListSection *nameListSectionRef;
    Array<int> boneNameIndices;
    vsize adjustAlignment;
    int sizeOfIKBones;
};

ModelSection::ModelSection(const Motion *motionRef, IModel *modelRef, vsize align)
    : BaseSection(motionRef),
      m_context(new PrivateContext(modelRef, motionRef->nameListSection(), align))
{
}

ModelSection::~ModelSection()
{
    release();
}

bool ModelSection::preparse(uint8 *&ptr, vsize &rest, Motion::DataInfo &info)
{
    ModelSectionHeader header;
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVDModelSection header detected: " << rest);
        return false;
    }
    internal::getData(ptr - sizeof(header), header);
    const int countOfIK = header.countOfIKBones;
    if (!internal::validateSize(ptr, sizeof(int32), countOfIK, rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVDModelSection header (IK count) detected: size=" << countOfIK << "rest=" << rest);
        return false;
    }
    const int sizeOfIK = header.sizeOfIKBones;
    if (!internal::validateSize(ptr, sizeOfIK - sizeof(int32) * (countOfIK + 1), rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVDModelSection header (IK size) detected: size=" << sizeOfIK << "rest=" << rest);
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const vsize adjust = info.adjustAlignment;
    const vsize reserved = header.sizeOfKeyframe - ((ModelKeyframe::size() - adjust) + countOfIK);
    VPVL2_VLOG(2, "MVDModelSection(Header): nkeyframes=" << nkeyframes);
    VPVL2_VLOG(2, "MVDModelSection(Header): sizeofKeyframe=" << header.sizeOfKeyframe);
    VPVL2_VLOG(2, "MVDModelSection(Header): nIKBones=" << header.countOfIKBones);
    VPVL2_VLOG(2, "MVDModelSection(Header): sizeofIKBones=" << header.sizeOfIKBones);
    VPVL2_VLOG(2, "MVDModelSection(Header): adjust=" << adjust);
    VPVL2_VLOG(2, "MVDModelSection(Header): reserved=" << reserved);
    for (int i = 0; i < nkeyframes; i++) {
        if (!ModelKeyframe::preparse(ptr, rest, reserved, countOfIK, info)) {
            VPVL2_LOG(WARNING, "Invalid size of MVDModelSection key detected: index=" << i << " rest=" << rest);
            return false;
        }
    }
    return true;
}

void ModelSection::release()
{
    internal::deleteObject(m_context);
}

void ModelSection::read(const uint8 *data)
{
    uint8 *ptr = const_cast<uint8 *>(data);
    ModelSectionHeader header;
    internal::getData(ptr, header);
    const vsize sizeOfKeyframe = header.sizeOfKeyframe + m_context->adjustAlignment;
    const int nkeyframes = header.countOfKeyframes;
    const int nBonesOfIK = header.countOfIKBones;
    m_context->boneNameIndices.reserve(nBonesOfIK);
    ptr += sizeof(header);
    for (int i = 0; i < nBonesOfIK; i++) {
        int32 key = *reinterpret_cast<const int32 *>(ptr);
        m_context->boneNameIndices.append(key);
        ptr += sizeof(int);
    }
    ptr += header.sizeOfIKBones - sizeof(int32) * (nBonesOfIK + 1);
    m_context->keyframes.reserve(nkeyframes);
    for (int i = 0; i < nkeyframes; i++) {
        ModelKeyframe *keyframe = m_context->keyframes.append(new ModelKeyframe(this));
        keyframe->read(ptr);
        setDuration(keyframe);
        ptr += sizeOfKeyframe;
    }
}

void ModelSection::seek(const IKeyframe::TimeIndex &timeIndex)
{
    saveCurrentTimeIndex(timeIndex);
}

void ModelSection::setParentModel(IModel *modelRef)
{
    m_context->modelRef = modelRef;
}

void ModelSection::write(uint8 *data) const
{
    const PrivateContext::KeyframeCollection &keyframes = m_context->keyframes;
    const int nkeyframes = keyframes.count();
    Motion::SectionTag tag;
    Hash<HashInt, IBone *> bones;
    m_context->getIKBoneRefs(bones);
    const int nbones = bones.count();
    tag.type = Motion::kModelSection;
    tag.minor = 1;
    internal::writeBytes(&tag, sizeof(tag), data);
    ModelSectionHeader header;
    header.countOfIKBones = nbones;
    header.countOfKeyframes = nkeyframes;
    header.reserved = 0;
    header.sizeOfIKBones = (nbones + 1) * sizeof(int32);
    header.sizeOfKeyframe = ModelKeyframe::size() + sizeof(uint8) * nbones - m_context->adjustAlignment;
    internal::writeBytes(&header, sizeof(header), data);
    for (int i = 0; i < nbones; i++) {
        const IBone *const *bone = bones.value(i);
        int key = m_nameListSectionRef->key((*bone)->name(IEncoding::kDefaultLanguage));
        internal::writeSignedIndex(key, sizeof(key), data);
    }
    for (int i = 0; i < nkeyframes; i++) {
        ModelKeyframe *keyframe = reinterpret_cast<ModelKeyframe *>(keyframes[i]);
        keyframe->setInverseKinematicsState(bones);
        keyframe->write(data);
        data += keyframe->estimateSize();
    }
}

vsize ModelSection::estimateSize() const
{
    vsize size = 0;
    size += sizeof(Motion::SectionTag);
    size += sizeof(ModelSectionHeader);
    Hash<HashInt, IBone *> bones;
    m_context->getIKBoneRefs(bones);
    size += bones.count() * sizeof(int);
    const PrivateContext::KeyframeCollection &keyframes = m_context->keyframes;
    const int nkeyframes = keyframes.count();
    for (int i = 0; i < nkeyframes; i++) {
        const IKeyframe *keyframe = keyframes[i];
        size += keyframe->estimateSize();
    }
    return size;
}

vsize ModelSection::countKeyframes() const
{
    return m_context->keyframes.count();
}

void ModelSection::update()
{
    IKeyframe::TimeIndex durationTimeIndex = 0;
    const int nkeyframes = m_context->keyframes.count();
    for (int i = 0; i < nkeyframes; i++) {
        IKeyframe *keyframe = m_context->keyframes[i];
        btSetMax(durationTimeIndex, keyframe->timeIndex());
    }
    m_durationTimeIndex = durationTimeIndex;
}

void ModelSection::addKeyframe(IKeyframe *keyframe)
{
    m_context->keyframes.append(keyframe);
    setDuration(keyframe);
}

void ModelSection::removeKeyframe(IKeyframe *keyframe)
{
    m_context->keyframes.remove(keyframe);
}

void ModelSection::deleteKeyframe(IKeyframe *&keyframe)
{
    removeKeyframe(keyframe);
    internal::deleteObject(keyframe);
}

void ModelSection::getKeyframes(const IKeyframe::TimeIndex & /* timeIndex */,
                                const IKeyframe::LayerIndex & /* layerIndex */,
                                Array<IKeyframe *> & /* keyframes */) const
{
}

void ModelSection::getAllKeyframes(Array<IKeyframe *> &keyframes) const
{
    keyframes.copy(m_context->keyframes);
}

void ModelSection::setAllKeyframes(const Array<IKeyframe *> &value)
{
    IModel *modelRef = m_context->modelRef;
    NameListSection *nameListSectionRef = m_context->nameListSectionRef;
    vsize adjustAlignment = m_context->adjustAlignment;
    release();
    m_context = new PrivateContext(modelRef, nameListSectionRef, adjustAlignment);
    const int nkeyframes = value.count();
    for (int i = 0; i < nkeyframes; i++) {
        IKeyframe *keyframe = value[i];
        if (keyframe && keyframe->type() == IKeyframe::kModelKeyframe) {
            addKeyframe(keyframe);
        }
    }
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

IBone *ModelSection::findInverseKinematicsBoneAt(int value) const
{
    if (internal::checkBound(value, 0, m_context->boneNameIndices.count())) {
        int boneNameIndex = m_context->boneNameIndices[value];
        const IString *name = m_nameListSectionRef->value(boneNameIndex);
        return m_context->modelRef->findBoneRef(name);
    }
    return 0;
}

int ModelSection::countInverseKinematicsBones() const
{
    return m_context->boneNameIndices.count();
}

} /* namespace mvd */
} /* namespace vpvl2 */

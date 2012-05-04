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
#include "vpvl2/pmd/Bone.h"
#include "vpvl2/pmd/Label.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmd/Morph.h"

namespace vpvl2
{
namespace pmd
{

Model::Model(IEncoding *encoding)
    : m_encoding(encoding),
      m_name(0),
      m_englishName(0),
      m_comment(0),
      m_englishComment(0)
{
}

Model::~Model()
{
    m_bones.releaseAll();
    m_morphs.releaseAll();
    m_encoding = 0;
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    delete m_comment;
    m_comment = 0;
    delete m_englishComment;
    m_englishComment = 0;
}

bool Model::load(const uint8_t *data, size_t size)
{
    bool ret = m_model.load(data, size);
    if (ret) {
        /* convert bones (vpvl::Bone => vpvl2::IBone) */
        const vpvl::BoneList &bones = m_model.bones();
        const int nbones = bones.count();
        Hash<HashPtr, Bone *> b2b;
        for (int i = 0; i < nbones; i++) {
            vpvl::Bone *b = bones[i];
            Bone *bone = new Bone(b, m_encoding);
            bone->setParentBone(b);
            bone->setChildBone(b);
            m_bones.add(bone);
            m_name2bones.insert(bone->name()->toHashString(), bone);
            HashPtr key(b);
            b2b.insert(key, bone);
        }
        /* set IK */
        const vpvl::IKList &IKs = m_model.IKs();
        const int nIKs = IKs.count();
        for (int i = 0; i < nIKs; i++) {
            vpvl::IK *ik = IKs[i];
            Bone **valuePtr = const_cast<Bone **>(b2b.find(ik->destinationBone()));
            if (valuePtr) {
                Bone *value = *valuePtr;
                value->setIK(ik, b2b);
            }
        }
        /* build first bone label (this is special label) */
        Array<IBone *> bones2, firstBone;
        firstBone.add(m_bones[0]);
        Label *label = new Label(reinterpret_cast<const uint8_t *>("Root"), firstBone, m_encoding, true);
        m_labels.add(label);
        /* other bone labels */
        const vpvl::Array<vpvl::BoneList *> &bonesForUI = m_model.bonesForUI();
        const vpvl::Array<uint8_t *> &categories = m_model.boneCategoryNames();
        const int ncategories = categories.count();
        for (int i = 0; i < ncategories; i++) {
            const vpvl::BoneList *bonesInCategory = bonesForUI[i];
            const int nBonesInCategory = bonesInCategory->count();
            const uint8_t *name = categories[i];
            bones2.clear();
            for (int j = 0; j < nBonesInCategory; j++) {
                vpvl::Bone *bone = bonesInCategory->at(j);
                Bone **valuePtr = const_cast<Bone **>(b2b.find(bone));
                if (valuePtr) {
                    Bone *value = *valuePtr;
                    bones2.add(value);
                }
            }
            label = new Label(name, bones2, m_encoding, false);
            m_labels.add(label);
        }
        /* convert morphs (vpvl::Face => vpvl2::IMorph) */
        const vpvl::FaceList &morphs = m_model.faces();
        const int nmorphs = morphs.count();
        for (int i = 0; i < nmorphs; i++) {
            vpvl::Face *face = morphs[i];
            if (face->type() != vpvl::Face::kBase) {
                Morph *morph = new Morph(face, m_encoding);
                morph->setIndex(i);
                m_morphs.add(morph);
                m_name2morphs.insert(morph->name()->toHashString(), morph);
            }
        }
        delete m_name;
        m_name = m_encoding->toString(m_model.name(), IString::kShiftJIS, vpvl::PMDModel::kNameSize);
        delete m_englishName;
        m_englishName = m_encoding->toString(m_model.englishName(), IString::kShiftJIS, vpvl::PMDModel::kNameSize);
        delete m_comment;
        m_comment = m_encoding->toString(m_model.comment(), IString::kShiftJIS, vpvl::PMDModel::kCommentSize);
        delete m_englishComment;
        m_englishComment = m_encoding->toString(m_model.englishComment(), IString::kShiftJIS, vpvl::PMDModel::kCommentSize);
        m_model.setVisible(true);
    }
    return ret;
}

void Model::save(uint8_t *data) const
{
    m_model.save(data);
}

size_t Model::estimateSize() const
{
    return m_model.estimateSize();
}

void Model::resetVertices()
{
}

void Model::performUpdate()
{
    m_model.updateImmediate();
}

void Model::joinWorld(btDiscreteDynamicsWorld *world)
{
    m_model.joinWorld(world);
}

void Model::leaveWorld(btDiscreteDynamicsWorld *world)
{
    m_model.leaveWorld(world);
}

IBone *Model::findBone(const IString *value) const
{
    IBone **bone = const_cast<IBone **>(m_name2bones.find(value->toHashString()));
    return bone ? *bone : 0;
}

IMorph *Model::findMorph(const IString *value) const
{
    IMorph **morph = const_cast<IMorph **>(m_name2morphs.find(value->toHashString()));
    return morph ? *morph : 0;
}

int Model::count(Object value) const
{
    switch (value) {
    case kBone:
        return m_model.bones().count();
    case kIK:
        return m_model.IKs().count();
    case kIndex:
        return m_model.indices().count();
    case kJoint:
        return m_model.constraints().count();
    case kMaterial:
        return m_model.materials().count();
    case kMorph:
        return m_model.faces().count();
    case kRigidBody:
        return m_model.rigidBodies().count();
    case kVertex:
        return m_model.vertices().count();
    default:
        return 0;
    }
}

void Model::getBoundingBox(Vector3 &min, Vector3 &max) const
{
    min.setZero();
    max.setZero();
    const uint8_t *verticesPtr = static_cast<const uint8_t *>(m_model.verticesPointer());
    const size_t stride = m_model.strideSize(vpvl::PMDModel::kVerticesStride);
    const int nvertices = m_model.vertices().count();
    size_t offset = m_model.strideOffset(vpvl::PMDModel::kVerticesStride);
    for (int i = 0; i < nvertices; i++) {
        const Vector3 &position = *reinterpret_cast<const Vector3 *>(verticesPtr + offset);
        if (position.x() < min.x() && position.y() < min.y() && position.z() < min.z())
            min = position;
        if (position.x() > max.x() && position.y() > max.y() && position.z() > max.z())
            max = position;
        offset += stride;
    }
}

void Model::getBoundingSphere(Vector3 &center, Scalar &radius) const
{
    center.setZero();
    radius = 0;
    IBone *bone = findBone(m_encoding->stringConstant(IEncoding::kCenter));
    if (bone) {
        const Vector3 &centerPosition = bone->localTransform().getOrigin();
        const uint8_t *verticesPtr = static_cast<const uint8_t *>(m_model.verticesPointer());
        const size_t stride = m_model.strideSize(vpvl::PMDModel::kVerticesStride);
        const int nvertices = m_model.vertices().count();
        size_t offset = m_model.strideOffset(vpvl::PMDModel::kVerticesStride);
        for (int i = 0; i < nvertices; i++) {
            const Vector3 &position = *reinterpret_cast<const Vector3 *>(verticesPtr + offset);
            btSetMax(radius, centerPosition.distance2(position));
            offset += stride;
        }
        radius = btSqrt(radius);
    }
    else {
        Vector3 min, max;
        getBoundingBox(min, max);
        center = (min + max) / 2.0;
        radius = center.length();
    }
}

void Model::setName(const IString *value)
{
    internal::setString(value, m_name);
    uint8_t *bytes = m_encoding->toByteArray(value, IString::kShiftJIS);
    m_model.setName(bytes);
    m_encoding->disposeByteArray(bytes);
}

void Model::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
    uint8_t *bytes = m_encoding->toByteArray(value, IString::kShiftJIS);
    m_model.setEnglishName(bytes);
    m_encoding->disposeByteArray(bytes);
}

void Model::setComment(const IString *value)
{
    internal::setString(value, m_comment);
    uint8_t *bytes = m_encoding->toByteArray(value, IString::kShiftJIS);
    m_model.setComment(bytes);
    m_encoding->disposeByteArray(bytes);
}

void Model::setEnglishComment(const IString *value)
{
    internal::setString(value, m_englishComment);
    uint8_t *bytes = m_encoding->toByteArray(value, IString::kShiftJIS);
    m_model.setEnglishComment(bytes);
    m_encoding->disposeByteArray(bytes);
}

void Model::setPosition(const Vector3 &value)
{
    m_model.setPositionOffset(value);
}

void Model::setRotation(const Quaternion &value)
{
    m_model.setRotationOffset(value);
}

void Model::setOpacity(const Scalar &value)
{
    m_opacity = value;
}

void Model::setScaleFactor(const Scalar &value)
{
    m_scaleFactor = value;
}

}
}

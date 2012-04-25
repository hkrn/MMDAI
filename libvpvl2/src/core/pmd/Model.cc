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
#include "vpvl2/pmd/Bone.h"
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
        const vpvl::Array<vpvl::Bone *> &bones = m_model.bones();
        const int nbones = bones.count();
        for (int i = 0; i < nbones; i++) {
            vpvl::Bone *b = bones[i];
            Bone *bone = new Bone(b, m_encoding);
            bone->setParentBone(b);
            bone->setChildBone(b);
            m_bones.add(bone);
            m_name2bones.insert(bone->name()->toHashString(), bone);
        }
        const vpvl::Array<vpvl::Face *> &morphs = m_model.faces();
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
        m_comment = m_encoding->toString(m_model.englishName(), IString::kShiftJIS, vpvl::PMDModel::kCommentSize);
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

}
}

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

namespace vpvl2
{
namespace pmd
{

Bone::Bone(vpvl::Bone *bone, IEncoding *encoding)
    : m_encoding(encoding),
      m_name(0),
      m_parentBone(0),
      m_childBone(0),
      m_bone(bone)
{
    m_name = m_encoding->toString(m_bone->name(), IString::kShiftJIS, vpvl::Bone::kNameSize);
}

Bone::~Bone()
{
    delete m_name;
    m_name = 0;
    delete m_parentBone;
    m_parentBone = 0;
    delete m_childBone;
    m_childBone = 0;
    m_encoding = 0;
    m_bone = 0;
}

const IString *Bone::name() const
{
    return m_name;
}

int Bone::index() const
{
    return m_bone->id();
}

const Transform &Bone::localTransform() const
{
    return m_bone->localTransform();
}

const Vector3 &Bone::origin() const
{
    return m_bone->originPosition();
}

const Vector3 Bone::destinationOrigin() const
{
    return m_bone->child()->localTransform().getOrigin();
}

const Vector3 &Bone::position() const
{
    return m_bone->position();
}

const Quaternion &Bone::rotation() const
{
    return m_bone->rotation();
}

void Bone::setPosition(const Vector3 &value)
{
    m_bone->setPosition(value);
}

void Bone::setRotation(const Quaternion &value)
{
    m_bone->setRotation(value);
}

bool Bone::isMovable() const
{
    return m_bone->isMovable();
}

bool Bone::isRotateable() const
{
    return m_bone->isRotateable();
}

bool Bone::isVisible() const
{
    return m_bone->isVisible();
}

bool Bone::hasFixedAxes() const
{
    return false;
}

bool Bone::hasLocalAxes() const
{
    // const QString &name = toQString(bone);
    // return name.indexOf("指") != -1 || name.endsWith("腕") || name.endsWith("ひじ") || name.endsWith("手首");
    return false;
}

void Bone::getFixedAxes(Matrix3x3 &value) const
{
    if (hasFixedAxes()) {
    }
    else {
        value.setIdentity();
    }
}

void Bone::getLocalAxes(Matrix3x3 &value) const
{
    if (hasLocalAxes()) {
        const Vector3 &axisX = (m_childBone->origin() - m_bone->originPosition()).normalized();
        Vector3 tmp1 = axisX;
        //const QString &name = toQString(bone);
        //name.startsWith("左") ? tmp1.setY(-axisX.y()) : tmp1.setX(-axisX.x());
        const Vector3 &axisZ = axisX.cross(tmp1).normalized();
        Vector3 tmp2 = axisX;
        tmp2.setZ(-axisZ.z());
        const Vector3 &axisY = tmp2.cross(-axisX).normalized();
        value[0] = axisX;
        value[1] = axisY;
        value[2] = axisZ;
    }
    else {
        value.setIdentity();
    }
}

void Bone::setParentBone(vpvl::Bone *value)
{
    if (value)
        m_parentBone = new Bone(const_cast<vpvl::Bone *>(value->parent()), m_encoding);
}

void Bone::setChildBone(vpvl::Bone *value)
{
    if (value)
        m_childBone = new Bone(const_cast<vpvl::Bone *>(value->child()), m_encoding);
}

}
}

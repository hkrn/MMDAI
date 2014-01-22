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

#include "RigidBodyRefObject.h"
#include "ModelProxy.h"
#include "Util.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

RigidBodyRefObject::RigidBodyRefObject(ModelProxy *parentModelRef,
                                       BoneRefObject *parentBoneRef,
                                       vpvl2::IRigidBody *rigidBodyRef,
                                       const QUuid &uuid)
    : m_parentModelRef(parentModelRef),
      m_parentBoneRef(parentBoneRef),
      m_rigidBodyRef(rigidBodyRef),
      m_uuid(uuid)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_rigidBodyRef);
    Q_ASSERT(!m_uuid.isNull());
}

RigidBodyRefObject::~RigidBodyRefObject()
{
    m_parentModelRef = 0;
    m_rigidBodyRef = 0;
}

IRigidBody *RigidBodyRefObject::data() const
{
    Q_ASSERT(m_rigidBodyRef);
    return m_rigidBodyRef;
}

ModelProxy *RigidBodyRefObject::parentModel() const
{
    return m_parentModelRef;
}

BoneRefObject *RigidBodyRefObject::parentBone() const
{
    return m_parentBoneRef;
}

QUuid RigidBodyRefObject::uuid() const
{
    return m_uuid;
}

QString RigidBodyRefObject::name() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_rigidBodyRef);
    IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(m_parentModelRef->language());
    return Util::toQString(m_rigidBodyRef->name(language));
}

void RigidBodyRefObject::setName(const QString &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_rigidBodyRef);
    if (name() != value) {
        IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(m_parentModelRef->language());
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_rigidBodyRef->setName(s.data(), language);
        emit nameChanged();
    }
}

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

#include "LabelRefObject.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>
#include <QtCore>

#include "BoneRefObject.h"
#include "ModelProxy.h"
#include "MorphRefObject.h"
#include "Util.h"

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

LabelRefObject::LabelRefObject(ModelProxy *modelRef, ILabel *labelRef)
    : QObject(modelRef),
      m_parentModelRef(modelRef),
      m_labelRef(labelRef)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_labelRef);
    connect(m_parentModelRef, &ModelProxy::languageChanged, this, &LabelRefObject::nameChanged);
}

LabelRefObject::~LabelRefObject()
{
    m_labelRef = 0;
}

void LabelRefObject::addBone(BoneRefObject *object)
{
    Q_ASSERT(object);
    m_bones.append(object);
}

void LabelRefObject::addMorph(MorphRefObject *object)
{
    Q_ASSERT(object);
    m_morphs.append(object);
}

void LabelRefObject::addObject(QObject *value)
{
    if (BoneRefObject *bone = qobject_cast<BoneRefObject *>(value)) {
        if (!m_bones.contains(bone)) {
            m_bones.append(bone);
            m_labelRef->addBoneRef(bone->data());
            m_parentModelRef->markDirty();
            emit bonesChanged();
        }
    }
    else if (MorphRefObject *morph = qobject_cast<MorphRefObject *>(value)) {
        if (!m_morphs.contains(morph)) {
            m_morphs.append(morph);
            m_labelRef->addMorphRef(morph->data());
            m_parentModelRef->markDirty();
            emit morphsChanged();
        }
    }
}

void LabelRefObject::removeObject(QObject *value)
{
    if (BoneRefObject *bone = qobject_cast<BoneRefObject *>(value)) {
        if (m_bones.removeOne(bone)) {
            m_labelRef->removeBoneRef(bone->data());
            m_parentModelRef->markDirty();
            emit bonesChanged();
        }
    }
    else if (MorphRefObject *morph = qobject_cast<MorphRefObject *>(value)) {
        if (m_morphs.removeOne(morph)) {
            m_labelRef->removeMorphRef(morph->data());
            m_parentModelRef->markDirty();
            emit morphsChanged();
        }
    }
}

ModelProxy *LabelRefObject::parentModel() const
{
    return m_parentModelRef;
}

QString LabelRefObject::name() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_labelRef);
    if (m_labelRef->isSpecial()) {
        if (BoneRefObject *bone = m_parentModelRef->resolveBoneRef(m_labelRef->boneRef(0))) {
            return bone->name();
        }
    }
    IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(m_parentModelRef->language());
    const IString *name = m_labelRef->name(language);
    return Util::toQString((name && name->size() > 0) ? name : m_labelRef->name(IEncoding::kDefaultLanguage));
}

void LabelRefObject::setName(const QString &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_labelRef);
    if (!m_labelRef->isSpecial() && name() != value) {
        IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(m_parentModelRef->language());
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_labelRef->setName(s.data(), language);
        m_parentModelRef->markDirty();
        emit nameChanged();
    }
}

int LabelRefObject::index() const
{
    Q_ASSERT(m_labelRef);
    return m_labelRef->index();
}

QQmlListProperty<BoneRefObject> LabelRefObject::bones()
{
    return QQmlListProperty<BoneRefObject>(this, m_bones);
}

QQmlListProperty<MorphRefObject> LabelRefObject::morphs()
{
    return QQmlListProperty<MorphRefObject>(this, m_morphs);
}

bool LabelRefObject::isSpecial() const
{
    Q_ASSERT(m_labelRef);
    return m_labelRef->isSpecial();
}

void LabelRefObject::setSpecial(bool value)
{
    Q_ASSERT(m_labelRef);
    if (isSpecial() != value) {
        m_labelRef->setSpecial(value);
        m_parentModelRef->markDirty();
        emit specialChanged();
    }
}

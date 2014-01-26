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

#include "ModelProxy.h"
#include "MorphRefObject.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>
#include <QtCore>

#include "LabelRefObject.h"
#include "Util.h"

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

MorphRefObject::MorphRefObject(ModelProxy *modelRef, LabelRefObject *labelRef, IMorph *morphRef, const QUuid &uuid)
    : QObject(labelRef),
      m_parentModelRef(modelRef),
      m_parentLabelRef(labelRef),
      m_morphRef(morphRef),
      m_uuid(uuid),
      m_originWeight(0)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_morphRef);
    Q_ASSERT(!m_uuid.isNull());
    connect(m_parentModelRef, &ModelProxy::languageChanged, this, &MorphRefObject::nameChanged);
    connect(this, &MorphRefObject::morphDidSync, this, &MorphRefObject::weightChanged);
}

MorphRefObject::~MorphRefObject()
{
    m_parentLabelRef = 0;
    m_morphRef = 0;
    m_originWeight = 0;
}

void MorphRefObject::setOriginWeight(const qreal &value)
{
    if (!qFuzzyCompare(value, m_originWeight)) {
        m_originWeight = value;
        emit originWeightChanged();
    }
}

vpvl2::IMorph *MorphRefObject::data() const
{
    return m_morphRef;
}

ModelProxy *MorphRefObject::parentModel() const
{
    return m_parentModelRef;
}

LabelRefObject *MorphRefObject::parentLabel() const
{
    return m_parentLabelRef;
}

QUuid MorphRefObject::uuid() const
{
    Q_ASSERT(!m_uuid.isNull());
    return m_uuid;
}

QString MorphRefObject::name() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_morphRef);
    ModelProxy *parentModel = m_parentModelRef;
    IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(parentModel->language());
    const IString *name = m_morphRef->name(language);
    return Util::toQString((name && name->size() > 0) ? name : m_morphRef->name(IEncoding::kDefaultLanguage));
}

void MorphRefObject::setName(const QString &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_morphRef);
    if (name() != value) {
        ModelProxy *parentModel = m_parentModelRef;
        IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(parentModel->language());
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_morphRef->setName(s.data(), language);
        parentModel->markDirty();
        emit nameChanged();
    }
}

MorphRefObject::Category MorphRefObject::category() const
{
    Q_ASSERT(m_morphRef);
    return static_cast<Category>(m_morphRef->category());
}

void MorphRefObject::setCategory(const Category &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_morphRef);
    if (category() != value) {
        m_morphRef->setCategory(static_cast<IMorph::Category>(value));
        m_parentModelRef->markDirty();
    }
}

MorphRefObject::Type MorphRefObject::type() const
{
    Q_ASSERT(m_morphRef);
    return static_cast<Type>(m_morphRef->type());
}

void MorphRefObject::setType(const Type &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_morphRef);
    if (type() != value) {
        m_morphRef->setType(static_cast<IMorph::Type>(value));
        m_parentModelRef->markDirty();
    }
}

int MorphRefObject::index() const
{
    Q_ASSERT(m_morphRef);
    return m_morphRef->index();
}

qreal MorphRefObject::weight() const
{
    Q_ASSERT(m_morphRef);
    return m_morphRef->weight();
}

void MorphRefObject::setWeight(const qreal &value)
{
    Q_ASSERT(m_morphRef);
    if (!qFuzzyCompare(value, weight())) {
        m_morphRef->setWeight(static_cast<IMorph::WeightPrecision>(value));
        emit weightChanged();
    }
}

qreal MorphRefObject::originWeight() const
{
    return m_originWeight;
}

void MorphRefObject::sync()
{
    emit morphDidSync();
}

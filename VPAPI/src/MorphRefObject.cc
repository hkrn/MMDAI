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
#include <QtCore>

#include "LabelRefObject.h"
#include "Util.h"

using namespace vpvl2;

MorphRefObject::MorphRefObject(LabelRefObject *labelRef, IMorph *morphRef, const QUuid &uuid)
    : QObject(labelRef),
      m_parentLabelRef(labelRef),
      m_morphRef(morphRef),
      m_uuid(uuid),
      m_originWeight(0)
{
    Q_ASSERT(m_parentLabelRef);
    Q_ASSERT(m_morphRef);
    Q_ASSERT(!m_uuid.isNull());
    connect(m_parentLabelRef->parentModel(), &ModelProxy::languageChanged, this, &MorphRefObject::nameChanged);
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

LabelRefObject *MorphRefObject::parentLabel() const
{
    return m_parentLabelRef;
}

QUuid MorphRefObject::uuid() const
{
    return m_uuid;
}

QString MorphRefObject::name() const
{
    ModelProxy *parentModel = m_parentLabelRef->parentModel();
    IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(parentModel->language());
    return Util::toQString(m_morphRef->name(language));
}

MorphRefObject::Category MorphRefObject::category() const
{
    switch (m_morphRef->category()) {
    case IMorph::kEye:
        return Eye;
    case IMorph::kLip:
        return Lip;
    case IMorph::kEyeblow:
        return Eyeblow;
    case IMorph::kOther:
        return Other;
    default:
        return Unknown;
    }
}

int MorphRefObject::index() const
{
    return m_morphRef->index();
}

qreal MorphRefObject::weight() const
{
    return m_morphRef->weight();
}

void MorphRefObject::setWeight(const qreal &value)
{
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

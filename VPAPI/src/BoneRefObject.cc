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

#include "BoneRefObject.h"

#include <vpvl2/vpvl2.h>
#include <QtCore>

#include "ModelProxy.h"
#include "LabelRefObject.h"
#include "Util.h"

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

BoneRefObject::BoneRefObject(LabelRefObject *labelRef, IBone *boneRef, const QUuid &uuid)
    : QObject(labelRef),
      m_parentLabelRef(labelRef),
      m_boneRef(boneRef),
      m_uuid(uuid)
{
    Q_ASSERT(m_parentLabelRef);
    Q_ASSERT(m_boneRef);
    Q_ASSERT(!m_uuid.isNull());
    connect(m_parentLabelRef->parentModel(), &ModelProxy::languageChanged, this, &BoneRefObject::nameChanged);
    connect(this, &BoneRefObject::boneDidSync, this, &BoneRefObject::localTranslationChanged);
    connect(this, &BoneRefObject::boneDidSync, this, &BoneRefObject::localOrientationChanged);
}

BoneRefObject::~BoneRefObject()
{
    m_parentLabelRef = 0;
    m_boneRef = 0;
}

void BoneRefObject::setOriginLocalTranslation(const QVector3D &value)
{
    if (!qFuzzyCompare(value, m_originTranslation)) {
        m_originTranslation = value;
        emit originLocalTranslationChanged();
    }
}

void BoneRefObject::setOriginLocalOrientation(const QQuaternion &value)
{
    if (!qFuzzyCompare(value, m_originOrientation)) {
        m_originOrientation = value;
        emit originLocalOrientationChanged();
    }
}

vpvl2::IBone *BoneRefObject::data() const
{
    return m_boneRef;
}

BoneRefObject *BoneRefObject::parentBone() const
{
    Q_ASSERT(m_parentLabelRef);
    return m_parentLabelRef->parentModel()->resolveBoneRef(m_boneRef->parentBoneRef());
}

LabelRefObject *BoneRefObject::parentLabel() const
{
    return m_parentLabelRef;
}

QUuid BoneRefObject::uuid() const
{
    Q_ASSERT(!m_uuid.isNull());
    return m_uuid;
}

QString BoneRefObject::name() const
{
    Q_ASSERT(m_parentLabelRef);
    Q_ASSERT(m_boneRef);
    ModelProxy *parentModel = m_parentLabelRef->parentModel();
    IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(parentModel->language());
    return Util::toQString(m_boneRef->name(language));
}

void BoneRefObject::setName(const QString &value)
{
    Q_ASSERT(m_parentLabelRef);
    Q_ASSERT(m_boneRef);
    if (name() != value) {
        ModelProxy *parentModel = m_parentLabelRef->parentModel();
        IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(parentModel->language());
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_boneRef->setName(s.data(), language);
        m_parentLabelRef->parentModel()->markDirty();
        emit nameChanged();
    }
}

QVector3D BoneRefObject::origin() const
{
    return Util::fromVector3(m_boneRef->origin());
}

void BoneRefObject::setOrigin(const QVector3D &value)
{
    Q_ASSERT(m_boneRef);
    if (!qFuzzyCompare(origin(), value)) {
        m_boneRef->setOrigin(Util::toVector3(value));
        m_parentLabelRef->parentModel()->markDirty();
        emit originChanged();
    }
}

QVector3D BoneRefObject::destinationOrigin() const
{
    return Util::fromVector3(m_boneRef->destinationOrigin());
}

void BoneRefObject::setDestinationOrigin(const QVector3D &value)
{
    Q_ASSERT(m_boneRef);
    if (!qFuzzyCompare(destinationOrigin(), value)) {
        m_boneRef->setDestinationOrigin(Util::toVector3(value));
        m_parentLabelRef->parentModel()->markDirty();
        emit destinationOriginChanged();
    }
}

QVector3D BoneRefObject::fixedAxis() const
{
    return Util::fromVector3(m_boneRef->fixedAxis());
}

void BoneRefObject::setFixedAxis(const QVector3D &value)
{
    Q_ASSERT(m_boneRef);
    if (!qFuzzyCompare(fixedAxis(), value)) {
        m_boneRef->setFixedAxis(Util::toVector3(value));
        m_parentLabelRef->parentModel()->markDirty();
        emit fixedAxisChanged();
    }
}

QVector3D BoneRefObject::localTranslation() const
{
    return Util::fromVector3(rawLocalTranslation());
}

void BoneRefObject::setLocalTranslation(const QVector3D &value)
{
    setRawLocalTranslation(Util::toVector3(value));
}

QQuaternion BoneRefObject::localOrientation() const
{
    return Util::fromQuaternion(rawLocalOrientation());
}

void BoneRefObject::setLocalOrientation(const QQuaternion &value)
{
    setRawLocalOrientation(Util::toQuaternion(value));
}

QVector3D BoneRefObject::localEulerOrientation() const
{
    Scalar yaw, pitch, roll;
    Matrix3x3 matrix(rawLocalOrientation());
    matrix.getEulerZYX(yaw, pitch, roll);
    return QVector3D(qRadiansToDegrees(roll), qRadiansToDegrees(pitch), qRadiansToDegrees(yaw));
}

void BoneRefObject::setLocalEulerOrientation(const QVector3D &value)
{
    if (!qFuzzyCompare(localEulerOrientation(), value)) {
        Quaternion rotation(Quaternion::getIdentity());
        rotation.setEulerZYX(qDegreesToRadians(value.z()), qDegreesToRadians(value.y()), qDegreesToRadians(value.x()));
        setRawLocalOrientation(rotation);
    }
}

QVector3D BoneRefObject::originLocalTranslation() const
{
    return m_originTranslation;
}

QQuaternion BoneRefObject::originLocalOrientation() const
{
    return m_originOrientation;
}

qreal BoneRefObject::coefficient() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->coefficient();
}

void BoneRefObject::setCoefficient(qreal value)
{
    Q_ASSERT(m_boneRef);
    if (!qFuzzyCompare(coefficient(), value)) {
        m_boneRef->coefficient();
        m_parentLabelRef->parentModel()->markDirty();
        emit coefficientChanged();
    }
}

int BoneRefObject::index() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->index();
}

bool BoneRefObject::isInverseKinematicsKEnabled() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isInverseKinematicsEnabled();
}

void BoneRefObject::setInverseKinematicsEnabled(bool value)
{
    Q_ASSERT(m_boneRef);
    if (value != isInverseKinematicsKEnabled()) {
        m_boneRef->setInverseKinematicsEnable(value);
        m_parentLabelRef->parentModel()->markDirty();
        emit inverseKinematicsEnabledChanged();
    }
}

bool BoneRefObject::isMovable() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isMovable();
}

void BoneRefObject::setMovable(bool value)
{
    Q_ASSERT(m_boneRef);
    if (isMovable() != value) {
        m_boneRef->setMovable(value);
        m_parentLabelRef->parentModel()->markDirty();
        emit movableChanged();
    }
}

bool BoneRefObject::isRotateable() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isRotateable();
}

void BoneRefObject::setRotateable(bool value)
{
    Q_ASSERT(m_boneRef);
    if (isRotateable() != value) {
        m_boneRef->setRotateable(value);
        m_parentLabelRef->parentModel()->markDirty();
        emit rotateableChanged();
    }
}

bool BoneRefObject::isVisible() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isVisible();
}

void BoneRefObject::setVisible(bool value)
{
    Q_ASSERT(m_boneRef);
    if (isVisible() != value) {
        m_boneRef->setVisible(value);
        m_parentLabelRef->parentModel()->markDirty();
        emit visibleChanged();
    }
}

bool BoneRefObject::isInteractive() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isInteractive();
}

void BoneRefObject::setInteractive(bool value)
{
    Q_ASSERT(m_boneRef);
    if (isInteractive() != value) {
        m_boneRef->setInteractive(value);
        m_parentLabelRef->parentModel()->markDirty();
        emit interactiveChanged();
    }
}

bool BoneRefObject::isInherenceTranslationEnabled() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isInherentTranslationEnabled();
}

void BoneRefObject::setInherenceTranslationEnabled(bool value)
{
    Q_ASSERT(m_boneRef);
    if (isInherenceTranslationEnabled() != value) {
        m_boneRef->setInherentTranslationEnable(value);
        m_parentLabelRef->parentModel()->markDirty();
        emit inherenceTranslationEnabledChanged();
    }
}

bool BoneRefObject::isInherenceOrientationEnabled() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isInherentOrientationEnabled();
}

void BoneRefObject::setInherenceOrientationEnabled(bool value)
{
    Q_ASSERT(m_boneRef);
    if (isInherenceOrientationEnabled() != value) {
        m_boneRef->setInherentOrientationEnable(value);
        m_parentLabelRef->parentModel()->markDirty();
        emit inherenceOrientationEnabledChanged();
    }
}

bool BoneRefObject::isFixedAxisEnabled() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->hasFixedAxes();
}

void BoneRefObject::setFixedAxisEnabled(bool value)
{
    Q_ASSERT(m_boneRef);
    if (isFixedAxisEnabled() != value) {
        m_boneRef->setFixedAxisEnable(value);
        m_parentLabelRef->parentModel()->markDirty();
        emit fixedAxisEnabledChanged();
    }
}

bool BoneRefObject::isLocalAxesEnabled() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->hasLocalAxes();
}

void BoneRefObject::setLocalAxesEnabled(bool value)
{
    Q_ASSERT(m_boneRef);
    if (isLocalAxesEnabled() != value) {
        m_boneRef->setLocalAxesEnable(value);
        m_parentLabelRef->parentModel()->markDirty();
        emit localAxesEnabledChanged();
    }
}

void BoneRefObject::sync()
{
    emit boneDidSync();
}

bool BoneRefObject::canHandle() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isVisible() && m_boneRef->isInteractive() && (m_boneRef->isRotateable() || m_boneRef->isMovable());
}

Vector3 BoneRefObject::rawFixedAxis() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->fixedAxis();
}

void BoneRefObject::getLocalAxes(Matrix3x3 &value)
{
    Q_ASSERT(m_boneRef);
    m_boneRef->getLocalAxes(value);
}

Vector3 BoneRefObject::rawLocalTranslation() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->localTranslation();
}

void BoneRefObject::setRawLocalTranslation(const Vector3 &value)
{
    Q_ASSERT(m_boneRef);
    if (!(value - m_boneRef->localTranslation()).fuzzyZero()) {
        m_boneRef->setLocalTranslation(value);
        emit localTranslationChanged();
    }
}

Quaternion BoneRefObject::rawLocalOrientation() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->localOrientation();
}

void BoneRefObject::setRawLocalOrientation(const Quaternion &value)
{
    Q_ASSERT(m_boneRef);
    if (!btFuzzyZero((value - m_boneRef->localOrientation()).length2())) {
        m_boneRef->setLocalOrientation(value);
        emit localOrientationChanged();
    }
}

Transform BoneRefObject::rawWorldTransform() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->worldTransform();
}


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

BoneRefObject::BoneRefObject(ModelProxy *modelRef, IBone *boneRef, const QUuid &uuid)
    : QObject(modelRef),
      m_parentModelRef(modelRef),
      m_parentLabelRef(0),
      m_boneRef(boneRef),
      m_uuid(uuid),
      m_dirty(false)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_boneRef);
    Q_ASSERT(!m_uuid.isNull());
    connect(m_parentModelRef, &ModelProxy::languageChanged, this, &BoneRefObject::nameChanged);
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

QJsonValue BoneRefObject::toJson() const
{
    QJsonObject v;
    v.insert("uuid", uuid().toString());
    v.insert("name", name());
    v.insert("destinationOriginBone", (destinationOriginBone() ? destinationOriginBone()->uuid() : QUuid()).toString());
    v.insert("parentInherentBone", (parentInherentBone() ? parentInherentBone()->uuid() : QUuid()).toString());
    v.insert("origin", Util::toJson(origin()));
    v.insert("destinationOrigin", Util::toJson(destinationOrigin()));
    v.insert("fixedAxis", Util::toJson(fixedAxis()));
    v.insert("inherentCoefficient", inherentCoefficient());
    v.insert("inverseKinematicsEnabled", isInverseKinematicsKEnabled());
    v.insert("movable", isMovable());
    v.insert("rotateable", isRotateable());
    v.insert("visible", isVisible());
    v.insert("interactive", isInteractive());
    v.insert("inherentTranslationEnabled", isInherentTranslationEnabled());
    v.insert("inherentOrientationEnabled", isInherentOrientationEnabled());
    v.insert("fixedAxisEnabled", isFixedAxisEnabled());
    v.insert("localAxesEnabled", isLocalAxesEnabled());
    return v;
}

vpvl2::IBone *BoneRefObject::data() const
{
    return m_boneRef;
}

ModelProxy *BoneRefObject::parentModel() const
{
    return m_parentModelRef;
}

BoneRefObject *BoneRefObject::parentBone() const
{
    Q_ASSERT(m_parentModelRef);
    return m_parentModelRef->resolveBoneRef(m_boneRef->parentBoneRef());
}

LabelRefObject *BoneRefObject::parentLabel() const
{
    return m_parentLabelRef;
}

void BoneRefObject::setParentLabel(LabelRefObject *value)
{
    if (parentLabel() != value) {
        m_parentLabelRef = value;
        emit parentLabelChanged();
    }
}

QUuid BoneRefObject::uuid() const
{
    Q_ASSERT(!m_uuid.isNull());
    return m_uuid;
}

QString BoneRefObject::name() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_boneRef);
    ModelProxy *parentModel = m_parentModelRef;
    IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(parentModel->language());
    const IString *name = m_boneRef->name(language);
    return Util::toQString((name && name->size() > 0) ? name : m_boneRef->name(IEncoding::kDefaultLanguage));
}

BoneRefObject *BoneRefObject::destinationOriginBone() const
{
    Q_ASSERT(m_parentModelRef);
    return m_parentModelRef->resolveBoneRef(m_boneRef->destinationOriginBoneRef());
}

void BoneRefObject::setDestinationOriginBone(BoneRefObject *value)
{
    if (destinationOriginBone() != value) {
        m_boneRef->setDestinationOriginBoneRef(value->data());
        emit destinationOriginBoneChanged();
    }
}

BoneRefObject *BoneRefObject::parentInherentBone() const
{
    Q_ASSERT(m_parentModelRef);
    return m_parentModelRef->resolveBoneRef(m_boneRef->parentInherentBoneRef());
}

void BoneRefObject::setParentInherentBone(BoneRefObject *value)
{
    if (parentInherentBone() != value) {
        m_boneRef->setParentInherentBoneRef(value->data());
        emit parentInherentBoneChanged();
    }
}

void BoneRefObject::setName(const QString &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_boneRef);
    if (name() != value) {
        ModelProxy *parentModel = m_parentModelRef;
        IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(parentModel->language());
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_parentModelRef->renameObject(this, value);
        m_boneRef->setName(s.data(), language);
        setDirty(true);
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
        setDirty(true);
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
        setDirty(true);
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
        setDirty(true);
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

qreal BoneRefObject::inherentCoefficient() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->inherentCoefficient();
}

void BoneRefObject::setInherentCoefficient(qreal value)
{
    Q_ASSERT(m_boneRef);
    if (!qFuzzyCompare(inherentCoefficient(), value)) {
        m_boneRef->inherentCoefficient();
        setDirty(true);
        emit inherentCoefficientChanged();
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
        setDirty(true);
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
        setDirty(true);
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
        setDirty(true);
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
        setDirty(true);
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
        setDirty(true);
        emit interactiveChanged();
    }
}

bool BoneRefObject::isInherentTranslationEnabled() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isInherentTranslationEnabled();
}

void BoneRefObject::setInherentTranslationEnabled(bool value)
{
    Q_ASSERT(m_boneRef);
    if (isInherentTranslationEnabled() != value) {
        m_boneRef->setInherentTranslationEnable(value);
        setDirty(true);
        emit inherentTranslationEnabledChanged();
    }
}

bool BoneRefObject::isInherentOrientationEnabled() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isInherentOrientationEnabled();
}

void BoneRefObject::setInherentOrientationEnabled(bool value)
{
    Q_ASSERT(m_boneRef);
    if (isInherentOrientationEnabled() != value) {
        m_boneRef->setInherentOrientationEnable(value);
        setDirty(true);
        emit inherentOrientationEnabledChanged();
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
        setDirty(true);
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
        setDirty(true);
        emit localAxesEnabledChanged();
    }
}

bool BoneRefObject::isDirty() const
{
    return m_dirty;
}

void BoneRefObject::setDirty(bool value)
{
    if (isDirty() != value) {
        m_dirty = value;
        emit dirtyChanged();
        if (value) {
            m_parentModelRef->markDirty();
        }
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


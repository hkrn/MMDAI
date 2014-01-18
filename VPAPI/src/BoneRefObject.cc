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
        emit enableInverseKinematicsChanged();
    }
}

bool BoneRefObject::isMovable() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isMovable();
}

bool BoneRefObject::isRotateable() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->isRotateable();
}

bool BoneRefObject::hasInverseKinematics() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->hasInverseKinematics();
}

bool BoneRefObject::hasFixedAxes() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->hasFixedAxes();
}

bool BoneRefObject::hasLocalAxes() const
{
    Q_ASSERT(m_boneRef);
    return m_boneRef->hasLocalAxes();
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

Vector3 BoneRefObject::fixedAxis() const
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


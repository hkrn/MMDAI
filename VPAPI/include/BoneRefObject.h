/**

 Copyright (c) 2010-2013  hkrn

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

#ifndef BONEREFOBJECT_H
#define BONEREFOBJECT_H

#include <QObject>
#include <QQuaternion>
#include <QUuid>
#include <QVector3D>
#include <vpvl2/Common.h>

class LabelRefObject;

namespace vpvl2 {
class IBone;
}

class BoneRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(BoneRefObject *parentBone READ parentBone CONSTANT FINAL)
    Q_PROPERTY(LabelRefObject *parentLabel READ parentLabel CONSTANT FINAL)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged FINAL)
    Q_PROPERTY(QVector3D localTranslation READ localTranslation WRITE setLocalTranslation NOTIFY localTranslationChanged FINAL)
    Q_PROPERTY(QQuaternion localOrientation READ localOrientation WRITE setLocalOrientation NOTIFY localOrientationChanged FINAL)
    Q_PROPERTY(QVector3D localEulerOrientation READ localEulerOrientation WRITE setLocalEulerOrientation NOTIFY localOrientationChanged FINAL)
    Q_PROPERTY(QVector3D originLocalTranslation READ originLocalTranslation NOTIFY originLocalTranslationChanged FINAL)
    Q_PROPERTY(QQuaternion originLocalOrientation READ originLocalOrientation NOTIFY originLocalOrientationChanged FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(bool enableInverseKinematics READ isInverseKinematicsKEnabled WRITE setInverseKinematicsEnabled NOTIFY enableInverseKinematicsChanged FINAL)
    Q_PROPERTY(bool movable READ isMovable CONSTANT FINAL)
    Q_PROPERTY(bool rotateable READ isRotateable CONSTANT FINAL)
    Q_PROPERTY(bool hasInverseKinematics READ hasInverseKinematics CONSTANT FINAL)
    Q_PROPERTY(bool hasFixedAxes READ hasFixedAxes CONSTANT FINAL)
    Q_PROPERTY(bool hasLocalAxes READ hasLocalAxes CONSTANT FINAL)

public:
    BoneRefObject(LabelRefObject *labelRef, vpvl2::IBone *boneRef, const QUuid &uuid);
    ~BoneRefObject();

    void setOriginLocalTranslation(const QVector3D &value);
    void setOriginLocalOrientation(const QQuaternion &value);

    vpvl2::IBone *data() const;
    BoneRefObject *parentBone() const;
    LabelRefObject *parentLabel() const;
    QUuid uuid() const;
    QString name() const;
    QVector3D localTranslation() const;
    void setLocalTranslation(const QVector3D &value);
    QQuaternion localOrientation() const;
    void setLocalOrientation(const QQuaternion &value);
    QVector3D localEulerOrientation() const;
    void setLocalEulerOrientation(const QVector3D &value);
    QVector3D originLocalTranslation() const;
    QQuaternion originLocalOrientation() const;
    int index() const;
    bool isInverseKinematicsKEnabled() const;
    void setInverseKinematicsEnabled(bool value);
    bool isMovable() const;
    bool isRotateable() const;
    bool hasInverseKinematics() const;
    bool hasFixedAxes() const;
    bool hasLocalAxes() const;

    bool canHandle() const;
    vpvl2::Vector3 fixedAxis() const;
    void getLocalAxes(vpvl2::Matrix3x3 &value);
    vpvl2::Vector3 rawLocalTranslation() const;
    void setRawLocalTranslation(const vpvl2::Vector3 &value);
    vpvl2::Quaternion rawLocalOrientation() const;
    void setRawLocalOrientation(const vpvl2::Quaternion &value);
    vpvl2::Transform rawWorldTransform() const;

public slots:
    Q_INVOKABLE void sync();

signals:
    void nameChanged();
    void localTranslationChanged();
    void localOrientationChanged();
    void localEulerorientationChanged();
    void originLocalTranslationChanged();
    void originLocalOrientationChanged();
    void enableInverseKinematicsChanged();
    void boneDidSync();

private:
    LabelRefObject *m_parentLabelRef;
    vpvl2::IBone *m_boneRef;
    const QUuid m_uuid;
    QVector3D m_originTranslation;
    QQuaternion m_originOrientation;
};

#endif // BONEREFOBJECT_H

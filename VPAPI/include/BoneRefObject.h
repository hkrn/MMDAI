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

#ifndef BONEREFOBJECT_H
#define BONEREFOBJECT_H

#include <QJsonValue>
#include <QObject>
#include <QQuaternion>
#include <QUuid>
#include <QVector3D>
#include <vpvl2/Common.h>

class LabelRefObject;
class ModelProxy;

namespace vpvl2 {
namespace VPVL2_VERSION_NS {
class IBone;
}
using namespace VPVL2_VERSION_NS;
}

class BoneRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ModelProxy *parentModel READ parentModel CONSTANT FINAL)
    Q_PROPERTY(BoneRefObject *parentBone READ parentBone CONSTANT FINAL)
    Q_PROPERTY(LabelRefObject *parentLabel READ parentLabel NOTIFY parentLabelChanged FINAL)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(BoneRefObject *destinationOriginBone READ destinationOriginBone WRITE setDestinationOriginBone NOTIFY destinationOriginBoneChanged FINAL)
    Q_PROPERTY(BoneRefObject *parentInherentBone READ parentInherentBone WRITE setParentInherentBone NOTIFY parentInherentBoneChanged FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(QVector3D origin READ origin WRITE setOrigin NOTIFY originChanged FINAL)
    Q_PROPERTY(QVector3D destinationOrigin READ destinationOrigin WRITE setDestinationOrigin NOTIFY destinationOriginChanged FINAL)
    Q_PROPERTY(QVector3D fixedAxis READ fixedAxis WRITE setFixedAxis NOTIFY fixedAxisChanged FINAL)
    Q_PROPERTY(QVector3D localTranslation READ localTranslation WRITE setLocalTranslation NOTIFY localTranslationChanged FINAL)
    Q_PROPERTY(QQuaternion localOrientation READ localOrientation WRITE setLocalOrientation NOTIFY localOrientationChanged FINAL)
    Q_PROPERTY(QVector3D localEulerOrientation READ localEulerOrientation WRITE setLocalEulerOrientation NOTIFY localOrientationChanged FINAL)
    Q_PROPERTY(QVector3D originLocalTranslation READ originLocalTranslation NOTIFY originLocalTranslationChanged FINAL)
    Q_PROPERTY(QQuaternion originLocalOrientation READ originLocalOrientation NOTIFY originLocalOrientationChanged FINAL)
    Q_PROPERTY(qreal inherentCoefficient READ inherentCoefficient WRITE setInherentCoefficient NOTIFY inherentCoefficientChanged)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(bool inverseKinematicsEnabled READ isInverseKinematicsKEnabled WRITE setInverseKinematicsEnabled NOTIFY inverseKinematicsEnabledChanged FINAL)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable NOTIFY movableChanged FINAL)
    Q_PROPERTY(bool rotateable READ isRotateable WRITE setRotateable NOTIFY rotateableChanged FINAL)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive NOTIFY interactiveChanged FINAL)
    Q_PROPERTY(bool inherentTranslationEnabled READ isInherentTranslationEnabled WRITE setInherentTranslationEnabled NOTIFY inherentTranslationEnabledChanged FINAL)
    Q_PROPERTY(bool inherentOrientationEnabled READ isInherentOrientationEnabled WRITE setInherentOrientationEnabled NOTIFY inherentOrientationEnabledChanged FINAL)
    Q_PROPERTY(bool fixedAxisEnabled READ isFixedAxisEnabled WRITE setFixedAxisEnabled NOTIFY fixedAxisEnabledChanged FINAL)
    Q_PROPERTY(bool localAxesEnabled READ isLocalAxesEnabled WRITE setLocalAxesEnabled NOTIFY localAxesEnabledChanged FINAL)
    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged FINAL)

public:
    BoneRefObject(ModelProxy *modelRef, vpvl2::IBone *boneRef, const QUuid &uuid);
    ~BoneRefObject();

    void setOriginLocalTranslation(const QVector3D &value);
    void setOriginLocalOrientation(const QQuaternion &value);
    Q_INVOKABLE QJsonValue toJson() const;

    vpvl2::IBone *data() const;
    ModelProxy *parentModel() const;
    BoneRefObject *parentBone() const;
    LabelRefObject *parentLabel() const;
    void setParentLabel(LabelRefObject *value);
    QUuid uuid() const;
    QString name() const;
    BoneRefObject *destinationOriginBone() const;
    void setDestinationOriginBone(BoneRefObject *value);
    BoneRefObject *parentInherentBone() const;
    void setParentInherentBone(BoneRefObject *value);
    void setName(const QString &value);
    QVector3D origin() const;
    void setOrigin(const QVector3D &value);
    QVector3D destinationOrigin() const;
    void setDestinationOrigin(const QVector3D &value);
    QVector3D fixedAxis() const;
    void setFixedAxis(const QVector3D &value);
    QVector3D localTranslation() const;
    void setLocalTranslation(const QVector3D &value);
    QQuaternion localOrientation() const;
    void setLocalOrientation(const QQuaternion &value);
    QVector3D localEulerOrientation() const;
    void setLocalEulerOrientation(const QVector3D &value);
    QVector3D originLocalTranslation() const;
    QQuaternion originLocalOrientation() const;
    qreal inherentCoefficient() const;
    void setInherentCoefficient(qreal value);
    int index() const;
    bool isInverseKinematicsKEnabled() const;
    void setInverseKinematicsEnabled(bool value);
    bool isMovable() const;
    void setMovable(bool value);
    bool isRotateable() const;
    void setRotateable(bool value);
    bool isVisible() const;
    void setVisible(bool value);
    bool isInteractive() const;
    void setInteractive(bool value);
    bool hasInverseKinematics() const;
    bool isInherentTranslationEnabled() const;
    void setInherentTranslationEnabled(bool value);
    bool isInherentOrientationEnabled() const;
    void setInherentOrientationEnabled(bool value);
    bool isFixedAxisEnabled() const;
    void setFixedAxisEnabled(bool value);
    bool isLocalAxesEnabled() const;
    void setLocalAxesEnabled(bool value);
    bool isDirty() const;
    void setDirty(bool value);

    bool canHandle() const;
    vpvl2::Vector3 rawFixedAxis() const;
    void getLocalAxes(vpvl2::Matrix3x3 &value);
    vpvl2::Vector3 rawLocalTranslation() const;
    void setRawLocalTranslation(const vpvl2::Vector3 &value);
    vpvl2::Quaternion rawLocalOrientation() const;
    void setRawLocalOrientation(const vpvl2::Quaternion &value);
    vpvl2::Transform rawWorldTransform() const;

public slots:
    Q_INVOKABLE void sync();

signals:
    void parentLabelChanged();
    void destinationOriginBoneChanged();
    void parentInherentBoneChanged();
    void nameChanged();
    void originChanged();
    void destinationOriginChanged();
    void fixedAxisChanged();
    void localTranslationChanged();
    void localOrientationChanged();
    void localEulerorientationChanged();
    void originLocalTranslationChanged();
    void originLocalOrientationChanged();
    void inverseKinematicsEnabledChanged();
    void inherentCoefficientChanged();
    void movableChanged();
    void rotateableChanged();
    void visibleChanged();
    void interactiveChanged();
    void inherentTranslationEnabledChanged();
    void inherentOrientationEnabledChanged();
    void fixedAxisEnabledChanged();
    void localAxesEnabledChanged();
    void dirtyChanged();
    void boneDidSync();

private:
    ModelProxy *m_parentModelRef;
    LabelRefObject *m_parentLabelRef;
    vpvl2::IBone *m_boneRef;
    const QUuid m_uuid;
    QVector3D m_originTranslation;
    QQuaternion m_originOrientation;
    bool m_dirty;
};

#endif // BONEREFOBJECT_H

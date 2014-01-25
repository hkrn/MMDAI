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

#ifndef JOINTREFOBJECT_H
#define JOINTREFOBJECT_H

#include <QObject>
#include <QUuid>
#include <QVector3D>
#include <vpvl2/IJoint.h>

class ModelProxy;
class RigidBodyRefObject;

class JointRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ModelProxy *parentModel READ parentModel CONSTANT FINAL)
    Q_PROPERTY(RigidBodyRefObject *bodyA READ bodyA CONSTANT FINAL)
    Q_PROPERTY(RigidBodyRefObject *bodyB READ bodyB CONSTANT FINAL)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged FINAL)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(QVector3D rotation READ rotation WRITE setRotation NOTIFY rotationChanged FINAL)
    Q_PROPERTY(QVector3D positionUpperLimit READ positionUpperLimit WRITE setPositionUpperLimit NOTIFY positionUpperLimitChanged FINAL)
    Q_PROPERTY(QVector3D rotationUpperLimit READ rotationUpperLimit WRITE setRotationUpperLimit NOTIFY rotationUpperLimitChanged FINAL)
    Q_PROPERTY(QVector3D positionLowerLimit READ positionLowerLimit WRITE setPositionLowerLimit NOTIFY positionLowerLimitChanged FINAL)
    Q_PROPERTY(QVector3D rotationLowerLimit READ rotationLowerLimit WRITE setRotationLowerLimit NOTIFY rotationLowerLimitChanged FINAL)
    Q_PROPERTY(QVector3D positionStiffness READ positionStiffness WRITE setPositionStiffness NOTIFY positionStiffnessChanged FINAL)
    Q_PROPERTY(QVector3D rotationStiffness READ rotationStiffness WRITE setRotationStiffness NOTIFY rotationStiffnessChanged FINAL)

public:
    enum Type {
        Generic6DofSpringConstraint = vpvl2::IJoint::kGeneric6DofSpringConstraint,
        Generic6DofConstraint       = vpvl2::IJoint::kGeneric6DofConstraint,
        Point2PointConstraint       = vpvl2::IJoint::kPoint2PointConstraint,
        ConeTwistConstraint         = vpvl2::IJoint::kConeTwistConstraint,
        SliderConstraint            = vpvl2::IJoint::kSliderConstraint,
        HigeConstraint              = vpvl2::IJoint::kHingeConstraint
    };

    JointRefObject(ModelProxy *parentModel,
                   RigidBodyRefObject *bodyA,
                   RigidBodyRefObject *bodyB,
                   vpvl2::IJoint *jointRef,
                   const QUuid &uuid);
    ~JointRefObject();

    vpvl2::IJoint *data() const;
    ModelProxy *parentModel() const;
    RigidBodyRefObject *bodyA() const;
    RigidBodyRefObject *bodyB() const;
    QUuid uuid() const;
    int index() const;
    QString name() const;
    void setName(const QString &value);
    Type type() const;
    void setType(const Type &value);
    QVector3D position() const;
    void setPosition(const QVector3D &value);
    QVector3D rotation() const;
    void setRotation(const QVector3D &value);
    QVector3D positionUpperLimit() const;
    void setPositionUpperLimit(const QVector3D &value);
    QVector3D rotationUpperLimit() const;
    void setRotationUpperLimit(const QVector3D &value);
    QVector3D positionLowerLimit() const;
    void setPositionLowerLimit(const QVector3D &value);
    QVector3D rotationLowerLimit() const;
    void setRotationLowerLimit(const QVector3D &value);
    QVector3D positionStiffness() const;
    void setPositionStiffness(const QVector3D &value);
    QVector3D rotationStiffness() const;
    void setRotationStiffness(const QVector3D &value);

signals:
    void nameChanged();
    void typeChanged();
    void positionChanged();
    void rotationChanged();
    void positionUpperLimitChanged();
    void rotationUpperLimitChanged();
    void positionLowerLimitChanged();
    void rotationLowerLimitChanged();
    void positionStiffnessChanged();
    void rotationStiffnessChanged();

private:
    ModelProxy *m_parentModelRef;
    RigidBodyRefObject *m_bodyARef;
    RigidBodyRefObject *m_bodyBRef;
    vpvl2::IJoint *m_jointRef;
    const QUuid m_uuid;
};

#endif

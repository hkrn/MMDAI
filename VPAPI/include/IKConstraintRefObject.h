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

#ifndef IKREFOBJECT_H
#define IKREFOBJECT_H

#include <QJsonValue>
#include <QObject>
#include <QQmlListProperty>
#include <QUuid>
#include <QVector3D>
#include <vpvl2/IModel.h>

class BoneRefObject;
class IKConstraintRefObject;
class ModelProxy;

class ChildIKJoint : public QObject
{
    Q_OBJECT
    Q_PROPERTY(IKConstraintRefObject *parentConstraint READ parentConstraint CONSTANT FINAL)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged FINAL)
    Q_PROPERTY(BoneRefObject *targetBone READ targetBone CONSTANT FINAL)
    Q_PROPERTY(QVector3D upperLimit READ upperLimit WRITE setUpperLimit NOTIFY upperLimitChanged FINAL)
    Q_PROPERTY(QVector3D lowerLimit READ lowerLimit WRITE setLowerLimit NOTIFY lowerLimitChanged FINAL)
    Q_PROPERTY(QVector3D degreeUpperLimit READ degreeUpperLimit WRITE setDegreeUpperLimit NOTIFY upperLimitChanged FINAL)
    Q_PROPERTY(QVector3D degreeLowerLimit READ degreeLowerLimit WRITE setDegreeLowerLimit NOTIFY lowerLimitChanged FINAL)
    Q_PROPERTY(bool hasAngleLimit READ hasAngleLimit WRITE setHasAngleLimit NOTIFY hasAngleLimitChanged FINAL)

public:
    ChildIKJoint(IKConstraintRefObject *parentConstraintRef, vpvl2::IBone::IKJoint *jointRef);
    ~ChildIKJoint();

    IKConstraintRefObject *parentConstraint() const;
    BoneRefObject *targetBone() const;
    QString name() const;
    QVector3D upperLimit() const;
    void setUpperLimit(const QVector3D &value);
    QVector3D lowerLimit() const;
    void setLowerLimit(const QVector3D &value);
    QVector3D degreeUpperLimit() const;
    void setDegreeUpperLimit(const QVector3D &value);
    QVector3D degreeLowerLimit() const;
    void setDegreeLowerLimit(const QVector3D &value);
    bool hasAngleLimit() const;
    void setHasAngleLimit(bool value);

signals:
    void nameChanged();
    void upperLimitChanged();
    void lowerLimitChanged();
    void hasAngleLimitChanged();

private:
    IKConstraintRefObject *m_parentConstraintRef;
    vpvl2::IBone::IKJoint *m_jointRef;
};

class IKConstraintRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ModelProxy *parentModel READ parentModel CONSTANT FINAL)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(QQmlListProperty<ChildIKJoint> allChildJoints READ allChildJoints NOTIFY allChildJointsChanged)
    Q_PROPERTY(BoneRefObject *rootBone READ rootBone WRITE setRootBone NOTIFY rootBoneChanged FINAL)
    Q_PROPERTY(BoneRefObject *effectorBone READ effectorBone WRITE setEffectorBone NOTIFY effectorBoneChanged FINAL)
    Q_PROPERTY(qreal angleLimit READ angleLimit WRITE setAngleLimit NOTIFY angleLimitChanged FINAL)
    Q_PROPERTY(int numIterations READ numIterations WRITE setNumIterations NOTIFY numIterationsChanged FINAL)
    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged FINAL)

public:
    IKConstraintRefObject(ModelProxy *parentModel, vpvl2::IBone::IKConstraint *constraintRef, const QUuid &uuid, int index);
    ~IKConstraintRefObject();

    void initialize();

    ModelProxy *parentModel() const;
    QUuid uuid() const;
    QString name() const;
    int index() const;
    QQmlListProperty<ChildIKJoint> allChildJoints();
    BoneRefObject *rootBone() const;
    void setRootBone(BoneRefObject *value);
    BoneRefObject *effectorBone() const;
    void setEffectorBone(BoneRefObject *value);
    qreal angleLimit() const;
    void setAngleLimit(qreal value);
    int numIterations() const;
    void setNumIterations(int value);
    bool isDirty() const;
    void setDirty(bool value);

signals:
    void allChildJointsChanged();
    void rootBoneChanged();
    void effectorBoneChanged();
    void angleLimitChanged();
    void numIterationsChanged();
    void dirtyChanged();

private:
    ModelProxy *m_parentModelRef;
    QList<ChildIKJoint *> m_childJoints;
    vpvl2::IBone::IKConstraint *m_constraintRef;
    const QUuid m_uuid;
    int m_index;
    bool m_dirty;
};

#endif


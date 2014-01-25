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
#include <vpvl2/Common.h>

class ModelProxy;
class RigidBodyRefObject;

namespace vpvl2 {
class IJoint;
}

class JointRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ModelProxy *parentModel READ parentModel CONSTANT FINAL)
    Q_PROPERTY(RigidBodyRefObject *bodyA READ bodyA CONSTANT FINAL)
    Q_PROPERTY(RigidBodyRefObject *bodyB READ bodyB CONSTANT FINAL)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)

public:
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

signals:
    void nameChanged();

private:
    ModelProxy *m_parentModelRef;
    RigidBodyRefObject *m_bodyARef;
    RigidBodyRefObject *m_bodyBRef;
    vpvl2::IJoint *m_jointRef;
    const QUuid m_uuid;
};

#endif

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

#ifndef WORLDPROXY_H
#define WORLDPROXY_H

#include <QObject>
#include <QVector3D>
#include <vpvl2/Common.h>

class btRigidBody;

namespace vpvl2 {
namespace extensions {
class World;
}
}

class BoneRefObject;
class ModelProxy;
class ProjectProxy;

class WorldProxy : public QObject
{
    Q_OBJECT
    Q_ENUMS(SimulationType)
    Q_PROPERTY(SimulationType simulationType READ simulationType WRITE setSimulationType NOTIFY simulationTypeChanged FINAL)
    Q_PROPERTY(QVector3D gravity READ gravity WRITE setGravity NOTIFY gravityChanged)
    Q_PROPERTY(int randSeed READ randSeed WRITE setRandSeed NOTIFY randSeedChanged)
    Q_PROPERTY(bool enableFloor READ isFloorEnabled WRITE setFloorEnabled NOTIFY enableFloorChanged)

public:
    enum SimulationType {
        EnableSimulationAnytime,
        EnableSimulationPlayOnly,
        DisableSimulation
    };

    explicit WorldProxy(ProjectProxy *parent);
    ~WorldProxy();

    BoneRefObject *ray(const vpvl2::Vector3 &from, const vpvl2::Vector3 &to);
    void joinWorld(ModelProxy *value);
    void leaveWorld(ModelProxy *value);
    void resetProjectInstance(ProjectProxy *value);
    void stepSimulation();

    SimulationType simulationType() const;
    void setSimulationType(SimulationType value);
    QVector3D gravity() const;
    void setGravity(const QVector3D &value);
    int randSeed() const;
    void setRandSeed(int value);
    bool isFloorEnabled() const;
    void setFloorEnabled(bool value);

signals:
    void simulationTypeChanged();
    void gravityChanged();
    void randSeedChanged();
    void enableFloorChanged();

private:
    QScopedPointer<vpvl2::extensions::World> m_sceneWorld;
    QScopedPointer<vpvl2::extensions::World> m_modelWorld;
    QScopedPointer<btRigidBody> m_groundBody;
    ProjectProxy *m_parentProjectProxyRef;
    SimulationType m_simulationType;
    QVector3D m_gravity;
    bool m_enableFloor;
};

#endif // WORLDPROXY_H

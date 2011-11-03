/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

#ifndef VPVL_NO_BULLET
#include <BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h>
#else
VPVL_DECLARE_HANDLE(btGeneric6DofConstraint)
VPVL_DECLARE_HANDLE(btGeneric6DofSpringConstraint)
#endif

namespace vpvl
{

#pragma pack(push, 1)

struct ConstraintChunk
{
    uint8_t name[Constraint::kNameSize];
    int bodyIDA;
    int bodyIDB;
    float position[3];
    float rotation[3];
    float limitPositionFrom[3];
    float limitPositionTo[3];
    float limitRotationFrom[3];
    float limitRotationTo[3];
    float stiffness[6];
};

#pragma pack(pop)

size_t Constraint::stride()
{
    return sizeof(ConstraintChunk);
}

Constraint::Constraint()
    : m_constraint(0),
      m_position(0.0f, 0.0f, 0.0f),
      m_rotation(0.0f, 0.0f, 0.0f),
      m_limitPositionFrom(0.0f, 0.0f, 0.0f),
      m_limitPositionTo(0.0f, 0.0f, 0.0f),
      m_limitRotationFrom(0.0f, 0.0f, 0.0f),
      m_limitRotationTo(0.0f, 0.0f, 0.0f),
      m_bodyA(0),
      m_bodyB(0)
{
    internal::zerofill(m_name, sizeof(m_name));
    internal::zerofill(m_stiffness, sizeof(m_stiffness));
}

Constraint::~Constraint()
{
    internal::zerofill(m_name, sizeof(m_name));
    internal::zerofill(m_stiffness, sizeof(m_stiffness));
    m_position.setZero();
    m_rotation.setZero();
    m_limitPositionFrom.setZero();
    m_limitPositionTo.setZero();
    m_limitRotationFrom.setZero();
    m_limitRotationTo.setZero();
    m_bodyA = 0;
    m_bodyB = 0;
    delete m_constraint;
    m_constraint = 0;
}

void Constraint::read(const uint8_t *data, const RigidBodyList &bodies, const Vector3 &offset)
{
#ifndef VPVL_NO_BULLET
    ConstraintChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    copyBytesSafe(m_name, chunk.name, sizeof(m_name));
    int32_t bodyID1 = chunk.bodyIDA;
    int32_t bodyID2 = chunk.bodyIDB;

#ifdef VPVL_BUILD_IOS
    float pos[3], rot[3], limitPosFrom[3], limitPosTo[3], limitRotFrom[3], limitRotTo[3], stiffness[6];
    memcpy(pos, &chunk.position, sizeof(pos));
    memcpy(rot, &chunk.rotation, sizeof(rot));
    memcpy(limitPosFrom, &chunk.limitPositionFrom, sizeof(limitPosFrom));
    memcpy(limitPosTo, &chunk.limitPositionTo, sizeof(limitPosTo));
    memcpy(limitRotFrom, &chunk.limitRotationFrom, sizeof(limitRotFrom));
    memcpy(limitRotTo, &chunk.limitRotationTo, sizeof(limitRotTo));
    memcpy(stiffness, &chunk.stiffness, sizeof(stiffness));
#else
    float *pos = chunk.position;
    float *rot = chunk.rotation;
    float *limitPosFrom = chunk.limitPositionFrom;
    float *limitPosTo = chunk.limitPositionTo;
    float *limitRotFrom = chunk.limitRotationFrom;
    float *limitRotTo = chunk.limitRotationTo;
    float *stiffness = chunk.stiffness;
#endif

    int nbodies = bodies.count();
    if (bodyID1 >= 0 && bodyID1 < nbodies &&bodyID2 >= 0 && bodyID2 < nbodies) {
        Transform transform;
        btMatrix3x3 basis;
        transform.setIdentity();
#ifdef VPVL_COORDINATE_OPENGL
        btMatrix3x3 mx, my, mz;
        mx.setEulerZYX(-rot[0], 0.0f, 0.0f);
        my.setEulerZYX(0.0f, -rot[1], 0.0f);
        mz.setEulerZYX(0.0f, 0.0f, rot[2]);
        basis = my * mz * mx;
#else  /* VPVL_COORDINATE_OPENGL */
        basis.setEulerZYX(rot[0], rot[1], rot[2]);
#endif /* VPVL_COORDINATE_OPENGL */
        transform.setBasis(basis);
#ifdef VPVL_COORDINATE_OPENGL
        transform.setOrigin(Vector3(pos[0], pos[1], -pos[2]) + offset);
#else  /* VPVL_COORDINATE_OPENGL */
        transform.setOrigin(Vector3(pos[0], pos[1], pos[2]) + offset);
#endif /* VPVL_COORDINATE_OPENGL */
        btRigidBody *bodyA = bodies[bodyID1]->body(), *bodyB = bodies[bodyID2]->body();
        Transform transformA = bodyA->getWorldTransform().inverse() * transform,
                transformB = bodyB->getWorldTransform().inverse() * transform;
        m_constraint = new btGeneric6DofSpringConstraint(*bodyA, *bodyB, transformA, transformB, true);
#ifdef VPVL_COORDINATE_OPENGL
        m_constraint->setLinearUpperLimit(Vector3(limitPosTo[0], limitPosTo[1], -limitPosFrom[2]));
        m_constraint->setLinearLowerLimit(Vector3(limitPosFrom[0], limitPosFrom[1], -limitPosTo[2]));
        m_constraint->setAngularUpperLimit(Vector3(-limitRotFrom[0], -limitRotFrom[1], limitRotTo[2]));
        m_constraint->setAngularLowerLimit(Vector3(-limitRotTo[0], -limitRotTo[1], limitRotFrom[2]));
#else  /* VPVL_COORDINATE_OPENGL */
        m_constraint->setLinearUpperLimit(Vector3(limitPosTo[0], limitPosTo[1], limitPosTo[2]));
        m_constraint->setLinearLowerLimit(Vector3(limitPosFrom[0], limitPosFrom[1], limitPosFrom[2]));
        m_constraint->setAngularUpperLimit(Vector3(limitRotTo[0], limitRotTo[1], limitRotTo[2]));
        m_constraint->setAngularLowerLimit(Vector3(limitRotFrom[0], limitRotFrom[1], limitRotFrom[2]));
#endif /* VPVL_COORDINATE_OPENGL */

        for (int i = 0; i < 6; i++) {
            if (i >= 3 || stiffness[i] != 0.0f) {
                m_constraint->enableSpring(i, true);
                m_constraint->setStiffness(i, stiffness[i]);
            }
        }
        internal::copyBytes(reinterpret_cast<uint8_t *>(m_stiffness),
                            reinterpret_cast<const uint8_t *>(stiffness),
                            sizeof(chunk.stiffness));
        m_position.setValue(pos[0], pos[1], pos[2]);
        m_rotation.setValue(rot[0], rot[1], rot[2]);
        m_limitPositionFrom.setValue(limitPosFrom[0], limitPosFrom[1], limitPosFrom[2]);
        m_limitPositionTo.setValue(limitPosTo[0], limitPosTo[1], limitPosTo[2]);
        m_limitRotationFrom.setValue(limitRotFrom[0], limitRotFrom[1], limitRotFrom[2]);
        m_limitRotationTo.setValue(limitRotTo[0], limitRotTo[1], limitRotTo[2]);
        m_bodyA = bodyID1;
        m_bodyB = bodyID2;
    }
#else  /* VPVL_NO_BULLET */
    (void) data;
    (void) bodies;
    (void) offset;
#endif /* VPVL_NO_BULLET */
}

void Constraint::write(uint8_t *data) const
{
    ConstraintChunk chunk;
    copyBytesSafe(chunk.name, m_name, sizeof(chunk.name));
    chunk.bodyIDA = m_bodyA;
    chunk.bodyIDB = m_bodyB;
    chunk.position[0] = m_position.x();
    chunk.position[1] = m_position.y();
    chunk.position[2] = m_position.z();
    chunk.rotation[0] = m_rotation.x();
    chunk.rotation[1] = m_rotation.y();
    chunk.rotation[2] = m_rotation.z();
    chunk.limitPositionFrom[0] = m_limitPositionFrom.x();
    chunk.limitPositionFrom[1] = m_limitPositionFrom.y();
    chunk.limitPositionFrom[2] = m_limitPositionFrom.z();
    chunk.limitPositionTo[0] = m_limitPositionTo.x();
    chunk.limitPositionTo[1] = m_limitPositionTo.y();
    chunk.limitPositionTo[2] = m_limitPositionTo.z();
    chunk.limitRotationFrom[0] = m_limitRotationFrom.x();
    chunk.limitRotationFrom[1] = m_limitRotationFrom.y();
    chunk.limitRotationFrom[2] = m_limitRotationFrom.z();
    chunk.limitRotationTo[0] = m_limitRotationTo.x();
    chunk.limitRotationTo[1] = m_limitRotationTo.y();
    chunk.limitRotationTo[2] = m_limitRotationTo.z();
    internal::copyBytes(reinterpret_cast<uint8_t *>(chunk.stiffness),
                        reinterpret_cast<const uint8_t *>(m_stiffness),
                        sizeof(chunk.stiffness));
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

} /* namespace vpvl */

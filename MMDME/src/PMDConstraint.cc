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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

/* headers */

#include "MMDME/MMDME.h"

namespace MMDAI {

PMDConstraint::PMDConstraint()
    : m_constraint(0),
      m_world(0)
{
}

PMDConstraint::~PMDConstraint()
{
    release();
}

void PMDConstraint::release()
{
    if (m_world != 0) {
       m_world->removeConstraint(m_constraint);
       m_world = 0;
    }
    delete m_constraint;
    m_constraint = 0;
}

bool PMDConstraint::setup(PMDFile_Constraint *c, PMDRigidBody *bodyList, btVector3 *offset)
{
    btTransform tr;
    btMatrix3x3 bm;

    /* reset this constraint */
    release();

    /* get pointer to bodies at both end of this constraint */
    const PMDRigidBody &bodyA = bodyList[c->bodyIDA];
    const PMDRigidBody &bodyB = bodyList[c->bodyIDB];
    btRigidBody *rbA = bodyA.getBody();
    btRigidBody *rbB = bodyB.getBody();

    if (rbA == NULL || rbB == NULL)
        return false;

    /* make global transform of this constraint */
    tr.setIdentity();
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
    btMatrix3x3 rx, ry, rz;
    rx.setEulerZYX(-c->rot[0], 0, 0);
    ry.setEulerZYX(0, -c->rot[1], 0);
    rz.setEulerZYX(0, 0, c->rot[2]);
    bm = ry * rz * rx;
#else
    bm.setEulerZYX(c->rot[0], c->rot[1], c->rot[2]);
#endif
    tr.setBasis(bm);
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
    tr.setOrigin(btVector3(c->pos[0], c->pos[1], -c->pos[2]) + *offset);
#else
    tr.setOrigin(btVector3(c->pos[0], c->pos[1], c->pos[2]) + *offset);
#endif
    /* make local transforms from both rigid bodies */
    const btTransform trA = rbA->getWorldTransform().inverse() * tr;
    const btTransform trB = rbB->getWorldTransform().inverse() * tr;

    /* create constraint */
    m_constraint = new btGeneric6DofSpringConstraint(*rbA, *rbB, trA, trB, true);

#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
    /* set linear translation limits */
    m_constraint->setLinearLowerLimit(btVector3(c->limitPosFrom[0], c->limitPosFrom[1], -c->limitPosTo[2]));
    m_constraint->setLinearUpperLimit(btVector3(c->limitPosTo[0], c->limitPosTo[1], -c->limitPosFrom[2]));

    /* set rotation angle limits */
    m_constraint->setAngularLowerLimit(btVector3(-c->limitRotTo[0], -c->limitRotTo[1], c->limitRotFrom[2]));
    m_constraint->setAngularUpperLimit(btVector3(-c->limitRotFrom[0], -c->limitRotFrom[1], c->limitRotTo[2]));
#else
    /* set linear translation limits */
    m_constraint->setLinearLowerLimit(btVector3(c->limitPosFrom[0], c->limitPosFrom[1], c->limitPosFrom[2]));
    m_constraint->setLinearUpperLimit(btVector3(c->limitPosTo[0], c->limitPosTo[1], c->limitPosTo[2]));

    /* set rotation angle limits */
    m_constraint->setAngularLowerLimit(btVector3(c->limitRotFrom[0], c->limitRotFrom[1], c->limitRotFrom[2]));
    m_constraint->setAngularUpperLimit(btVector3(c->limitRotTo[0], c->limitRotTo[1], c->limitRotTo[2]));
#endif

    /* set spring stiffnesses */
    for (int i = 0; i < 6; i++) {
        if (i >= 3 || c->stiffness[i] != 0.0f) {
            /* allow spring always for rotation and only if stiffness is specified for translation */
            m_constraint->enableSpring(i, true);
            m_constraint->setStiffness(i, c->stiffness[i]);
        }
    }

    MMDAILogDebugSJIS("name=\"%s\" bodyA=\"%s\", bodyB=\"%s\" position=(%.2f, %.2f, %.2f) rotation=(%.2f, %.2f, %.2f)",
                      c->name, bodyA.getName(), bodyB.getName(), c->pos[0], c->pos[1], c->pos[2], c->rot[0], c->rot[1], c->rot[2]);

    return true;
}

void PMDConstraint::joinWorld(btDiscreteDynamicsWorld *btWorld)
{
    assert(m_constraint != 0 && btWorld != 0);

    btWorld->addConstraint(m_constraint);
    m_world = btWorld;
}

} /* namespace */


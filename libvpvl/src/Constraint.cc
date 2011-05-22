#include "vpvl/vpvl.h"

namespace vpvl
{

Constraint::Constraint()
    : m_constraint(0)
{
    memset(m_name, 0, sizeof(m_name));
}

Constraint::~Constraint()
{
    memset(m_name, 0, sizeof(m_name));
    delete m_constraint;
    m_constraint = 0;
}

size_t Constraint::stride(const char * /* data */)
{
    return 20 + (sizeof(uint32_t) * 2) + (sizeof(float) * 24);
}

void Constraint::read(const char *data, const RigidBodyList &bodies, const btVector3 &offset)
{
    char *ptr = const_cast<char *>(data);
    vpvlStringCopySafe(m_name, ptr, sizeof(m_name));
    ptr += sizeof(m_name);
    int32_t bodyID1 = *reinterpret_cast<int32_t *>(ptr);
    ptr += sizeof(uint32_t);
    int32_t bodyID2 = *reinterpret_cast<int32_t *>(ptr);
    ptr += sizeof(uint32_t);
    float pos[3], rot[3], limitPosFrom[3], limitPosTo[3], limitRotFrom[3], limitRotTo[3], stiffness[6];
    vpvlStringGetVector3(ptr, pos);
    vpvlStringGetVector3(ptr, rot);
    vpvlStringGetVector3(ptr, limitPosFrom);
    vpvlStringGetVector3(ptr, limitPosTo);
    vpvlStringGetVector3(ptr, limitRotFrom);
    vpvlStringGetVector3(ptr, limitRotTo);
    for (int i = 0; i < 6; i++) {
        stiffness[i] = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(float);
    }

    int nbodies = bodies.size();
    if (bodyID1 >= 0 && bodyID1 < nbodies &&bodyID2 >= 0 && bodyID2 < nbodies) {
        btTransform transform;
        btMatrix3x3 basis;
        transform.setIdentity();
#ifdef VPVL_COORDINATE_OPENGL
        btMatrix3x3 mx, my, mz;
        mx.setEulerZYX(-rot[0], 0.0f, 0.0f);
        my.setEulerZYX(0.0f, -rot[1], 0.0f);
        mz.setEulerZYX(0.0f, 0.0f, -rot[2]);
        basis = mx * my * mz;
#else
        basis.setEulerZYX(rot[0], rot[1], rot[2]);
#endif
        transform.setBasis(basis);
#ifdef VPVL_COORDINATE_OPENGL
        transform.setOrigin(btVector3(pos[0], pos[1], -pos[2]) * offset);
#else
        transform.setOrigin(btVector3(pos[0], pos[1], pos[2]) * offset);
#endif
        btRigidBody *bodyA = bodies[bodyID1].body(), *bodyB = bodies[bodyID2].body();
        btTransform transformA = bodyA->getWorldTransform().inverse() * transform,
                transformB = bodyB->getWorldTransform().inverse() *transform;
        m_constraint = new btGeneric6DofSpringConstraint(*bodyA, *bodyB, transformA, transformB, true);
#ifdef VPVL_COORDINATE_OPENGL
        m_constraint->setLinearUpperLimit(btVector3(limitPosTo[0], limitPosTo[1], -limitPosTo[2]));
        m_constraint->setLinearLowerLimit(btVector3(limitPosFrom[0], limitPosFrom[1], -limitPosFrom[2]));
        m_constraint->setAngularUpperLimit(btVector3(limitRotTo[0], limitRotTo[1], -limitRotTo[2]));
        m_constraint->setAngularLowerLimit(btVector3(limitRotFrom[0], limitRotFrom[1], -limitRotFrom[2]));
#else
        m_constraint->setLinearUpperLimit(btVector3(limitPosTo[0], limitPosTo[1], limitPosTo[2]));
        m_constraint->setLinearLowerLimit(btVector3(limitPosFrom[0], limitPosFrom[1], limitPosFrom[2]));
        m_constraint->setAngularUpperLimit(btVector3(limitRotTo[0], limitRotTo[1], limitRotTo[2]));
        m_constraint->setAngularLowerLimit(btVector3(limitRotFrom[0], limitRotFrom[1], limitRotFrom[2]));
#endif

        for (int i = 0; i < 6; i++) {
            if (i >= 3 || stiffness[i] != 0.0f) {
                m_constraint->enableSpring(i, true);
                m_constraint->setStiffness(i, stiffness[i]);
            }
        }
    }
}

} /* namespace vpvl */

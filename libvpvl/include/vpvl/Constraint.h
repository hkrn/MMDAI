#ifndef VPVL_CONSTRAINT_H_
#define VPVL_CONSTRAINT_H_

#include <BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h>
#include <LinearMath/btAlignedObjectArray.h>
#include "vpvl/RigidBody.h"

namespace vpvl
{

class Constraint
{
public:
    Constraint();
    ~Constraint();

    static size_t stride(const char *data);

    void read(const char *data, const RigidBodyList &bodies, const btVector3 &offset);

    const char *name() const {
        return m_name;
    }
    btGeneric6DofConstraint *constraint() const {
        return m_constraint;
    }

    void setName(const char *value) {
        stringCopySafe(m_name, value, sizeof(m_name));
    }

private:
    char m_name[20];
    btGeneric6DofSpringConstraint *m_constraint;
};

typedef btAlignedObjectArray<Constraint*> ConstraintList;

} /* namespace vpvl */

#endif

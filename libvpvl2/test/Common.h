#ifndef COMMON_H
#define COMMON_H

#include <QtCore/QtCore>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <tr1/tuple>
#include <vpvl2/Common.h>

#define ASSERT_OR_RETURN(expr) do { AssertionResult r = (expr); if (!r) { return r; } } while (0)

namespace vpvl2 {

class IBone;
class IBoneKeyframe;
class ICameraKeyframe;
class IMaterial;
class IMorphKeyframe;
class IVertex;

std::ostream& operator <<(std::ostream &os, const Vector3 &value);
std::ostream& operator <<(std::ostream &os, const Vector4 &value);
std::ostream& operator <<(std::ostream &os, const Quaternion &value);
std::ostream& operator <<(std::ostream &os, const QuadWord &value);

namespace pmx {
class Bone;
class Joint;
class RigidBody;
class Vertex;
}

}

static const float kIdentity4x4[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};
static const float kEpsilon = 0.000001;

::testing::AssertionResult CompareVector(const vpvl2::Vector3 &expected, const vpvl2::Vector3 &actual);
::testing::AssertionResult CompareVector(const vpvl2::Vector4 &expected, const vpvl2::Vector4 &actual);
::testing::AssertionResult CompareVector(const vpvl2::Quaternion &expected, const vpvl2::Quaternion &actual);
::testing::AssertionResult CompareVector(const vpvl2::QuadWord &actual, const vpvl2::QuadWord &expected);

::testing::AssertionResult CompareBoneInterface(const vpvl2::IBone &expected, const vpvl2::IBone &actual);
::testing::AssertionResult CompareBone(const vpvl2::pmx::Bone &expected, const vpvl2::pmx::Bone &actual);
::testing::AssertionResult CompareJoint(const vpvl2::pmx::Joint &expected,
                                        const vpvl2::pmx::Joint &actual,
                                        const vpvl2::pmx::RigidBody &body,
                                        const vpvl2::pmx::RigidBody &body2);
::testing::AssertionResult CompareMaterialInterface(const vpvl2::IMaterial &expected,
                                                    const vpvl2::IMaterial &actual);
::testing::AssertionResult CompareRigidBody(const vpvl2::pmx::RigidBody &expected,
                                            const vpvl2::pmx::RigidBody &actual,
                                            const vpvl2::IBone &bone);
::testing::AssertionResult CompareVertexInterface(const vpvl2::IVertex &expected, const vpvl2::IVertex &actual);
::testing::AssertionResult CompareVertex(const vpvl2::pmx::Vertex &expected,
                                         const vpvl2::pmx::Vertex &actual,
                                         const vpvl2::Array<vpvl2::pmx::Bone *> &bones);

::testing::AssertionResult CompareBoneKeyframe(const vpvl2::IBoneKeyframe &expected,
                                               const vpvl2::IBoneKeyframe &actual);
::testing::AssertionResult CompareCameraKeyframe(const vpvl2::ICameraKeyframe &expected,
                                                 const vpvl2::ICameraKeyframe &actual);
::testing::AssertionResult CompareMorphKeyframe(const vpvl2::IMorphKeyframe &expected,
                                                const vpvl2::IMorphKeyframe &actual);

void AssertMatrix(const float *expected, const float *actual);

struct ScopedPointerListDeleter {
    template<typename T>
    static inline void cleanup(QList<T *> *list) {
        qDeleteAll(*list);
    }
};

#endif // COMMON_H

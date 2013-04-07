#include "Common.h"
#include "vpvl2/vpvl2.h"
#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::pmx;

namespace vpvl2 {

inline std::ostream& operator <<(std::ostream &os, const Vector3 &value)
{
    return os << "Vector3(x=" << value.x()
              << ", y=" << value.y()
              << ", z=" << value.z()
              << ")";
}

inline std::ostream& operator <<(std::ostream &os, const Vector4 &value)
{
    return os << "Vector4(x=" << value.x()
              << ", y=" << value.y()
              << ", z=" << value.z()
              << ", w=" << value.w()
              << ")";
}

inline std::ostream& operator <<(std::ostream &os, const Quaternion &value)
{
    return os << "Quaternion(x=" << value.x()
              << ", y=" << value.y()
              << ", z=" << value.z()
              << ", w=" << value.w()
              << ")";
}

inline std::ostream& operator <<(std::ostream &os, const QuadWord &value)
{
    return os << "QuadWord(x=" << value.x()
              << ", y=" << value.y()
              << ", z=" << value.z()
              << ", w=" << value.w()
              << ")";
}

}

AssertionResult CompareVector(const Vector3 &expected, const Vector3 &actual)
{
    if (!btEqual(expected.x() - actual.x(), kEpsilon))
        return AssertionFailure() << "X is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.y() - actual.y(), kEpsilon))
        return AssertionFailure() << "Y is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.z() - actual.z(), kEpsilon))
        return AssertionFailure() << "Z is not equal: expected is " << expected << " but actual is " << actual;
    return AssertionSuccess();
}

AssertionResult CompareVector(const Vector4 &expected, const Vector4 &actual)
{
    if (!btEqual(expected.x() - actual.x(), kEpsilon))
        return AssertionFailure() << "X is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.y() - actual.y(), kEpsilon))
        return AssertionFailure() << "Y is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.z() - actual.z(), kEpsilon))
        return AssertionFailure() << "Z is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.w() - actual.w(), kEpsilon))
        return AssertionFailure() << "W is not equal: expected is " << expected << " but actual is " << actual;
    return AssertionSuccess();
}

AssertionResult CompareVector(const Quaternion &expected, const Quaternion &actual)
{
    if (!btEqual(expected.x() - actual.x(), kEpsilon))
        return AssertionFailure() << "X is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.y() - actual.y(), kEpsilon))
        return AssertionFailure() << "Y is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.z() - actual.z(), kEpsilon))
        return AssertionFailure() << "Z is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.w() - actual.w(), kEpsilon))
        return AssertionFailure() << "W is not equal: expected is " << expected << " but actual is " << actual;
    return AssertionSuccess();
}

AssertionResult CompareVector(const QuadWord &actual, const QuadWord &expected)
{
    if (!btEqual(expected.x() - actual.x(), kEpsilon))
        return AssertionFailure() << "X is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.y() - actual.y(), kEpsilon))
        return AssertionFailure() << "Y is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.z() - actual.z(), kEpsilon))
        return AssertionFailure() << "Z is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.w() - actual.w(), kEpsilon))
        return AssertionFailure() << "W is not equal: expected is " << expected << " but actual is " << actual;
    return AssertionSuccess();
}

AssertionResult CompareBoneInterface(const IBone &expected, const IBone &actual)
{
    if (!actual.name()->equals(expected.name())) {
        return AssertionFailure() << "Bone#name is not same: expected=" << expected.name()
                                  << " actual=" << actual.name();
    }
    if (expected.origin() != actual.origin()) {
        return AssertionFailure() << "Bone#origin is not same: expected=" << expected.origin()
                                  << " actual=" << actual.origin();
    }
    const Vector3 &dest = actual.destinationOrigin() - expected.origin();
    const Vector3 &delta = expected.destinationOrigin() - dest;
    if (!btFuzzyZero(delta.x()) || !btFuzzyZero(delta.y()) || !btFuzzyZero(delta.z())) {
        return AssertionFailure() << "Bone#destinationOrigin is not same: expected="
                                  << expected.destinationOrigin()
                                  << " actual=" << actual.destinationOrigin() - expected.origin();
    }
    if (expected.fixedAxis() != actual.fixedAxis()) {
        return AssertionFailure() << "Bone#fixedAxis is not same: expected=" << expected.fixedAxis()
                                  << " actual=" << actual.fixedAxis();
    }
    if (actual.isRotateable() != expected.isRotateable()) {
        return AssertionFailure() << "Bone#isRotateable is not same: expected=" << expected.isRotateable()
                                  << " actual=" << actual.isRotateable();
    }
    if (actual.isMovable() != expected.isMovable()) {
        return AssertionFailure() << "Bone#isMovable is not same: expected=" << expected.isMovable()
                                  << " actual=" << actual.isMovable();
    }
    if (actual.isVisible() != expected.isVisible()) {
        return AssertionFailure() << "Bone#isVisible is not same: expected=" << expected.isVisible()
                                  << " actual=" << actual.isVisible();
    }
    if (actual.isInteractive() != expected.isInteractive()) {
        return AssertionFailure() << "Bone#isInteractive is not same: expected=" << expected.isInteractive()
                                  << " actual=" << actual.isInteractive();
    }
    if (actual.hasInverseKinematics() != expected.hasInverseKinematics()) {
        return AssertionFailure() << "Bone#hasInverseKinematics is not same: expected="
                                  << expected.hasInverseKinematics()
                                  << " actual=" << actual.hasInverseKinematics();
    }
    if (actual.hasFixedAxes() != expected.hasFixedAxes()) {
        return AssertionFailure() << "Bone#hasFixedAxes is not same: expected=" << expected.hasFixedAxes()
                                  << " actual=" << actual.hasFixedAxes();
    }
    if (actual.hasLocalAxes() != expected.hasLocalAxes()) {
        return AssertionFailure() << "Bone#hasLocalAxes is not same: expected=" << expected.hasLocalAxes()
                                  << " actual=" << actual.hasLocalAxes();
    }
    return AssertionSuccess();
}

AssertionResult CompareBone(const Bone &expected, const Bone &actual)
{
    AssertionResult result = CompareBoneInterface(expected, actual);
    if (!result) {
        return result;
    }
    if (!actual.englishName()->equals(expected.englishName())) {
        return AssertionFailure() << "Bone#englishName is not same: expected=" << expected.englishName()
                                  << " actual=" << actual.englishName();
    }
    if (expected.layerIndex() != actual.layerIndex()) {
        return AssertionFailure() << "Bone#layerIndex is not same: expected=" << expected.layerIndex()
                                  << " actual=" << actual.layerIndex();
    }
    if (expected.externalIndex() != actual.externalIndex()) {
        return AssertionFailure() << "Bone#externalIndex is not same: expected=" << expected.layerIndex()
                                  << " actual=" << actual.externalIndex();
    }
    if (actual.hasPositionInherence() != expected.hasPositionInherence()) {
        return AssertionFailure() << "Bone#hasPositionInherence is not same: expected="
                                  << expected.hasPositionInherence()
                                  << " actual=" << actual.hasPositionInherence();
    }
    if (actual.hasRotationInherence() != expected.hasRotationInherence()) {
        return AssertionFailure() << "Bone#hasRotationInherence is not same: expected="
                                  << expected.hasRotationInherence()
                                  << " actual=" << actual.hasRotationInherence();
    }
    if (actual.isTransformedAfterPhysicsSimulation() != expected.isTransformedAfterPhysicsSimulation()) {
        return AssertionFailure() << "Bone#isTransformedAfterPhysicsSimulation is not same: expected="
                                  << expected.isTransformedAfterPhysicsSimulation()
                                  << " actual=" << actual.isTransformedAfterPhysicsSimulation();
    }
    if (actual.isTransformedByExternalParent() != expected.isTransformedByExternalParent()) {
        return AssertionFailure() << "Bone#isTransformedByExternalParent is not same: expected="
                                  << expected.isTransformedByExternalParent()
                                  << " actual=" << actual.isTransformedByExternalParent();
    }
    return AssertionSuccess();
}

AssertionResult CompareJoint(const Joint &expected,
                             const Joint &actual,
                             const RigidBody &body,
                             const RigidBody &body2)
{
    if (!actual.name()->equals(expected.name())) {
        return AssertionFailure() << "Joint#name is not same: expected=" << expected.name()
                                  << " actual=" << actual.name();
    }
    if (!actual.englishName()->equals(expected.englishName())) {
        return AssertionFailure() << "Joint#englishName is not same: expected=" << expected.englishName()
                                  << " actual=" << actual.englishName();
    }
    if (expected.position() != actual.position()) {
        return AssertionFailure() << "Joint#position is not same: expected=" << expected.position()
                                  << " actual=" << actual.position();
    }
    if (expected.rotation() != actual.rotation()) {
        return AssertionFailure() << "Joint#rotation is not same: expected=" << expected.rotation()
                                  << " actual=" << actual.rotation();
    }
    if (expected.positionLowerLimit() != actual.positionLowerLimit()) {
        return AssertionFailure() << "Joint#positionLowerLimit is not same: expected="
                                  << expected.positionLowerLimit()
                                  << " actual=" << actual.positionLowerLimit();
    }
    if (expected.rotationLowerLimit() != actual.rotationLowerLimit()) {
        return AssertionFailure() << "Joint#rotationLowerLimit is not same: expected="
                                  << expected.rotationLowerLimit()
                                  << " actual=" << actual.rotationLowerLimit();
    }
    if (expected.positionUpperLimit() != actual.positionUpperLimit()) {
        return AssertionFailure() << "Joint#positionUpperLimit is not same: expected="
                                  << expected.positionUpperLimit()
                                  << " actual=" << actual.positionUpperLimit();
    }
    if (expected.rotationUpperLimit() != actual.rotationUpperLimit()) {
        return AssertionFailure() << "Joint#rotationUpperLimit is not same: expected="
                                  << expected.rotationUpperLimit()
                                  << " actual=" << actual.rotationUpperLimit();
    }
    if (expected.positionStiffness() != actual.positionStiffness()) {
        return AssertionFailure() << "Joint#positionStiffness is not same: expected="
                                  << expected.positionStiffness()
                                  << " actual=" << actual.positionStiffness();
    }
    if (expected.rotationStiffness() != actual.rotationStiffness()) {
        return AssertionFailure() << "Joint#rotationStiffness is not same: expected="
                                  << expected.rotationStiffness()
                                  << " actual=" << actual.rotationStiffness();
    }
    if (body.index() != actual.rigidBodyIndex1()) {
        return AssertionFailure() << "Joint#rigidBodyIndex1 is not same: expected=" << body.index()
                                  << " actual=" << actual.rigidBodyIndex1();
    }
    if (body2.index() != actual.rigidBodyIndex2()) {
        return AssertionFailure() << "Joint#rigidBodyIndex2 is not same: expected=" << body2.index()
                                  << " actual=" << actual.rigidBodyIndex2();
    }
    return AssertionSuccess();
}

AssertionResult CompareMaterialInterface(const IMaterial &expected, const IMaterial &actual)
{
    if (!actual.name()->equals(expected.name())) {
        return AssertionFailure() << "Material#name is not same: expected=" << expected.name()
                                  << " actual=" << actual.name();
    }
    if (!actual.englishName()->equals(expected.englishName())) {
        return AssertionFailure() << "Material#englishName is not same: expected=" << expected.englishName()
                                  << " actual=" << actual.englishName();
    }
    if (expected.ambient() != actual.ambient()) {
        return AssertionFailure() << "Material#ambient is not same: expected=" << expected.ambient()
                                  << " actual=" << actual.ambient();
    }
    if (expected.diffuse() != actual.diffuse()) {
        return AssertionFailure() << "Material#diffuse is not same: expected=" << expected.diffuse()
                                  << " actual=" << actual.diffuse();
    }
    if (expected.specular() != actual.specular()) {
        return AssertionFailure() << "Material#specular is not same: expected=" << expected.specular()
                                  << " actual=" << actual.specular();
    }
    if (expected.edgeColor() != actual.edgeColor()) {
        return AssertionFailure() << "Material#edgeColor is not same: expected=" << expected.edgeColor()
                                  << " actual=" << actual.edgeColor();
    }
    if (expected.sphereTextureRenderMode() != actual.sphereTextureRenderMode()) {
        return AssertionFailure() << "Material#sphereTextureRenderMode is not same: expected="
                                  << expected.sphereTextureRenderMode()
                                  << " actual=" << actual.sphereTextureRenderMode();
    }
    if (expected.shininess() != actual.shininess()) {
        return AssertionFailure() << "Material#shininess is not same: expected=" << expected.shininess()
                                  << " actual=" << actual.shininess();
    }
    if (expected.edgeSize() != actual.edgeSize()) {
        return AssertionFailure() << "Material#edgeSize is not same: expected=" << expected.edgeSize()
                                  << " actual=" << actual.edgeSize();
    }
    if (expected.textureIndex() != actual.textureIndex()) {
        return AssertionFailure() << "Material#textureIndex is not same: expected=" << expected.textureIndex()
                                  << " actual=" << actual.textureIndex();
    }
    if (expected.sphereTextureIndex() != actual.sphereTextureIndex()) {
        return AssertionFailure() << "Material#sphereTextureIndex is not same: expected="
                                  << expected.sphereTextureIndex()
                                  << " actual=" << actual.sphereTextureIndex();
    }
    if (expected.toonTextureIndex() != actual.toonTextureIndex()) {
        return AssertionFailure() << "Material#toonTextureIndex is not same: expected="
                                  << expected.toonTextureIndex()
                                  << " actual=" << actual.toonTextureIndex();
    }
    if (expected.indexRange().count != actual.indexRange().count) {
        return AssertionFailure() << "count of Material#indexRange is not same: expected="
                                  << expected.indexRange().count
                                  << " actual=" << actual.indexRange().count;
    }
    return AssertionSuccess();
}

AssertionResult CompareRigidBody(const RigidBody &expected, const RigidBody &actual, const IBone &bone)
{
    if (!actual.name()->equals(expected.name())) {
        return AssertionFailure() << "RigidBody#name is not same: expected=" << expected.name()
                                  << " actual=" << actual.name();
    }
    if (!actual.englishName()->equals(expected.englishName())) {
        return AssertionFailure() << "RigidBody#englishName is not same: expected=" << expected.englishName()
                                  << " actual=" << actual.englishName();
    }
    if (expected.angularDamping() != actual.angularDamping()) {
        return AssertionFailure() << "RigidBody#angularDamping is not same: expected="
                                  << expected.angularDamping()
                                  << " actual=" << actual.angularDamping();
    }
    if (expected.collisionGroupID() != actual.collisionGroupID()) {
        return AssertionFailure() << "RigidBody#collisionGroupID is not same: expected="
                                  << expected.collisionGroupID()
                                  << " actual=" << actual.collisionGroupID();
    }
    if (expected.collisionGroupMask() != actual.collisionGroupMask()) {
        return AssertionFailure() << "RigidBody#collisionGroupMask is not same: expected="
                                  << expected.collisionGroupMask()
                                  << " actual=" << actual.collisionGroupMask();
    }
    if (expected.friction() != actual.friction()) {
        return AssertionFailure() << "RigidBody#friction is not same: expected=" << expected.friction()
                                  << " actual=" << actual.friction();
    }
    if (expected.linearDamping() != actual.linearDamping()) {
        return AssertionFailure() << "RigidBody#linearDamping is not same: expected="
                                  << expected.linearDamping()
                                  << " actual=" << actual.linearDamping();
    }
    if (expected.mass() != actual.mass()) {
        return AssertionFailure() << "RigidBody#mass is not same: expected=" << expected.mass()
                                  << " actual=" << actual.mass();
    }
    if (expected.position() != actual.position()) {
        return AssertionFailure() << "RigidBody#position is not same: expected=" << expected.position()
                                  << " actual=" << actual.position();
    }
    if (expected.restitution() != actual.restitution()) {
        return AssertionFailure() << "RigidBody#restitution is not same: expected=" << expected.restitution()
                                  << " actual=" << actual.restitution();
    }
    if (expected.rotation() != actual.rotation()) {
        return AssertionFailure() << "RigidBody#rotation is not same: expected=" << expected.rotation()
                                  << " actual=" << actual.rotation();
    }
    if (expected.size() != actual.size()) {
        return AssertionFailure() << "RigidBody#size is not same: expected=" << expected.size()
                                  << " actual=" << actual.size();
    }
    if (bone.index() != actual.boneIndex()) {
        return AssertionFailure() << "RigidBody#boneIndex is not same: expected=" << bone.index()
                                  << " actual=" << actual.boneIndex();
    }
    return AssertionSuccess();
}

AssertionResult CompareVertexInterface(const IVertex &expected, const IVertex &actual)
{
    if (expected.origin() != actual.origin()) {
        return AssertionFailure() << "Vertex#origin is not same: expected="
                                  << expected.origin() << " actual=" << actual.origin();
    }
    if (expected.normal() != actual.normal()) {
        return AssertionFailure() << "Vertex#normal is not same: expected="
                                  << expected.normal() << " actual=" << actual.normal();
    }
    if (expected.textureCoord() != actual.textureCoord()) {
        return AssertionFailure() << "Vertex#textureCoord is not same: expected="
                                  << expected.textureCoord() << " actual=" << actual.textureCoord();
    }
    if (expected.uv(0) != actual.uv(0)) {
        return AssertionFailure() << "Vertex#uv(0) is not same: expected="
                                  << expected.uv(0) << " actual=" << actual.uv(0);
    }
    if (expected.uv(1) != kZeroV4) {
        return AssertionFailure() << "Vertex#uv(1) is not zero: actual=" << actual.uv(1);
    }
    if (expected.type() != actual.type()) {
        return AssertionFailure() << "Vertex#type is not same: expected="
                                  << expected.type() << " actual=" << actual.type();
    }
    if (expected.edgeSize() != actual.edgeSize()) {
        return AssertionFailure() << "Vertex#edgeSize is not same: expected="
                                  << expected.edgeSize() << " actual=" << actual.type();
    }
    return AssertionSuccess();
}

AssertionResult CompareVertex(const Vertex &expected, const Vertex &actual, const Array<Bone *> &bones)
{
    AssertionResult result = CompareVertexInterface(expected, actual);
    if (!result) {
        return result;
    }
    if (expected.type() == Vertex::kSdef) {
        if (expected.sdefC() != actual.sdefC()) {
            return AssertionFailure() << "Vertex#sdefC is not same: expected="
                                      << expected.sdefC() << " actual=" << actual.sdefC();
        }
        if (expected.sdefR0() != actual.sdefR0()) {
            return AssertionFailure() << "Vertex#sdefR0 is not same: expected="
                                      << expected.sdefR0() << " actual=" << actual.sdefR0();
        }
        if (expected.sdefR1() != actual.sdefR1()) {
            return AssertionFailure() << "Vertex#sdefR1 is not same: expected="
                                      << expected.sdefR1() << " actual=" << actual.sdefR1();
        }
    }
    else {
        if (actual.sdefC() != kZeroV3) {
            return AssertionFailure() << "Vertex#sdefC is not zero: " << actual.sdefC();
        }
        if (actual.sdefR0() != kZeroV3)
            return AssertionFailure() << "Vertex#sdefR0 is not zero: " << actual.sdefR0();
        if (actual.sdefR1() != kZeroV3)
            return AssertionFailure() << "Vertex#sdefR1 is not zero: " << actual.sdefR1();
    }
    Array<Vertex *> vertices;
    vertices.append(const_cast<Vertex *>(&expected));
    Vertex::loadVertices(vertices, bones);
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        if (expected.bone(i) != bones[i]) {
            return AssertionFailure() << "Vertex#bone(i) is not same: expected=" << expected.bone(i)
                                      << " actual=" << bones[i] << " index=" << i;
        }
        if (bones[i]->index() == -1) {
            return AssertionFailure() << "index of Vertex#bone(i) is null: index=" << i;
        }
        if (nbones == 4) {
            if (actual.weight(i) != (0.2f + 0.1f * i)) {
                return AssertionFailure() << "Vertex#weight(i) is not same: actual="
                                          << actual.weight(i) << " index=" << i;
            }
        }
    }
    if (nbones == 2) {
        if (actual.weight(0) != 0.2f) {
            return AssertionFailure() << "Vertex#weight(0) is not 0.2f: actual=" << actual.weight(0);
        }
    }
    return AssertionSuccess();
}

AssertionResult CompareBoneKeyframe(const IBoneKeyframe &expected, const IBoneKeyframe &actual)
{
    if (expected.timeIndex() != actual.timeIndex()) {
        return AssertionFailure() << "IBoneKeyframe#timeIndex is not same: expected="
                                  << expected.timeIndex() << " actual=" << actual.timeIndex();
    }
    if (expected.layerIndex() != actual.layerIndex()) {
        return AssertionFailure() << "IBoneKeyframe#layerIndex is not same: expected="
                                  << expected.layerIndex() << " actual=" << actual.layerIndex();
    }
    if (expected.name() && !expected.name()->equals(actual.name())) {
        return AssertionFailure() << "IBoneKeyframe#name is not same: expected="
                                  << expected.name() << " actual=" << actual.name();
    }
    if (expected.localTranslation() != actual.localTranslation()) {
        return AssertionFailure() << "IBoneKeyframe#localPosition is not same: expected="
                                  << expected.localTranslation() << " actual=" << actual.localTranslation();
    }
    if (expected.localRotation() != actual.localRotation()) {
        return AssertionFailure() << "IBoneKeyframe#localRotation is not same: expected="
                                  << expected.localRotation() << " actual=" << actual.localRotation();
    }
    Quaternion eq, aq;
    for (int i = 0; i < IBoneKeyframe::kMaxBoneInterpolationType; i++) {
        IBoneKeyframe::InterpolationType index = static_cast<IBoneKeyframe::InterpolationType>(i);
        expected.getInterpolationParameter(index, eq);
        actual.getInterpolationParameter(index, aq);
        if (eq != aq) {
            return AssertionFailure() << "IBoneKeyframe#getInterpolation(i, q) is not same: expected="
                                      << eq << " actual=" << aq << " index=" << index;
        }
    }
    return AssertionSuccess();
}

AssertionResult CompareCameraKeyframe(const ICameraKeyframe &expected, const ICameraKeyframe &actual)
{
    if (expected.timeIndex() != actual.timeIndex()) {
        return AssertionFailure() << "ICameraKeyframe#timeIndex is not same: expected="
                                  << expected.timeIndex() << " actual=" << actual.timeIndex();
    }
    if (expected.layerIndex() != actual.layerIndex()) {
        return AssertionFailure() << "ICameraKeyframe#layerIndex is not same: expected="
                                  << expected.layerIndex() << " actual=" << actual.layerIndex();
    }
    if (expected.lookAt() != actual.lookAt()) {
        return AssertionFailure() << "ICameraKeyframe#lookAt is not same: expected="
                                  << expected.lookAt() << " actual=" << actual.lookAt();
    }
    if (expected.distance() != actual.distance()) {
        return AssertionFailure() << "ICameraKeyframe#distance is not same: expected="
                                  << expected.distance() << " actual=" << actual.distance();
    }
    if (expected.fov() != actual.fov()) {
        return AssertionFailure() << "ICameraKeyframe#fov is not same: expected="
                                  << expected.fov() << " actual=" << actual.fov();
    }
    if (expected.isPerspective() != actual.isPerspective()) {
        return AssertionFailure() << "ICameraKeyframe#isPerspective is not same: expected="
                                  << expected.isPerspective() << " actual=" << actual.isPerspective();
    }
    Quaternion eq, aq;
    for (int i = 2; i < ICameraKeyframe::kCameraMaxInterpolationType; i++) { /* skip kX and kY because of MVD spec */
        ICameraKeyframe::InterpolationType index = static_cast<ICameraKeyframe::InterpolationType>(i);
        expected.getInterpolationParameter(index, eq);
        actual.getInterpolationParameter(index, aq);
        if (eq != aq) {
            return AssertionFailure() << "ICameraKeyframe#getInterpolation(i, q) is not same: expected="
                                      << eq << " actual=" << aq << " index=" << index;
        }
    }
    return AssertionSuccess();
}

AssertionResult CompareMorphKeyframe(const IMorphKeyframe &expected, const IMorphKeyframe &actual)
{
    if (expected.timeIndex() != actual.timeIndex()) {
        return AssertionFailure() << "IMorphKeyframe#timeIndex is not same: expected="
                                  << expected.timeIndex() << " actual=" << actual.timeIndex();
    }
    if (expected.layerIndex() != actual.layerIndex()) {
        return AssertionFailure() << "IMorphKeyframe#layerIndex is not same: expected="
                                  << expected.layerIndex() << " actual=" << actual.layerIndex();
    }
    if (expected.name() && !expected.name()->equals(actual.name())) {
        return AssertionFailure() << "IMorphKeyframe#name is not same: expected="
                                  << expected.name() << " actual=" << actual.name();
    }
    if (expected.weight() != actual.weight()) {
        return AssertionFailure() << "IMorphKeyframe#weight is not same: expected="
                                  << expected.weight() << " actual=" << actual.weight();
    }
    return AssertionSuccess();
}

void AssertMatrix(const float *expected, const float *actual)
{
    for (int i = 0; i < 16; i++) {
        ASSERT_FLOAT_EQ(expected[i], actual[i]) << "matrix value differs "
                                                << expected[i]
                                                << " but "
                                                << actual[i]
                                                << " at index "
                                                << i;
    }
}

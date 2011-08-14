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

#include <btBulletDynamicsCommon.h>

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

const float Scene::kFrustumNear = 0.5f;
const float Scene::kFrustumFar = 8000.0f;
const float Scene::kMinMoveDiff = 0.000001f;
const float Scene::kMoveSpeedRate = 0.9f;
const float Scene::kMinSpinDiff = 0.000001f;
const float Scene::kSpinSpeedRate = 0.9f;
const float Scene::kMinDistanceDiff = 0.1f;
const float Scene::kDistanceSpeedRate = 0.9f;
const float Scene::kMinFovyDiff = 0.01f;
const float Scene::kFovySpeedRate = 0.9f;

class SceneModelDistancePredication
{
public:
    SceneModelDistancePredication(const btTransform &transform)
        : m_transform(transform) {
    }
    bool operator()(const PMDModel *left, const PMDModel *right) {
        const btVector3 positionLeft = m_transform * Bone::centerBone(&left->bones())->localTransform().getOrigin();
        const btVector3 positionRight = m_transform * Bone::centerBone(&right->bones())->localTransform().getOrigin();
        return positionLeft.z() < positionRight.z();
    }
private:
    const btTransform m_transform;
};

Scene::Scene(int width, int height, int fps)
    : m_world(0),
      m_cameraMotion(0),
      m_currentRotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_rotation(m_currentRotation),
      m_viewMoveRotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_lightColor(0.0f, 0.0f, 0.0f, 1.0f),
      m_lightDirection(0.0f, 0.0f, 0.0f, 0.0f),
      m_currentPosition(0.0f, 10.0f, 0.0f),
      m_position(m_currentPosition),
      m_viewMovePosition(m_currentPosition),
      m_angle(0.0f, 0.0f, 0.0f),
      m_currentDistance(0.0f, 0.0f, 100.0f),
      m_viewMoveDistance(m_currentDistance),
      m_distance(m_currentDistance.z()),
      m_currentFovy(16.0f),
      m_fovy(m_currentFovy),
      m_viewMoveFovy(m_currentFovy),
      m_viewMoveTime(-1),
      m_preferredFPS(fps),
      m_width(width),
      m_height(height)
{
    updateProjectionMatrix();
    updateModelViewMatrix();
}

Scene::~Scene()
{
    internal::zerofill(m_projection, sizeof(m_projection));
    setWorld(0);
    m_cameraMotion = 0;
    m_currentRotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_viewMoveRotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_lightColor.setZero();
    m_lightDirection.setZero();
    m_currentPosition.setZero();
    m_position.setZero();
    m_viewMovePosition.setZero();
    m_angle.setZero();
    m_currentDistance.setZero();
    m_viewMoveDistance.setZero();
    m_modelview.setIdentity();
    m_distance = 0.0f;
    m_currentFovy = 0.0f;
    m_fovy = 0.0f;
    m_viewMoveFovy = 0.0f;
    m_viewMoveTime = -1;
    m_preferredFPS = 0;
    m_width = 0;
    m_height = 0;
}

void Scene::addModel(PMDModel *model)
{
    m_models.add(model);
    sortRenderingOrder();
    model->setLightDirection(m_lightDirection);
    model->joinWorld(m_world);
}

PMDModel **Scene::getRenderingOrder(size_t &size)
{
    size = m_models.count();
    return &m_models[0];
}

void Scene::getModelViewMatrix(float matrix[]) const
{
    m_modelview.getOpenGLMatrix(matrix);
}

void Scene::getProjectionMatrix(float matrix[]) const
{
    memcpy(matrix, m_projection, sizeof(m_projection));
}

void Scene::removeModel(PMDModel *model)
{
    m_models.remove(model);
    model->leaveWorld(m_world);
    sortRenderingOrder();
}

void Scene::resetCamera()
{
    setCameraPerspective(btVector3(0.0f, 10.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f), 16.0f, 100.0f);
}

void Scene::seek(float frameIndex)
{
    sortRenderingOrder();
    const uint32_t nModels = m_models.count();
    // Updating model
    for (uint32_t i = 0; i < nModels; i++) {
        PMDModel *model = m_models[i];
        model->updateRootBone();
        model->seekMotion(frameIndex);
        model->updateSkins();
    }
    // Updating camera motion
    if (m_cameraMotion) {
        CameraAnimation *camera = m_cameraMotion->mutableCameraAnimation();
        camera->seek(frameIndex);
        m_position = camera->position();
        m_angle = camera->angle();
        m_distance = camera->distance();
        m_fovy = camera->fovy();
        updateRotationFromAngle();
    }
}

void Scene::setCameraPerspective(const btVector3 &position, const btVector3 &angle, float fovy, float distance)
{
    if (!m_cameraMotion) {
        m_position = position;
        m_angle = angle;
        m_fovy = fovy;
        m_distance = distance;
        updateRotationFromAngle();
    }
}

void Scene::setCameraMotion(VMDMotion *motion)
{
    m_cameraMotion = motion;
}

void Scene::setLight(const btVector4 &color, const btVector4 &direction)
{
    m_lightColor = color;
    m_lightDirection = direction;
    const uint32_t nModels = m_models.count();
    for (uint32_t i = 0; i < nModels; i++) {
        PMDModel *model = m_models[i];
        model->setLightDirection(direction);
    }
}

void Scene::setViewMove(int viewMoveTime)
{
    if (viewMoveTime) {
        m_viewMovePosition = m_currentPosition;
        m_viewMoveRotation = m_currentRotation;
        m_viewMoveDistance = m_currentDistance;
        m_viewMoveFovy = m_currentFovy;
    }
    m_viewMoveTime = viewMoveTime;
}

void Scene::setWorld(::btDiscreteDynamicsWorld *world)
{
    const uint32_t nModels = m_models.count();
    // Remove rigid bodies and constraints from the current world
    // before setting the new world
    if (m_world) {
        for (uint32_t i = 0; i < nModels; i++) {
            PMDModel *model = m_models[i];
            model->leaveWorld(m_world);
        }
    }
    if (world) {
        for (uint32_t i = 0; i < nModels; i++) {
            PMDModel *model = m_models[i];
            model->joinWorld(world);
        }
    }
    m_world = world;
}

void Scene::update(float deltaFrame)
{
    sortRenderingOrder();
    const uint32_t nModels = m_models.count();
    // Updating model
    for (uint32_t i = 0; i < nModels; i++) {
        PMDModel *model = m_models[i];
        model->updateRootBone();
        model->updateMotion(deltaFrame);
        model->updateSkins();
    }
    // Updating world simulation
    if (m_world) {
        btScalar sec = deltaFrame / kFPS;
        if (sec > 1.0f)
            m_world->stepSimulation(sec, 1, sec);
        else
            m_world->stepSimulation(sec, m_preferredFPS, 1.0f / m_preferredFPS);
    }
    // Updating camera motion
    if (m_cameraMotion) {
        bool reached = false;
        CameraAnimation *camera = m_cameraMotion->mutableCameraAnimation();
        camera->advance(deltaFrame, reached);
        m_position = camera->position();
        m_angle = camera->angle();
        m_distance = camera->distance();
        m_fovy = camera->fovy();
        updateRotationFromAngle();
        if (reached)
            m_cameraMotion = 0;
    }
}

void Scene::updateModelView(int ellapsedTimeForMove)
{
    if (updateDistance(ellapsedTimeForMove) || m_currentRotation != m_rotation || m_currentPosition != m_position) {
        if (m_viewMoveTime == 0 || m_cameraMotion) {
            m_currentPosition = m_position;
            m_currentRotation = m_rotation;
        }
        else if (m_viewMoveTime > 0) {
            if (ellapsedTimeForMove >= m_viewMoveTime) {
                m_currentPosition = m_position;
                m_currentRotation = m_rotation;
            }
            else {
                const float moveTime = static_cast<float>(m_viewMoveTime);
                m_currentPosition = m_viewMovePosition.lerp(m_position, ellapsedTimeForMove / moveTime);
                m_currentRotation = m_viewMoveRotation.slerp(m_rotation, ellapsedTimeForMove / moveTime);
            }
        }
        else {
            const btVector3 position = m_position - m_currentPosition;
            const btQuaternion rotation = m_rotation - m_currentRotation;
            if (position.length2() > kMinMoveDiff) {
                /* current * 0.9 + target * 0.1 */
                m_currentPosition = m_currentPosition.lerp(m_position, 1.0f - kMoveSpeedRate);
            }
            else {
                m_currentPosition = m_position;
            }
            if (rotation.length2() > kMinSpinDiff) {
                /* current * 0.9 + target * 0.1 */
                m_currentRotation = m_currentRotation.slerp(m_rotation, 1.0f - kSpinSpeedRate);
            }
            else {
                m_currentRotation = m_rotation;
            }
        }
        updateModelViewMatrix();
    }
}

void Scene::updateProjection(int ellapsedTimeForMove)
{
    if (updateFovy(ellapsedTimeForMove))
        updateProjectionMatrix();
}

void Scene::sortRenderingOrder()
{
    m_models.sort(SceneModelDistancePredication(m_modelview));
}

void Scene::updateModelViewMatrix()
{
    m_modelview.setIdentity();
    m_modelview.setRotation(m_currentRotation);
    m_modelview.setOrigin(m_modelview * (-m_currentPosition) - m_currentDistance);
}

void Scene::updateProjectionMatrix()
{
    const float aspect = static_cast<float>(m_width) / m_height;
    // borrowed code from http://www.geeks3d.com/20090729/howto-perspective-projection-matrix-in-opengl/
    static const float kPIOver360 = kPI / 360.0f;
    const float xymax = kFrustumNear * tanf(m_currentFovy * kPIOver360);
    const float xmin = -xymax;
    const float ymin = -xymax;
    const float width = xymax - xmin;
    const float height = xymax - ymin;
    const float depth = kFrustumFar - kFrustumNear;
    const float q = -(kFrustumFar + kFrustumNear) / depth;
    const float qn = -2 * (kFrustumFar * kFrustumNear) / depth;
    const float w = (2 * kFrustumNear / width) / aspect;
    const float h = 2 * kFrustumNear / height;
    const float matrix[16] = {
        w, 0.0f, 0.0f, 0.0f,
        0.0f, h, 0.0f, 0.0f,
        0.0f, 0.0f, q, -1.0f,
        0.0f, 0.0f, qn, 0.0f
    };
    memcpy(m_projection, matrix, sizeof(matrix));
}

void Scene::updateRotationFromAngle()
{
    static const btVector3 x(1.0f, 0.0f, 0.0f), y(0.0f, 1.0f, 0.0f), z(0.0f, 0.0f, 1.0f);
    btQuaternion rz(z, radian(m_angle.z())), rx(x, radian(m_angle.x())), ry(y, radian(m_angle.y()));
    m_rotation = rz * rx * ry;
}

bool Scene::updateDistance(int ellapsedTimeForMove)
{
    const float distance = m_currentDistance.z();
    if (distance != m_distance) {
        if (m_viewMoveTime == 0 || m_cameraMotion) {
            m_currentDistance.setZ(m_distance);
        }
        else if (m_viewMoveTime > 0) {
            if (ellapsedTimeForMove >= m_viewMoveTime) {
                m_currentDistance.setZ(m_distance);
            }
            else {
                const float viewMoveDistance = m_viewMoveDistance.z();
                m_currentDistance.setZ(viewMoveDistance + (m_distance - viewMoveDistance) * ellapsedTimeForMove / m_viewMoveTime);
            }
        }
        else {
            const float diff = fabsf(distance - m_distance);
            if (diff < kMinDistanceDiff)
                m_currentDistance.setZ(m_distance);
            else
                m_currentDistance.setZ(internal::lerp(m_distance, distance, kDistanceSpeedRate));
        }
        return true;
    }
    return false;
}

bool Scene::updateFovy(int ellapsedTimeForMove)
{
    if (m_currentFovy != m_fovy) {
        if (m_viewMoveTime == 0 || m_cameraMotion) {
            m_currentFovy = m_fovy;
        }
        else if (m_viewMoveTime > 0) {
            if (ellapsedTimeForMove >= m_viewMoveTime)
                m_currentFovy = m_fovy;
            else
                m_currentFovy = m_viewMoveFovy + (m_fovy - m_viewMoveFovy) * ellapsedTimeForMove / m_viewMoveTime;
        }
        else {
            const float diff = fabsf(m_currentFovy - m_fovy);
            if (diff < kMinFovyDiff)
                m_currentFovy = m_fovy;
            else
                m_currentFovy = internal::lerp(m_fovy, m_currentFovy, kFovySpeedRate);
        }
        return true;
    }
    return false;
}

}

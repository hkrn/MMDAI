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
#include <btBulletDynamicsCommon.h>
#else
VPVL_DECLARE_HANDLE(btDiscreteDynamicsWorld)
#endif

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
    SceneModelDistancePredication(const Transform &transform)
        : m_transform(transform) {
    }
    bool operator()(const PMDModel *left, const PMDModel *right) {
        const Vector3 &positionLeft = m_transform * Bone::centerBone(&left->bones())->localTransform().getOrigin();
        const Vector3 &positionRight = m_transform * Bone::centerBone(&right->bones())->localTransform().getOrigin();
        return positionLeft.z() < positionRight.z();
    }
private:
    const Transform m_transform;
};

Scene::Scene(int width, int height, int fps)
    : m_world(0),
      m_cameraMotion(0),
      m_currentRotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_rotation(m_currentRotation),
      m_viewMoveRotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_lightColor(1.0f, 1.0f, 1.0f, 1.0f),
      m_lightPosition(0.5f, 1.0f, 0.5f),
      m_lightAmbient(1.0f, 1.0f, 1.0f, 1.0f),
      m_lightDiffuse(1.0f, 1.0f, 1.0f, 1.0f),
      m_lightSpecular(1.0f, 1.0f, 1.0f, 1.0f),
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
      m_lightIntensity(0.7f),
      m_viewMoveTime(-1),
      m_preferredFPS(fps),
      m_width(width),
      m_height(height)
{
    m_lightColor.setW(1.0f);
    m_lightAmbient.setW(m_lightAmbient.x());
    m_lightDiffuse.setW(0.0f);
    m_lightSpecular.setW(m_lightSpecular.x());
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
    m_lightPosition.setZero();
    m_lightAmbient.setZero();
    m_lightDiffuse.setZero();
    m_lightSpecular.setZero();
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
    m_lightIntensity = 0.0f;
    m_viewMoveTime = -1;
    m_preferredFPS = 0;
    m_width = 0;
    m_height = 0;
}

void Scene::addModel(PMDModel *model)
{
    m_models.add(model);
    sortRenderingOrder();
    model->setLightPosition(m_lightPosition);
    model->joinWorld(m_world);
    model->setVisible(true);
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

void Scene::getNormalMatrix(float matrix[]) const
{
    float v[12];
    m_modelview.getBasis().inverse().transpose().getOpenGLSubMatrix(v);
    matrix[0] = v[0]; matrix[1] = v[1]; matrix[2] = v[2];
    matrix[3] = v[4]; matrix[4] = v[5]; matrix[5] = v[6];
    matrix[6] = v[8]; matrix[7] = v[9]; matrix[8] = v[10];
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
    setCameraPerspective(Vector3(0.0f, 10.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), 16.0f, 100.0f);
}

void Scene::seekMotion(float frameIndex)
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
        setCameraPerspective(camera);
    }
}

void Scene::setCameraPerspective(CameraAnimation *camera)
{
    setCameraPerspective(camera->position(), camera->angle(), camera->fovy(), camera->distance());
}

void Scene::setCameraPerspective(const Vector3 &position, const Vector3 &angle, float fovy, float distance)
{
    m_position = position;
    m_angle = angle;
    m_fovy = fovy;
    m_distance = distance;
    updateRotationFromAngle();
}

void Scene::setCameraMotion(VMDMotion *motion)
{
    m_cameraMotion = motion;
}

void Scene::setLightComponent(const Vector4 &ambient, const Vector4 &diffuse, const Vector4 &specular)
{
    m_lightAmbient = ambient;
    m_lightDiffuse = diffuse;
    m_lightSpecular = specular;
}

void Scene::setLightSource(const Vector4 &color, const Vector3 &position)
{
    m_lightColor = color;
    m_lightPosition = position;
    const uint32_t nModels = m_models.count();
    for (uint32_t i = 0; i < nModels; i++) {
        PMDModel *model = m_models[i];
        model->setLightPosition(position);
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

void Scene::setWorld(btDiscreteDynamicsWorld *world)
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

void Scene::advanceMotion(float deltaFrame)
{
    sortRenderingOrder();
    const uint32_t nModels = m_models.count();
    // Updating model
    for (uint32_t i = 0; i < nModels; i++) {
        PMDModel *model = m_models[i];
        model->updateRootBone();
        model->advanceMotion(deltaFrame);
        model->updateSkins();
    }
    // Updating world simulation
#ifndef VPVL_NO_BULLET
    if (m_world) {
        Scalar sec = deltaFrame / kFPS;
        if (sec > 1.0f)
            m_world->stepSimulation(sec, 1, sec);
        else
            m_world->stepSimulation(sec, m_preferredFPS, 1.0f / m_preferredFPS);
    }
#endif /* VPVL_NO_BULLET */
    // Updating camera motion
    if (m_cameraMotion) {
        CameraAnimation *camera = m_cameraMotion->mutableCameraAnimation();
        camera->advance(deltaFrame);
        setCameraPerspective(camera);
    }
}

void Scene::resetMotion()
{
    sortRenderingOrder();
    const uint32_t nModels = m_models.count();
    // Updating model
    for (uint32_t i = 0; i < nModels; i++) {
        PMDModel *model = m_models[i];
        model->updateRootBone();
        model->resetMotion();
        model->updateSkins();
    }
    // Updating camera motion
    if (m_cameraMotion) {
        CameraAnimation *camera = m_cameraMotion->mutableCameraAnimation();
        camera->seek(0.0f);
        camera->reset();
        setCameraPerspective(camera);
    }
}

bool Scene::isMotionReached(float atEnd)
{
    const uint32_t nModels = m_models.count();
    bool ret = true;
    for (uint32_t i = 0; i < nModels; i++)
        ret = ret && m_models[i]->isMotionReached(atEnd);
    return ret;
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
            const Vector3 position = m_position - m_currentPosition;
            const Quaternion rotation = m_rotation - m_currentRotation;
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
    static const Vector3 x(1.0f, 0.0f, 0.0f), y(0.0f, 1.0f, 0.0f), z(0.0f, 0.0f, 1.0f);
    Quaternion rz(z, radian(m_angle.z())), rx(x, radian(m_angle.x())), ry(y, radian(m_angle.y()));
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

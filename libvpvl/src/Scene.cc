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
const float Scene::kFrustumFar = 10000.0f;

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
      m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_lightColor(0.6f, 0.6f, 0.6f, 1.0f),
      m_lightPosition(-0.5f, 1.0f, 0.5f),
      m_position(0.0f, 10.0f, 0.0f),
      m_angle(0.0f, 0.0f, 0.0f),
      m_distance(100.0f),
      m_fovy(16.0f),
      m_preferredFPS(fps),
      m_width(width),
      m_height(height),
      m_enableSoftwareSkinning(true)
{
    m_lightColor.setW(1.0f);
    updateProjection();
    updateModelView();
}

Scene::~Scene()
{
    internal::zerofill(m_projection, sizeof(m_projection));
    setWorld(0);
    m_cameraMotion = 0;
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_lightColor.setZero();
    m_lightPosition.setZero();
    m_position.setZero();
    m_angle.setZero();
    m_modelview.setIdentity();
    m_distance = 0.0f;
    m_fovy = 0.0f;
    m_preferredFPS = 0;
    m_width = 0;
    m_height = 0;
    m_enableSoftwareSkinning = false;
}

void Scene::addModel(PMDModel *model)
{
    m_models.add(model);
    sortRenderingOrder();
    model->setLightPosition(m_lightPosition);
    model->joinWorld(m_world);
    model->setSoftwareSkinningEnable(m_enableSoftwareSkinning);
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
    const int nmodels = m_models.count();
    // Updating model
    for (int i = 0; i < nmodels; i++) {
        PMDModel *model = m_models[i];
        model->updateRootBone();
        model->seekMotion(frameIndex);
        model->updateSkins();
    }
    // Updating camera motion
    if (m_cameraMotion) {
        CameraAnimation *camera = m_cameraMotion->mutableCameraAnimation();
        if (camera->countKeyFrames() > 0) {
            camera->seek(frameIndex);
            setCameraPerspective(camera);
        }
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

void Scene::setLightSource(const Vector4 &color, const Vector3 &position)
{
    m_lightColor = color;
    m_lightPosition.setValue(position.x(), position.y(), position.z());
    const int nmodels = m_models.count();
    for (int i = 0; i < nmodels; i++) {
        PMDModel *model = m_models[i];
        model->setLightPosition(position);
    }
}

void Scene::setSoftwareSkinningEnable(bool value)
{
    const int nmodels = m_models.count();
    for (int i = 0; i < nmodels; i++) {
        PMDModel *model = m_models[i];
        model->setSoftwareSkinningEnable(value);
    }
    m_enableSoftwareSkinning = value;
}

void Scene::setWorld(btDiscreteDynamicsWorld *world)
{
    const int nmodels = m_models.count();
    // Remove rigid bodies and constraints from the current world
    // before setting the new world
    if (m_world) {
        for (int i = 0; i < nmodels; i++) {
            PMDModel *model = m_models[i];
            model->leaveWorld(m_world);
        }
    }
    if (world) {
        for (int i = 0; i < nmodels; i++) {
            PMDModel *model = m_models[i];
            model->joinWorld(world);
        }
    }
    m_world = world;
}

void Scene::advanceMotion(float deltaFrame)
{
    sortRenderingOrder();
    const int nmodels = m_models.count();
    // Updating model
    for (int i = 0; i < nmodels; i++) {
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
        if (camera->countKeyFrames() > 0) {
            camera->advance(deltaFrame);
            setCameraPerspective(camera);
        }
    }
}

void Scene::resetMotion()
{
    sortRenderingOrder();
    const int nmodels = m_models.count();
    // Updating model
    for (int i = 0; i < nmodels; i++) {
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

float Scene::maxFrameIndex() const
{
    const int nmodels = m_models.count();
    float max = 0.0f;
    for (int i = 0; i < nmodels; i++)
        btSetMax(max, m_models[i]->maxFrameIndex());
    return max;
}

bool Scene::isMotionFinished() const
{
    return isMotionReachedTo(maxFrameIndex());
}

bool Scene::isMotionReachedTo(float atEnd) const
{
    const int nmodels = m_models.count();
    bool ret = true;
    for (int i = 0; i < nmodels; i++)
        ret = ret && m_models[i]->isMotionReachedTo(atEnd);
    return ret;
}

void Scene::sortRenderingOrder()
{
    m_models.sort(SceneModelDistancePredication(m_modelview));
}

void Scene::updateModelView()
{
    m_modelview.setIdentity();
    m_modelview.setRotation(m_rotation);
    m_modelview.setOrigin(m_modelview * (-m_position) - Vector3(0, 0, m_distance));
}

void Scene::updateProjection()
{
    const float aspect = static_cast<float>(m_width) / m_height;
    // borrowed code from http://www.geeks3d.com/20090729/howto-perspective-projection-matrix-in-opengl/
    static const float kPIOver360 = kPI / 360.0f;
    const float xymax = kFrustumNear * tanf(m_fovy * kPIOver360);
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

}

/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/asset/Model.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/vmd/Motion.h"
#include "vpvl2/gl2/AssetRenderEngine.h"
#include "vpvl2/gl2/PMDRenderEngine.h"
#include "vpvl2/gl2/PMXRenderEngine.h"

namespace
{

using namespace vpvl2;

class Matrices : public Scene::IMatrices {
public:
    Matrices() {}
    ~Matrices() {}

    void getModel(float value[16]) const { memcpy(value, m_model, sizeof(m_model)); }
    void getView(float value[16]) const { memcpy(value, m_view, sizeof(m_view)); }
    void getProjection(float value[16]) const { memcpy(value, m_projection, sizeof(m_projection)); }
    void getModelView(float value[16]) const { memcpy(value, m_modelView, sizeof(m_modelView)); }
    void getModelViewProjection(float value[16]) const { memcpy(value, m_modelViewProjection, sizeof(m_modelViewProjection)); }
    void getNormal(float value[9]) const { memcpy(value, m_normal, sizeof(m_normal)); }
    void setModel(float value[16]) { memcpy(m_model, value, sizeof(m_model)); }
    void setView(float value[16]) { memcpy(m_view, value, sizeof(m_view)); }
    void setProjection(float value[16]) { memcpy(m_projection, value, sizeof(m_projection)); }
    void setModelView(float value[16]) { memcpy(m_modelView, value, sizeof(m_modelView)); }
    void setModelViewProjection(float value[16]) { memcpy(m_modelViewProjection, value, sizeof(m_modelViewProjection)); }
    void setNormal(float value[9]) { memcpy(m_normal, value, sizeof(m_normal)); }

private:
    float m_model[16];
    float m_view[16];
    float m_projection[16];
    float m_modelView[16];
    float m_modelViewProjection[16];
    float m_normal[9];
};
class Light : public Scene::ILight {
public:
    Light() :
        m_motion(0),
        m_color(kZeroV3),
        m_direction(kZeroV3)
    {
        resetDefault();
    }
    ~Light() {
    }

    const Vector3 &color() const { return m_color; }
    const Vector3 &direction() const { return m_direction; }
    IMotion *motion() const { return m_motion; }
    void setColor(const Vector3 &value) { m_color = value; }
    void setDirection(const Vector3 &value) { m_direction = value; }
    void setMotion(IMotion *value) { m_motion = value; }
    void resetDefault() {
        setColor(Vector3(0.6, 0.6, 0.6));
        setDirection(Vector3(0.5, 1.0, 0.5));
    }

private:
    IMotion *m_motion;
    Vector3 m_color;
    Vector3 m_direction;
};
class Camera : public Scene::ICamera {
public:
    Camera()
        : m_motion(0),
          m_transform(Transform::getIdentity()),
          m_position(kZeroV3),
          m_angle(kZeroV3),
          m_fovy(0),
          m_distance(0)
    {
        resetDefault();
    }
    ~Camera()
    {
    }

    const Transform &transform() const { return m_transform; }
    const Vector3 &position() const { return m_position; }
    const Vector3 &angle() const { return m_angle; }
    Scalar fovy() const { return m_fovy; }
    Scalar distance() const { return m_distance; }
    IMotion *motion() const { return m_motion; }
    void setPosition(const Vector3 &value) { m_position = value; }
    void setAngle(const Vector3 &value) { m_angle = value; }
    void setFovy(float value) { m_fovy = value; }
    void setDistance(float value) { m_distance = value; }
    void setMotion(IMotion *value) { m_motion = value; }
    void resetDefault() {
        setPosition(Vector3(0, 10, 0));
        setFovy(30);
        setDistance(50);
        updateTransform();
    }

    void updateTransform() {
        static const Vector3 kUnitX(1, 0, 0), kUnitY(0, 1, 0), kUnitZ(0, 0, 1);
        const Quaternion rotationX(kUnitX, vpvl2::radian(m_angle.x())),
                rotationY(kUnitY, vpvl2::radian(m_angle.y())),
                rotationZ(kUnitZ, vpvl2::radian(m_angle.z()));
        m_transform.setIdentity();
        m_transform.setRotation(rotationZ * rotationX * rotationY);
        m_transform.setOrigin(m_transform * -m_position - Vector3(0, 0, m_distance));
    }
    void updateMatrices(Matrices &matrices) const {
        float matrix4x4[16], v[12], matrix3x3[9];
        m_transform.getOpenGLMatrix(matrix4x4);
        matrices.setView(matrix4x4);
        matrices.setModelView(matrix4x4);
        m_transform.getBasis().inverse().transpose().getOpenGLSubMatrix(v);
        matrix3x3[0] = v[0]; matrix3x3[1] = v[1]; matrix3x3[2] = v[2];
        matrix3x3[3] = v[4]; matrix3x3[4] = v[5]; matrix3x3[5] = v[6];
        matrix3x3[6] = v[8]; matrix3x3[7] = v[9]; matrix3x3[8] = v[10];
        matrices.setNormal(matrix3x3);
    }

private:
    IMotion *m_motion;
    Transform m_transform;
    Quaternion m_rotation;
    Vector3 m_position;
    Vector3 m_angle;
    float m_fovy;
    float m_distance;
};

}

namespace vpvl2
{

struct Scene::PrivateContext {
    PrivateContext()
    {
    }
    ~PrivateContext() {
        engines.releaseAll();
        motions.releaseAll();
    }

    Hash<HashPtr, IRenderEngine *> model2engine;
    Array<IModel *> models;
    Array<IMotion *> motions;
    Array<IRenderEngine *> engines;
    Matrices matrices;
    Light light;
    Camera camera;
    Color lightColor;
};

bool Scene::isAcceleratorSupported()
{
#ifdef VPVL2_ENABLE_OPENCL
    return true;
#else
    return false;
#endif
}

Scene::Scene()
    : m_context(0)
{
    m_context = new Scene::PrivateContext();
}

Scene::~Scene()
{
    delete m_context;
    m_context = 0;
}

IRenderEngine *Scene::createRenderEngine(IRenderDelegate *delegate, IModel *model) const
{
    IRenderEngine *engine = 0;
    switch (model->type()) {
    case IModel::kAsset:
#ifdef VPVL2_LINK_ASSIMP
        engine = new gl2::AssetRenderEngine(delegate, this, static_cast<asset::Model *>(model));
#endif
        break;
    case IModel::kPMD:
        engine = new gl2::PMDRenderEngine(delegate, this, static_cast<pmd::Model *>(model));
        break;
    case IModel::kPMX:
        engine = new gl2::PMXRenderEngine(delegate, this, static_cast<pmx::Model *>(model));
        break;
    default:
        break;
    }
    return engine;
}

void Scene::addModel(IModel *model, IRenderEngine *engine)
{
    m_context->models.add(model);
    m_context->engines.add(engine);
    m_context->model2engine.insert(HashPtr(model), engine);
}

void Scene::addMotion(IMotion *motion)
{
    m_context->motions.add(motion);
}

void Scene::removeModel(IModel *model)
{
    const HashPtr key(model);
    m_context->models.remove(model);
    IRenderEngine **engine = const_cast<IRenderEngine **>(m_context->model2engine.find(key));
    if (engine) {
        m_context->model2engine.remove(key);
        m_context->engines.remove(*engine);
    }
}

void Scene::removeMotion(IMotion *motion)
{
    m_context->motions.remove(motion);
}

void Scene::seek(float frameIndex)
{
    Camera &camera = m_context->camera;
    vmd::Motion *cameraMotion = static_cast<vmd::Motion *>(camera.motion());
    if (cameraMotion) {
        const vmd::CameraAnimation &animation = cameraMotion->cameraAnimation();
        if (animation.countKeyframes() > 0) {
            cameraMotion->seek(frameIndex);
            camera.setPosition(animation.position());
            camera.setAngle(animation.angle());
            camera.setFovy(animation.fovy());
            camera.setDistance(animation.distance());
        }
    }
    Light &light = m_context->light;
    vmd::Motion *lightMotion = static_cast<vmd::Motion *>(camera.motion());
    if (lightMotion) {
        const vmd::LightAnimation &animation = lightMotion->lightAnimation();
        if (animation.countKeyframes() > 0) {
            lightMotion->seek(frameIndex);
            light.setColor(animation.color());
            light.setDirection(animation.direction());
        }
    }
    camera.updateTransform();
    camera.updateMatrices(m_context->matrices);
}

void Scene::advance(float delta)
{
    Camera &camera = m_context->camera;
    vmd::Motion *cameraMotion = static_cast<vmd::Motion *>(camera.motion());
    if (cameraMotion) {
        const vmd::CameraAnimation &animation = cameraMotion->cameraAnimation();
        if (animation.countKeyframes() > 0) {
            cameraMotion->advance(delta);
            camera.setPosition(animation.position());
            camera.setAngle(animation.angle());
            camera.setFovy(animation.fovy());
            camera.setDistance(animation.distance());
        }
    }
    Light &light = m_context->light;
    vmd::Motion *lightMotion = static_cast<vmd::Motion *>(m_context->light.motion());
    if (lightMotion) {
        const vmd::LightAnimation &animation = lightMotion->lightAnimation();
        if (animation.countKeyframes() > 0) {
            lightMotion->advance(delta);
            light.setColor(animation.color());
            light.setDirection(animation.direction());
        }
    }
    camera.updateTransform();
    camera.updateMatrices(m_context->matrices);
}

const Array<IModel *> &Scene::models() const
{
    return m_context->models;
}

const Array<IMotion *> &Scene::motions() const
{
    return m_context->motions;
}

const Array<IRenderEngine *> &Scene::renderEngines() const
{
    return m_context->engines;
}

IRenderEngine *Scene::renderEngine(IModel *model) const
{
    IRenderEngine **engine = const_cast<IRenderEngine **>(m_context->model2engine.find(HashPtr(model)));
    return engine ? *engine : 0;
}

Scene::IMatrices *Scene::matrices() const
{
    return &m_context->matrices;
}

Scene::ILight *Scene::light() const
{
    return &m_context->light;
}

Scene::ICamera *Scene::camera() const
{
    return &m_context->camera;
}

}

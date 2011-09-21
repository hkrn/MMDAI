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

#ifndef VPVL_SCENE_H_
#define VPVL_SCENE_H_

#include "vpvl/Common.h"

class btDiscreteDynamicsWorld;

namespace vpvl
{

class CameraAnimation;
class PMDModel;
class VMDMotion;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Scene class represents scene with camera and set of models.
 */

class VPVL_API Scene
{
public:
    static const int kFPS = 30;
    static const float kFrustumNear;
    static const float kFrustumFar;
    static const float kMinMoveDiff;
    static const float kMoveSpeedRate;
    static const float kMinSpinDiff;
    static const float kSpinSpeedRate;
    static const float kMinDistanceDiff;
    static const float kDistanceSpeedRate;
    static const float kMinFovyDiff;
    static const float kFovySpeedRate;

    Scene(int width, int height, int fps);
    ~Scene();

    void addModel(PMDModel *model);
    PMDModel **getRenderingOrder(size_t &size);
    void getModelViewMatrix(float matrix[16]) const;
    void getInvertedModelViewMatrix(float matrix[16]) const;
    void getProjectionMatrix(float matrix[16]) const;
    void removeModel(PMDModel *model);
    void resetCamera();
    void seekMotion(float frameIndex);
    void setCameraPerspective(CameraAnimation *camera);
    void setCameraPerspective(const btVector3 &position, const btVector3 &angle, float fovy, float distance);
    void setCameraMotion(VMDMotion *motion);
    void setLightComponent(const btVector4 &ambient, const btVector4 &diffuse, const btVector4 &specular);
    void setLightSource(const btVector4 &color, const btVector3 &position);
    void setViewMove(int viewMoveTime);
    void setWorld(btDiscreteDynamicsWorld *world);
    void advanceMotion(float deltaFrame);
    void resetMotion();
    bool isMotionReached(float atEnd);
    void updateModelView(int ellapsedTimeForMove);
    void updateProjection(int ellapsedTimeForMove);

    const Array<PMDModel *> &models() const {
        return m_models;
    }
    const btVector4 &lightColor() const {
        return m_lightColor;
    }
    const btVector3 &lightPosition() const {
        return m_lightPosition;
    }
    const btVector4 &lightAmbient() const {
        return m_lightAmbient;
    }
    const btVector4 &lightDiffuse() const {
        return m_lightDiffuse;
    }
    const btVector4 &lightSpecular() const {
        return m_lightSpecular;
    }
    const btVector3 &angle() const {
        return m_angle;
    }
    const btVector3 &position() const {
        return m_position;
    }
    float distance() const {
        return m_distance;
    }
    float fovy() const {
        return m_fovy;
    }
    int width() const {
        return m_width;
    }
    int height() const {
        return m_height;
    }
    int preferredFPS() const {
        return m_preferredFPS;
    }

    void setWidth(int value) {
        m_width = value;
        updateProjectionMatrix();
    }
    void setHeight(int value) {
        m_height = value;
        updateProjectionMatrix();
    }
    void setPreferredFPS(int value) {
        m_preferredFPS = value;
    }

private:
    void sortRenderingOrder();
    void updateModelViewMatrix();
    void updateProjectionMatrix();
    void updateRotationFromAngle();
    bool updateDistance(int ellapsedTimeForMove);
    bool updateFovy(int ellapsedTimeForMove);

    btDiscreteDynamicsWorld *m_world;
    Array<PMDModel *> m_models;
    VMDMotion *m_cameraMotion;
    btTransform m_modelview;
    btQuaternion m_currentRotation;
    btQuaternion m_rotation;
    btQuaternion m_viewMoveRotation;
    btVector4 m_lightColor;
    btVector3 m_lightPosition;
    btVector4 m_lightAmbient;
    btVector4 m_lightDiffuse;
    btVector4 m_lightSpecular;
    btVector3 m_currentPosition;
    btVector3 m_position;
    btVector3 m_viewMovePosition;
    btVector3 m_angle;
    btVector3 m_currentDistance;
    btVector3 m_viewMoveDistance;
    float m_projection[16];
    float m_distance;
    float m_currentFovy;
    float m_fovy;
    float m_viewMoveFovy;
    int m_viewMoveTime;
    int m_preferredFPS;
    int m_width;
    int m_height;

    VPVL_DISABLE_COPY_AND_ASSIGN(Scene)
};

}

#endif

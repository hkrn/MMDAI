/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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
class LightAnimation;
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

    Scene(int width, int height, int fps);
    ~Scene();

    void addModel(PMDModel *model);
    const Array<PMDModel *> &getRenderingOrder() const;
    void getModelViewMatrix(float matrix[16]) const;
    void getNormalMatrix(float matrix[9]) const;
    void getProjectionMatrix(float matrix[16]) const;
    void removeModel(PMDModel *model);
    void resetCamera();
    void seekMotion(float frameIndex);
    void setCameraPerspective(CameraAnimation *camera);
    void setCameraPerspective(const Vector3 &cameraPosition, const Vector3 &cameraAngle, float fovy, float cameraDistance);
    void setCameraMotion(VMDMotion *motion);
    void setHeight(int value);
    void setLightMotion(VMDMotion *motion);
    void setLightSource(LightAnimation *light);
    void setLightSource(const Color &color, const Vector3 &cameraPosition);
    void setPreferredFPS(int value);
    void setRenderingOrder(const Array<PMDModel *> &models);
    void setSoftwareSkinningEnable(bool value);
    void setViewMove(int viewMoveTime);
    void setWidth(int value);
    void setWorld(btDiscreteDynamicsWorld *world);
    void sortRenderingOrder();
    void advanceMotion(float deltaFrame);
    void resetMotion();
    float maxFrameIndex() const;
    bool isMotionFinished() const;
    bool isMotionReachedTo(float frameIndex) const;
    void updateModelView();
    void updateProjection();

    VMDMotion *cameraMotion() const {
        return m_cameraMotion;
    }
    VMDMotion *lightMotion() const {
        return m_lightMotion;
    }
    const Transform &modelViewTransform() const {
        return m_modelview;
    }
    const Color &lightColor() const {
        return m_lightColor;
    }
    const Vector3 &lightPosition() const {
        return m_lightPosition;
    }
    const Vector3 &cameraAngle() const {
        return m_cameraAngle;
    }
    const Vector3 &cameraPosition() const {
        return m_cameraPosition;
    }
    float cameraDistance() const {
        return m_cameraDistance;
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
    bool isSoftwareSkinningEnabled() const {
        return m_enableSoftwareSkinning;
    }

private:
    void updateRotationFromAngle();

    btDiscreteDynamicsWorld *m_world;
    Array<PMDModel *> m_models;
    VMDMotion *m_cameraMotion;
    VMDMotion *m_lightMotion;
    Transform m_modelview;
    Quaternion m_rotation;
    Color m_lightColor;
    Vector3 m_lightPosition;
    Vector3 m_cameraPosition;
    Vector3 m_cameraAngle;
    float m_projection[16];
    float m_cameraDistance;
    float m_fovy;
    int m_preferredFPS;
    int m_width;
    int m_height;
    bool m_enableSoftwareSkinning;

    VPVL_DISABLE_COPY_AND_ASSIGN(Scene)
};

}

#endif

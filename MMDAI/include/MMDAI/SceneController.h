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

/* headers */

#ifndef MMDAI_SCENECONTROLLER_H_
#define MMDAI_SCENECONTROLLER_H_

#include <MMDME/MMDME.h>

#include "MMDAI/MotionCache.h"
#include "MMDAI/Stage.h"

namespace MMDAI {

class ILipSyncLoader;
class IPreference;
class ISceneEventHandler;
class PMDObject;
class SceneRenderEngine;

typedef struct RenderDepth RenderDepth;

class SceneController
{
public:
    static const float kRenderViewPointFrustumNear;
    static const float kRenderViewPointFrustumFar;
    static const float kRenderViewPointCameraZ;
    static const float kRenderViewPointYOffset;

    SceneController(ISceneEventHandler *handler, IPreference *preference);
    ~SceneController();

    void initialize(int width, int height);

    PMDObject *allocateObject();
    PMDObject *findObject(PMDObject *object);
    PMDObject *findObject(const char *alias);

    bool loadFloor(IModelLoader *loader);
    bool loadBackground(IModelLoader *loader);
    bool loadStage(IModelLoader *loader);

    bool addMotion(PMDObject *object,
                   const char *motionAlias,
                   IMotionLoader *loader,
                   bool full,
                   bool once,
                   bool enableSmooth,
                   bool enableRePos,
                   float priority);
    bool changeMotion(PMDObject *object,
                      const char *motionAlias,
                      IMotionLoader *loader);
    bool deleteMotion(PMDObject *object,
                      const char *motionAlias);

    bool addModel(const char *modelAlias,
                  IModelLoader *modelLoader,
                  ILipSyncLoader *lipSyncLoader,
                  btVector3 *pos,
                  btQuaternion *rot,
                  const char *baseModelAlias,
                  const char *baseBoneName);
    bool changeModel(PMDObject *object,
                     IModelLoader *loader,
                     ILipSyncLoader *lipSyncLoader);
    void deleteModel(PMDObject *object);

    void startMove(PMDObject *object,
                   const btVector3 &pos,
                   bool local,
                   float speed);
    void stopMove(PMDObject *object);

    void startRotation(PMDObject *object,
                       const btQuaternion &rot,
                       bool local,
                       float speed);
    void stopRotation(PMDObject *object);

    void startTurn(PMDObject *object,
                   const btVector3 &pos,
                   bool local,
                   float speed);
    void stopTurn(PMDObject *object);

    bool startLipSync(PMDObject *object,
                      const char *seq);
    bool stopLipSync(PMDObject *object);
    void resetCamera(const btVector3 &trans,
                     const btVector3 &angle,
                     const float distance,
                     const float fovy);
    void loadCameraMotion(IMotionLoader *motionLoader);

    void setLightDirection(float x, float y);
    void setLightDirection(const btVector3 &direction);
    void setLightColor(const btVector3 &color);
    void setModelViewPosition(int x, int y);
    void setModelViewRotation(int x, int y);
    void setHighlightObject(PMDObject *object);
    void setRect(int width, int height);
    void setViewMoveTimer(int ms);

    void updateLight();
    void translate(const btVector3 &value);

    void selectObject(PMDObject *object);
    void selectObject(int x, int y);
    void selectObject(int x, int y, PMDObject **dropAllowedModel);
    void deselectObject();

    void updateMotion(double procFrame, double adjustFrame);
    void updateObject(double fps);
    void updateDepthTextureViewParam();
    void updateSkin();
    void updateProjection(int ellapsedTimeForMove);
    void updateModelView(int ellapsedTimeForMove);
    void prerenderScene();
    void renderScene();

    inline PMDObject *getObjectAt(int index) const {
        if (index < 0 || index > m_numModel)
            return NULL;
        return m_objects[index];
    }
    inline PMDObject *getSelectedObject() const {
        return getObjectAt(m_selectedModel);
    }
    inline const int countObjects() const {
        return m_numModel;
    }
    inline const int getMaxObjects() const {
        return m_maxModel;
    }
    inline const btVector3 getScreenPointPosition(const btVector3 &src) {
        return m_transMatrix.inverse() * src;
    }
    inline const float getDistance() const {
        return m_distance;
    }
    inline void setDistance(const float value) {
        if (!m_viewControlledByMotion)
            m_distance = value;
    }
    inline const float getFovy() const {
        return m_fovy;
    }
    inline void setFovy(const float value) {
        if (!m_viewControlledByMotion)
            m_fovy = value;
    }
    inline void setModelView(const btTransform &modelView) {
        m_engine->setModelView(modelView);
    }
    inline void setProjection(const float projection[16]) {
        m_engine->setProjection(projection);
    }
    inline void setShadowMapping() {
        m_engine->setShadowMapping();
    }
    inline Stage *getStage() const {
        return m_stage;
    }
    inline const int getWidth() const {
        return m_width;
    }
    inline const int getHeight() const {
        return m_height;
    }
    inline bool isViewMoving() const {
        return m_viewMoveTime > 0 && (
                    m_currentRot != m_rot ||
                    m_currentTrans != m_trans ||
                    m_currentDistance != m_distance ||
                    m_currentFovy != m_fovy
                    );
    }
    inline BulletPhysics *getPhysicalEngine() {
        return &m_bullet;
    }

private:
    void updateModelViewMatrix();
    void updateProjectionMatrix();
    void updateRotationFromAngle();
    bool updateDistance(int ellapsedTimeForMove);
    bool updateFovy(int ellapsedTimeForMove);
    void sortRenderOrder();
    void eraseModel(PMDObject *object);
    void sendEvent0(const char *type);
    void sendEvent1(const char *type, const char *arg1);
    void sendEvent2(const char *type, const char *arg1, const char *arg2);

    BulletPhysics m_bullet;
    CameraController m_cameraController;
    VMD m_cameraMotion;
    SceneRenderEngine *m_engine;
    MotionCache m_motion;
    PMDObject **m_objects;
    PMDObject *m_highlightModel;
    IPreference *m_preference;
    ISceneEventHandler *m_handler;
    Stage *m_stage;
    int m_maxModel;
    int m_numModel;
    int m_selectedModel;
    int m_width;
    int m_height;
    bool m_cameraControlled;
    bool m_enablePhysicsSimulation;

    int m_viewMoveTime;
    bool m_viewControlledByMotion;
    btVector3 m_viewMoveStartTrans;
    btQuaternion m_viewMoveStartRot;
    float m_viewMoveStartDistance;
    float m_viewMoveStartFovy;
    RenderDepth *m_depth;
    int16_t *m_order;

    btVector3 m_trans;
    btVector3 m_angle;
    btQuaternion m_rot;
    float m_distance;
    float m_fovy;

    btVector3 m_currentTrans;
    btQuaternion m_currentRot;
    float m_currentDistance;
    float m_currentFovy;
    btTransform m_transMatrix;

    MMDME_DISABLE_COPY_AND_ASSIGN(SceneController)
};

} /* namespace */

#endif // MMDAI_SCENECONTROLLER_H_


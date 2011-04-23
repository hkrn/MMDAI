/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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

#include "MMDAI/MMDAI.h"

#if defined(OPENGLES1)
#include "MMDAI/GLES1SceneRenderEngine.h"
#else
#include "GLSceneRenderEngine.h"
#endif

namespace MMDAI {

/* command names */
const char *SceneEventHandler::kModelAddCommand = "MODEL_ADD";
const char *SceneEventHandler::kModelChangeCommand = "MODEL_CHANGE";
const char *SceneEventHandler::kModelDeleteCommand = "MODEL_DELETE";
const char *SceneEventHandler::kMotionAddCommand = "MOTION_ADD";
const char *SceneEventHandler::kMotionChangeCommand = "MOTION_CHANGE";
const char *SceneEventHandler::kMotionDeleteCommand = "MOTION_DELETE";
const char *SceneEventHandler::kMoveStartCommand = "MOVE_START";
const char *SceneEventHandler::kMoveStopCommand = "MOVE_STOP";
const char *SceneEventHandler::kTurnStartCommand = "TURN_START";
const char *SceneEventHandler::kTurnStopCommand = "TURN_STOP";
const char *SceneEventHandler::kRotateStartCommand = "ROTATE_START";
const char *SceneEventHandler::kRotateStopCommand = "ROTATE_STOP";
// const char *SceneEventHandler::kSoundStartCommand = "SOUND_START";
// const char *SceneEventHandler::kSoundStopCommand = "SOUND_STOP";
const char *SceneEventHandler::kStageCommand = "STAGE";
const char *SceneEventHandler::kFloorCommand = "FLOOR";
const char *SceneEventHandler::kBackgroundCommand = "BACKGROUND";
const char *SceneEventHandler::kLightColorCommand = "LIGHTCOLOR";
const char *SceneEventHandler::kLightDirectionCommand = "LIGHTDIRECTION";
const char *SceneEventHandler::kLipSyncStartCommand = "LIPSYNC_START";
const char *SceneEventHandler::kLipSyncStopCommand = "LIPSYNC_STOP";

const char *SceneEventHandler::kModelAddEvent = "MODEL_EVENT_ADD";
const char *SceneEventHandler::kModelChangeEvent = "MODEL_EVENT_CHANGE";
const char *SceneEventHandler::kModelDeleteEvent = "MODEL_EVENT_DELETE";
const char *SceneEventHandler::kMotionAddEvent = "MOTION_EVENT_ADD";
const char *SceneEventHandler::kMotionDeleteEvent = "MOTION_EVENT_DELETE";
const char *SceneEventHandler::kMotionChangeEvent = "MOTION_EVENT_CHANGE";
const char *SceneEventHandler::kMotionLoopEvent = "MOTION_EVENT_LOOP";
const char *SceneEventHandler::kMoveStartEvent = "MOVE_EVENT_START";
const char *SceneEventHandler::kMoveStopEvent = "MOVE_EVENT_STOP";
const char *SceneEventHandler::kTurnStartEvent = "TURN_EVENT_START";
const char *SceneEventHandler::kTurnStopEvent = "TURN_EVENT_STOP";
const char *SceneEventHandler::kRotateStartEvent = "ROTATE_EVENT_START";
const char *SceneEventHandler::kRotateStopEvent = "ROTATE_EVENT_STOP";
// const char *SceneEventHandler::kSoundStartEvent = "SOUND_EVENT_START";
// const char *SceneEventHandler::kSoundStopEvent = "SOUND_EVENT_STOP";
const char *SceneEventHandler::kStageEvent = "STAGE";
const char *SceneEventHandler::kFloorEvent = "FLOOR";
const char *SceneEventHandler::kBackgroundEvent = "BACKGROUND";
const char *SceneEventHandler::kLightColorEvent = "LIGHTCOLOR";
const char *SceneEventHandler::kLightDirectionEvent = "LIGHTDIRECTION";
const char *SceneEventHandler::kLipSyncStartEvent = "LIPSYNC_EVENT_START";
const char *SceneEventHandler::kLipSyncStopEvent = "LIPSYNC_EVENT_STOP";
const char *SceneEventHandler::kKeyEvent = "KEY";

const float SceneController::kRenderViewPointFrustumNear = 5.0f;
const float SceneController::kRenderViewPointFrustumFar = 2000.0f;
const float SceneController::kRenderViewPointCameraZ = -100.0f;
const float SceneController::kRenderViewPointYOffset = -13.0f;

#define RENDER_MINSCALEDIFF   0.001f
#define RENDER_SCALESPEEDRATE 0.9f
#define RENDER_MINMOVEDIFF    0.000001f
#define RENDER_MOVESPEEDRATE  0.9f
#define RENDER_MINSPINDIFF    0.000001f
#define RENDER_SPINSPEEDRATE  0.9f

struct RenderDepth {
    float distance;
    short id;
};

static int compareDepth(const void *a, const void *b)
{
    const RenderDepth *x = static_cast<const RenderDepth *>(a);
    const RenderDepth *y = static_cast<const RenderDepth *>(b);
    if (x->distance == y->distance)
        return 0;
    return x->distance > y->distance ? 1 : -1;
}

// from util.h
static int getNumDigit(int in)
{
    int out = 0;
    if(in == 0)
        return 1;
    if (in < 0) {
        out = 1;
        in *= -1;
    }
    for (; in != 0; out++)
        in /= 10;
    return out;
}

SceneController::SceneController(SceneEventHandler *handler, IPreference *preference)
    : m_engine(0),
    m_objects(0),
    m_highlightModel(0),
    m_preference(preference),
    m_handler(handler),
    m_stage(0),
    m_numModel(0),
    m_selectedModel(-1),
    m_width(0),
    m_height(0),
    m_enablePhysicsSimulation(true),
    m_scale(1.0),
    m_trans(0.0f, 0.0f, 0.0f),
    m_rot(0.0f, 0.0f, 0.0f, 1.0f),
    m_cameraTrans(0.0f, kRenderViewPointYOffset, kRenderViewPointCameraZ),
    m_viewMoveTime(-1),
    m_viewMoveStartTrans(0.0f, 0.0f, 0.0f),
    m_viewMoveStartRot(0.0f, 0.0f, 0.0f, 1.0f),
    m_viewMoveStartScale(0.0f),
    m_depth(0),
    m_order(0),
    m_currentScale(m_scale),
    m_currentTrans(m_trans),
    m_currentRot(m_rot)
{
#if defined(OPENGLES1)
    m_engine = new GLES1SceneRenderEngine(preference);
#else
    m_engine = new GLSceneRenderEngine(preference);
#endif
    m_objects = new PMDObject*[MAX_MODEL];
    m_stage = new Stage(m_engine);
    for (int i = 0; i < MAX_MODEL; i++) {
        m_objects[i] = new PMDObject(m_engine);
    }
    m_transMatrix.setIdentity();
    m_depth = new RenderDepth[MAX_MODEL];
    m_order = new int16_t[MAX_MODEL];
    updateModelView(0);
    updateProjection(0);
}

SceneController::~SceneController()
{
    m_currentScale = 0.0f;
    m_scale = 0.0f;
    m_enablePhysicsSimulation = false;
    m_height = 0;
    m_width = 0;
    m_selectedModel = -1;
    m_numModel = 0;
    m_highlightModel = 0;
    for (int i = 0; i < MAX_MODEL; i++) {
        delete m_objects[i];
        m_objects[i] = 0;
    }
    delete[] m_order;
    delete[] m_depth;
    delete[] m_objects;
    m_objects = 0;
    delete m_stage;
    m_stage = 0;
    delete m_engine;
    m_engine = 0;
    m_handler = 0;
    m_preference = 0;
}

void SceneController::updateLight()
{
    int i = 0;
    float direction[4];
    btVector3 l;
    m_engine->updateLighting();
    m_preference->getFloat4(kPreferenceLightDirection, direction);
    m_stage->updateShadowMatrix(direction);
    l = btVector3(direction[0], direction[1], direction[2]);
    for (i = 0; i < m_numModel; i++) {
        PMDObject *object = m_objects[i];
        if (object->isEnable())
            object->setLightForToon(&l);
    }
}

/* SceneController::setFloor: set floor image */
bool SceneController::loadFloor(IModelLoader *loader)
{
    /* load floor */
    const char *fileName = loader->getLocation();
    if (fileName == NULL)
        return false;
    if (m_stage->loadFloor(loader, &m_bullet) == false) {
        MMDAILogWarn("cannot set floor %s.", fileName);
        return false;
    }

    /* send event message */
    sendEvent1(SceneEventHandler::kFloorEvent, fileName);

    return true;
}

/* SceneController::setBackground: set background image */
bool SceneController::loadBackground(IModelLoader *loader)
{
    /* load background */
    const char *fileName = loader->getLocation();
    if (fileName == NULL)
        return false;
    if (m_stage->loadBackground(loader, &m_bullet) == false) {
        MMDAILogWarn("cannot set background %s.", fileName);
        return false;
    }

    /* send event message */
    sendEvent1(SceneEventHandler::kBackgroundEvent, fileName);

    return true;
}

/* SceneController::setStage: set stage */
bool SceneController::loadStage(IModelLoader *loader)
{
    /* load stage */
    const char *fileName = loader->getLocation();
    if (fileName == NULL)
        return false;
    if (m_stage->loadStagePMD(loader, &m_bullet) == false) {
        MMDAILogWarn("cannot set stage %s.", fileName);
        return false;
    }

    /* send event message */
    sendEvent1(SceneEventHandler::kStageEvent, fileName);

    return true;
}

/* SceneController::getNewModelId: return new model ID */
PMDObject *SceneController::allocatePMDObject()
{
    PMDObject *object = NULL;
    for (int i = 0; i < m_numModel; i++) {
        PMDObject *object = m_objects[i];
        if (!object->isEnable())
            return object; /* re-use it */
    }
    if (m_numModel >= MAX_MODEL)
        return NULL; /* no more room */
    object = m_objects[m_numModel];
    m_numModel++;
    object->setEnable(false); /* model is not loaded yet */
    return object;
}

PMDObject *SceneController::findPMDObject(PMDObject *object)
{
    return findPMDObject(object->getAlias());
}

PMDObject *SceneController::findPMDObject(const char *alias)
{
    for (int i = 0; i < m_numModel; i++) {
        PMDObject *object = m_objects[i];
        if (object->isEnable() && MMDAIStringEquals(object->getAlias(), alias))
            return object;
    }
    return NULL;
}

/* SceneController::addMotion: add motion */
bool SceneController::addMotion(PMDObject *object, IMotionLoader *loader)
{
    return addMotion(object, NULL, loader, false, true, true, true, 0.0f);
}

/* SceneController::addMotion: add motion */
bool SceneController::addMotion(PMDObject *object,
                                const char *motionAlias,
                                IMotionLoader *loader,
                                bool full,
                                bool once,
                                bool enableSmooth,
                                bool enableRePos,
                                float priority)
{
    int i;
    bool find;
    VMD *vmd;
    MotionPlayer *motionPlayer;
    char *name;
    size_t allocSize;

    /* motion file */
    vmd = m_motion.loadFromLoader(loader);
    if (vmd == NULL) {
        MMDAILogWarn("addMotion: failed to load %s.", loader->getLocation());
        return false;
    }

    /* alias */
    if (motionAlias && MMDAIStringLength(motionAlias) > 0) {
        /* check the same alias */
        name = MMDAIStringClone(motionAlias);
        motionPlayer = object->getMotionManager()->getMotionPlayerList();
        for (; motionPlayer != NULL; motionPlayer = motionPlayer->next) {
            if (motionPlayer->active && MMDAIStringEquals(motionPlayer->name, name)) {
                MMDAILogWarn("addMotion: motion alias \"%s\" is already used.", name);
                MMDAIMemoryRelease(name);
                return false;
            }
        }
    } else {
        /* if motion alias is not specified, unused digit is used */
        for (i = 0;; i++) {
            find = false;
            allocSize = sizeof(char) * (getNumDigit(i) + 1);
            name = static_cast<char *>(MMDAIMemoryAllocate(allocSize));
            if (name == NULL)
                return false;
            MMDAIStringFormat(name, allocSize, "%d", i);
            motionPlayer = object->getMotionManager()->getMotionPlayerList();
            for (; motionPlayer != NULL; motionPlayer = motionPlayer->next) {
                if (motionPlayer->active && MMDAIStringEquals(motionPlayer->name, name)) {
                    find = true;
                    break;
                }
            }
            if(find == false)
                break;
            MMDAIMemoryRelease(name);
        }
    }

    /* start motion */
    if (!object->startMotion(vmd, name, full, once, enableSmooth, enableRePos, priority)) {
        MMDAIMemoryRelease(name);
        return false;
    }

    /* send event message */
    sendEvent2(SceneEventHandler::kMotionAddEvent, object->getAlias(), name);

    MMDAIMemoryRelease(name);
    return true;
}

/* SceneController::changeMotion: change motion */
bool SceneController::changeMotion(PMDObject *object, const char *motionAlias, IMotionLoader *loader)
{
    VMD *vmd, *old = NULL;
    MotionPlayer *motionPlayer;

    /* check */
    if (!motionAlias) {
        MMDAILogWarn("changeMotion: not specified %s.", motionAlias);
        return false;
    }

    /* motion file */
    vmd = m_motion.loadFromLoader(loader);
    if (vmd == NULL) {
        MMDAILogWarn("changeMotion: failed to load %s.", loader->getLocation());
        return false;
    }

    /* get motion before change */
    motionPlayer = object->getMotionManager()->getMotionPlayerList();
    for (; motionPlayer != NULL; motionPlayer = motionPlayer->next) {
        if (motionPlayer->active && MMDAIStringEquals(motionPlayer->name, motionAlias)) {
            old = motionPlayer->vmd;
            break;
        }
    }
    if (old == NULL) {
        MMDAILogWarn("changeMotion: motion alias \"%s\"is not found.", motionAlias);
        m_motion.unload(vmd);
        return false;
    }

    /* change motion */
    if (object->swapMotion(vmd, motionAlias) == false) {
        MMDAILogWarn("changeMotion: motion alias \"%s\"is not found.", motionAlias);
        m_motion.unload(vmd);
        return false;
    }

    /* send event message */
    sendEvent2(SceneEventHandler::kMotionChangeEvent, object->getAlias(), motionAlias);

    /* unload old motion from motion stocker */
    m_motion.unload(old);

    return true;
}

/* SceneController::deleteMotion: delete motion */
bool SceneController::deleteMotion(PMDObject *object, const char *motionAlias)
{
    /* delete motion */
    if (!object->getMotionManager()->deleteMotion(motionAlias)) {
        MMDAILogWarn("deleteMotion: motion alias \"%s\"is not found.", motionAlias);
        return false;
    }

    /* don't send event message */
    return true;
}

/* SceneController::addModel: add model */
bool SceneController::addModel(IModelLoader *modelLoader, ILipSyncLoader *lipSyncLoader)
{
    return addModel(NULL, modelLoader, lipSyncLoader, NULL, NULL, NULL, NULL);
}

/* SceneController::addModel: add model */
bool SceneController::addModel(const char *modelAlias,
                               IModelLoader *modelLoader,
                               ILipSyncLoader *lipSyncLoader,
                               btVector3 *pos,
                               btQuaternion *rot,
                               const char *baseModelAlias,
                               const char *baseBoneName)
{
    int i;
    char *name;
    btVector3 offsetPos = btVector3(0.0f, 0.0f, 0.0f);
    btQuaternion offsetRot = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
    bool forcedPosition = false;
    PMDBone *assignBone = NULL;
    PMDObject *assignObject = NULL, *newObject = NULL;
    float direction[4];

    m_preference->getFloat4(kPreferenceLightDirection, direction);
    btVector3 light = btVector3(direction[0], direction[1], direction[2]);

    /* set */
    if (pos)
        offsetPos = (*pos);
    if (rot)
        offsetRot = (*rot);
    if (pos || rot)
        forcedPosition = true;
    if (baseModelAlias) {
        assignObject = findPMDObject(baseModelAlias);
        if (assignObject == NULL) {
            MMDAILogWarn("model alias \"%s\" is not found", baseModelAlias);
            return false;
        }
        PMDModel *model = assignObject->getPMDModel();
        if (baseBoneName) {
            assignBone = model->getBone(baseBoneName);
        } else {
            assignBone = model->getCenterBone();
        }
        if (assignBone == NULL) {
            if (baseBoneName)
                MMDAILogWarn("bone \"%s\" is not exist on %s.", baseBoneName, baseModelAlias);
            else
                MMDAILogWarn("cannot assign to bone of %s.", baseModelAlias);
            return false;
        }
    }

    /* ID */
    newObject = allocatePMDObject();
    if (newObject == NULL) {
        MMDAILogWarnString("addModel: too many models.");
        return false;
    }

    /* determine name */
    if (modelAlias && MMDAIStringLength(modelAlias) > 0) {
        /* check the same alias */
        name = MMDAIStringClone(modelAlias);
        if (findPMDObject(name) != NULL) {
            MMDAILogWarn("addModel: model alias \"%s\" is already used.", name);
            MMDAIMemoryRelease(name);
            return false;
        }
    } else {
        /* if model alias is not specified, unused digit is used */
        for(i = 0;; i++) {
            size_t allocSize = sizeof(char) * (getNumDigit(i) + 1);
            name = static_cast<char *>(MMDAIMemoryAllocate(allocSize));
            if (name == NULL)
                return false;
            MMDAIStringFormat(name, allocSize, "%d", i);
            if (findPMDObject(name) != NULL)
                MMDAIMemoryRelease(name);
            else
                break;
        }
    }

    /* add model */
    if (!newObject->load(modelLoader,
                         lipSyncLoader,
                         &offsetPos,
                         &offsetRot,
                         forcedPosition,
                         assignBone,
                         assignObject,
                         &m_bullet,
                         m_preference->getBool(kPreferenceUseCartoonRendering),
                         m_preference->getFloat(kPreferenceCartoonEdgeWidth),
                         &light)) {
        MMDAILogWarn("addModel: failed to load %s.", modelLoader->getLocation());
        newObject->release();
        MMDAIMemoryRelease(name);
        return false;
    }

    /* initialize motion manager */
    newObject->resetMotionManager();
    newObject->setAlias(name);
    m_numModel++;

    /* send event message */
    sendEvent1(SceneEventHandler::kModelAddEvent, name);

    MMDAIMemoryRelease(name);
    return true;
}

/* SceneController::changeModel: change model */
bool SceneController::changeModel(PMDObject *object,
                                  IModelLoader *modelLoader,
                                  ILipSyncLoader *lipSyncLoader)
{
    MotionPlayer *motionPlayer = NULL;
    int i = 0;
    double currentFrame = 0;
    const char *modelAlias = object->getAlias();
    float direction[4];

    m_preference->getFloat4(kPreferenceLightDirection, direction);
    btVector3 light = btVector3(direction[0], direction[1], direction[2]);

    /* load model */
    if (!object->load(modelLoader,
                      lipSyncLoader,
                      NULL,
                      NULL,
                      false,
                      NULL,
                      NULL,
                      &m_bullet,
                      m_preference->getBool(kPreferenceUseCartoonRendering),
                      m_preference->getFloat(kPreferenceCartoonEdgeWidth),
                      &light)) {
        MMDAILogWarn("changeModel: failed to load model %s.", modelLoader->getLocation());
        return false;
    }

    /* update motion manager */
    MotionManager *manager = object->getMotionManager();
    if (manager != NULL) {
        motionPlayer = manager->getMotionPlayerList();
        for (; motionPlayer != NULL; motionPlayer = motionPlayer->next) {
            if (motionPlayer->active) {
                currentFrame = motionPlayer->mc.getCurrentFrame();
                manager->startMotionSub(motionPlayer->vmd, motionPlayer);
                motionPlayer->mc.setCurrentFrame(currentFrame);
            }
        }
    }

    /* set alias */
    object->setAlias(modelAlias);

    /* delete accessories  */
    for (i = 0; i < MAX_MODEL; i++) {
        PMDObject *assoc = m_objects[i];
        if (assoc->isEnable() && assoc->getAssignedModel() == object) {
            deleteModel(assoc);
        }
    }

    /* send event message */
    sendEvent1(SceneEventHandler::kModelChangeEvent, modelAlias);

    return true;
}

/* SceneController::deleteModel: delete model */
void SceneController::deleteModel(PMDObject *object)
{
    /* delete accessories  */
    for (int i = 0; i < MAX_MODEL; i++) {
        PMDObject *assoc = m_objects[i];
        if (assoc->isEnable() && assoc->getAssignedModel() == object) {
            deleteModel(assoc);
        }
    }

    /* set frame from now to disappear */
    object->startDisappear();
    m_numModel--;

    /* send event message */
    sendEvent1(SceneEventHandler::kModelDeleteEvent, object->getAlias());
}

void SceneController::updateLightDirection(float x, float y)
{
    float direction[4];
    float step = m_preference->getFloat(kPreferenceRotateStep) * 0.1f;
    m_preference->getFloat4(MMDAI::kPreferenceLightDirection, direction);
    btVector3 v = btVector3(direction[0], direction[1], direction[2]);
    btMatrix3x3 matrix = btMatrix3x3(btQuaternion(0, y * step, 0.0) * btQuaternion(x * step, 0, 0));
    v = v * matrix;
    setLightDirection(v.x(), v.y(), v.z());
}

void SceneController::setLightDirection(float x, float y, float z)
{
    float f[4] = { x, y, z, 0.0f };
    m_preference->setFloat4(kPreferenceLightDirection, f);
    updateLight();

    /* send event message */
    if (m_handler != NULL) {
        char buf[BUFSIZ];
        MMDAIStringFormatSafe(buf, sizeof(buf), "%.2f,%.2f,%.2f", x, y, z);
        sendEvent1(SceneEventHandler::kLightDirectionEvent, buf);
    }
}

void SceneController::setLightColor(float r, float g, float b)
{
    float f[3] = { r, g, b };
    m_preference->setFloat3(kPreferenceLightColor, f);
    updateLight();

    /* send event message */
    if (m_handler != NULL) {
        char buf[BUFSIZ];
        MMDAIStringFormatSafe(buf, sizeof(buf), "%.2f,%.2f,%.2f", r, g, b);
        sendEvent1(SceneEventHandler::kLightColorEvent, buf);
    }
}

/* SceneController::startMove: start moving */
void SceneController::startMove(PMDObject *object, btVector3 *pos, bool local, float speed)
{
    const char *alias = object->getAlias();

    if(object->isMoving())
        sendEvent1(SceneEventHandler::kMoveStopEvent, alias);

    /* get */
    const btVector3 currentPos = object->getCurrentPosition();
    btVector3 targetPos = (*pos);

    /* local or global */
    if (local) {
        const btQuaternion currentRot = object->getCurrentRotation();
        btTransform tr = btTransform(currentRot, currentPos);
        targetPos = tr * targetPos;
    }

    /* not need to start */
    if (currentPos == targetPos) {
        sendEvent1(SceneEventHandler::kMoveStartEvent, alias);
        sendEvent1(SceneEventHandler::kMoveStopEvent, alias);
        return;
    }

    object->setMoveSpeed(speed);
    object->setPosition(targetPos);

    /* send event message */
    sendEvent1(SceneEventHandler::kMoveStartEvent, alias);
}

/* SceneController::stopMove: stop moving */
void SceneController::stopMove(PMDObject *object)
{
    const char *alias = object->getAlias();

    if (!object->isMoving()) {
        MMDAILogWarn("stopMove: %s is not moving", alias);
        return;
    }

    const btVector3 currentPos = object->getCurrentPosition();
    object->setPosition(currentPos);

    /* send event message */
    sendEvent1(SceneEventHandler::kMoveStopEvent, alias);
}

/* SceneController::startRotation: start rotation */
void SceneController::startRotation(PMDObject *object, btQuaternion *rot, bool local, float speed)
{
    const char *alias = object->getAlias();

    if (object->isRotating()) {
        if (object->isTurning())
            sendEvent1(SceneEventHandler::kTurnStopEvent, alias);
        else
            sendEvent1(SceneEventHandler::kRotateStopEvent, alias);
    }

    const btQuaternion currentRot = object->getCurrentRotation();
    btQuaternion targetRot = *rot;
    targetRot = local ? currentRot * targetRot : currentRot.nearest(targetRot);

    /* not need to start */
    if (currentRot == targetRot) {
        sendEvent1(SceneEventHandler::kRotateStartEvent, alias);
        sendEvent1(SceneEventHandler::kRotateStopEvent, alias);
        return;
    }

    object->setSpinSpeed(speed);
    object->setRotation(targetRot);
    object->setTurning(false);

    sendEvent1(SceneEventHandler::kRotateStartEvent, alias);
}

/* SceneController::stopRotation: stop rotation */
void SceneController::stopRotation(PMDObject *object)
{
    const char *alias = object->getAlias();

    if (!object->isRotating() || object->isTurning()) {
        MMDAILogWarn("stopTurn: %s is not rotating", alias);
        return;
    }

    const btQuaternion currentRot = object->getCurrentRotation();
    object->setRotation(currentRot);

    sendEvent1(SceneEventHandler::kRotateStopEvent, alias);
}

/* SceneController::startTurn: start turn */
void SceneController::startTurn(PMDObject *object, btVector3 *pos, bool local, float speed)
{
    btQuaternion targetRot;
    const char *alias = object->getAlias();

    if (object->isRotating()) {
        if (object->isTurning())
            sendEvent1(SceneEventHandler::kTurnStopEvent, alias);
        else
            sendEvent1(SceneEventHandler::kRotateStopEvent, alias);
    }

    const btVector3 currentPos = object->getCurrentPosition();
    const btQuaternion currentRot = object->getCurrentRotation();

    btVector3 targetPos = local ? *pos : *pos - currentPos;
    targetPos.normalize();

    /* calculate target rotation from (0,0,1) */
    float z = targetPos.z();
    if (z > 1.0f)
        z = 1.0f;
    if (z < -1.0f)
        z = -1.0f;
    float rad = acosf(z);
    btVector3 axis = btVector3(0.0f, 0.0f, 1.0f).cross(targetPos);
    if (axis.length2() < PMDOBJECT_MINSPINDIFF) {
        targetRot = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else {
        axis.normalize();
        targetRot = btQuaternion(axis, btScalar(rad));
    }
    targetRot = local ? currentRot * targetRot : currentRot.nearest(targetRot);

    if (currentRot == targetRot) {
        sendEvent1(SceneEventHandler::kTurnStartEvent, alias);
        sendEvent1(SceneEventHandler::kTurnStopEvent, alias);
        return;
    }

    object->setSpinSpeed(speed);
    object->setRotation(targetRot);
    object->setTurning(true);

    /* send event message */
    sendEvent1(SceneEventHandler::kTurnStartEvent, alias);
}

/* SceneController::stopTurn: stop turn */
void SceneController::stopTurn(PMDObject *object)
{
    const char *alias = object->getAlias();

    if (!object->isRotating() || !object->isTurning()) {
        MMDAILogWarn("stopTurn: %s is not turning", alias);
        return;
    }

    const btQuaternion currentRot = object->getCurrentRotation();
    object->setRotation(currentRot);

    /* send event message */
    sendEvent1(SceneEventHandler::kTurnStopEvent, alias);
}

/* SceneController::startLipSync: start lip sync */
bool SceneController::startLipSync(PMDObject *object, const char *seq)
{
    unsigned char *vmdData;
    size_t vmdSize;
    VMD *vmd;
    bool found = false;
    MotionPlayer *motionPlayer;
    const char *name = LipSync::getMotionName();

    /* create motion */
    if(object->createLipSyncMotion(seq, &vmdData, &vmdSize) == false) {
        MMDAILogWarnString("cannot create lip motion.");
        return false;
    }
    vmd = m_motion.loadFromData(vmdData, vmdSize);
    MMDAIMemoryRelease(vmdData);

    /* search running lip motion */
    motionPlayer = object->getMotionManager()->getMotionPlayerList();
    for (; motionPlayer != NULL; motionPlayer = motionPlayer->next) {
        if (motionPlayer->active && MMDAIStringEquals(motionPlayer->name, name)) {
            found = true;
            break;
        }
    }

    /* start lip sync */
    if (found) {
        if (!object->swapMotion(vmd, name)) {
            MMDAILogWarnString("cannot start lip sync.");
            m_motion.unload(vmd);
            return false;
        }
    } else {
        if (!object->startMotion(vmd, name, false, true, true, true, MotionManager::kDefaultPriority)) {
            MMDAILogWarnString("cannot start lip sync.");
            m_motion.unload(vmd);
            return false;
        }
    }

    /* send event message */
    sendEvent1(SceneEventHandler::kLipSyncStartEvent, object->getAlias());

    return true;
}

/* SceneController::stopLipSync: stop lip sync */
bool SceneController::stopLipSync(PMDObject *object)
{
    /* stop lip sync */
    if (object->getMotionManager()->deleteMotion(LipSync::getMotionName()) == false) {
        MMDAILogWarnString("lipsync motion not found");
        return false;
    }

    /* don't send message */
    return true;
}

void SceneController::initializeScreen(int width, int height)
{
    float size[3];
    m_width = width;
    m_height = height;
    m_bullet.setup(m_preference->getInt(kPreferenceBulletFPS));
    m_engine->setup();
    m_preference->getFloat3(kPreferenceStageSize, size);
    m_stage->setSize(size, 1.0f, 1.0f);
    float rot[3], trans[3], scale = 0.0f;
    scale = m_preference->getFloat(kPreferenceRenderingScale);
    m_preference->getFloat3(kPreferenceRenderingRotation, rot);
    m_preference->getFloat3(kPreferenceRenderingTransition, trans);
    resetLocation(btVector3(trans[0], trans[1], trans[2]), rot, scale);
    MMDAILogInfo("reset location rot=(%.2f, %.2f, %.2f) trans=(%.2f, %.2f, %.2f) scale=%.2f",
                 rot[0], rot[1], rot[2], trans[0], trans[1], trans[2], scale);
}

void SceneController::resetLocation(const btVector3 &trans, const float *rot, const float scale)
{
    btMatrix3x3 bm;
    bm.setEulerZYX(MMDAIMathRadian(rot[0]), MMDAIMathRadian(rot[1]), MMDAIMathRadian(rot[2]));
    bm.getRotation(m_rot);
    m_trans = trans;
    m_scale = scale;
}

void SceneController::setModelViewPosition(int x, int y)
{
    float cameraZ = kRenderViewPointCameraZ;
    float znear = kRenderViewPointFrustumNear;
    float fx = 0.0f, fy = 0.0f, fz = 20.0f;
    fx = x / static_cast<float>(m_width);
    fy = -y / static_cast<float>(m_height);
    fx = static_cast<float>(fx * (fz - cameraZ) / znear);
    fy = static_cast<float>(fy * (fz - cameraZ) / znear);
    if (m_scale != 0) {
        fx /= m_scale;
        fy /= m_scale;
    }
    fz = 0.0f;
    translate(btVector3(fx, fy, fz));
}

void SceneController::setModelViewRotation(int x, int y)
{
    float step = m_preference->getFloat(kPreferenceRotateStep) * 0.1f;
    m_rot = m_rot * btQuaternion(x * step, 0.0f, 0.0f);
    m_rot = btQuaternion(0.0f, y * step, 0.0f) * m_rot;
}

void SceneController::translate(const btVector3 &value)
{
    m_trans += value;
}

void SceneController::setRect(int width, int height)
{
    if (m_width != width || m_height != height) {
        if (width > 0)
            m_width = width;
        if (height > 0)
            m_height = height;
        updateProjection(0);
    }
}

void SceneController::setViewMoveTimer(int ms)
{
    m_viewMoveTime = ms;
    if (m_viewMoveTime > 0) {
        m_viewMoveStartRot = m_currentRot;
        m_viewMoveStartTrans = m_currentTrans;
        m_viewMoveStartScale = m_currentScale;
    }
}

void SceneController::selectPMDObject(PMDObject *object)
{
    const char *alias = object->getAlias();
    for (int i = 0; i < m_numModel; i++) {
        PMDObject *o = m_objects[i];
        if (o->isEnable() && MMDAIStringEquals(o->getAlias(), alias)) {
            m_selectedModel = i;
            break;
        }
    }
}

void SceneController::selectPMDObject(int x, int y)
{
    m_selectedModel = m_engine->pickModel(m_objects,
                                          m_numModel,
                                          x,
                                          y,
                                          NULL);
}

void SceneController::selectPMDObject(int x, int y, PMDObject **dropAllowedModel)
{
    int dropAllowedModelID = -1;
    m_selectedModel = m_engine->pickModel(m_objects,
                                          m_numModel,
                                          x,
                                          y,
                                          &dropAllowedModelID);
    if (m_selectedModel == -1)
        *dropAllowedModel = getPMDObject(dropAllowedModelID);
}

void SceneController::setHighlightPMDObject(PMDObject *object)
{
    float col[4];

    if (m_highlightModel != NULL) {
        /* reset current highlighted model */
        col[0] = PMDModel::kEdgeColorR;
        col[1] = PMDModel::kEdgeColorG;
        col[2] = PMDModel::kEdgeColorB;
        col[3] = PMDModel::kEdgeColorA;
        m_highlightModel->getPMDModel()->setEdgeColor(col);
    }
    if (object != NULL) {
        /* set highlight to the specified model */
        m_preference->getFloat4(kPreferenceCartoonEdgeSelectedColor, col);
        object->getPMDModel()->setEdgeColor(col);
    }

    m_highlightModel = object;
}

void SceneController::updateMotion(double procFrame, double adjustFrame)
{
    const char *lipSyncMotion = LipSync::getMotionName();
    for (int i = 0; i < m_numModel; i++) {
        PMDObject *object = m_objects[i];
        if (object->isEnable()) {
            object->updateRootBone();
            if (object->updateMotion(procFrame + adjustFrame)) {
                MotionPlayer *player = object->getMotionManager()->getMotionPlayerList();
                for (; player != NULL; player = player->next) {
                    if (player->statusFlag == MOTION_STATUS_DELETED) {
                        if (MMDAIStringEquals(player->name, lipSyncMotion)) {
                            sendEvent1(SceneEventHandler::kLipSyncStopEvent, object->getAlias());
                        }
                        else {
                            sendEvent2(SceneEventHandler::kMotionDeleteEvent, object->getAlias(), player->name);
                        }
                        m_motion.unload(player->vmd);
                    }
                }
            }
            if (object->updateAlpha(procFrame + adjustFrame)) {
                eraseModel(object);
            }
        }
    }
    m_bullet.update((float)procFrame);
}

void SceneController::eraseModel(PMDObject *object)
{
    /* remove assigned accessories */
    for (int i = 0; i < MAX_MODEL; i++) {
        PMDObject *assoc = m_objects[i];
        if (assoc->isEnable() && assoc->getAssignedModel() == object) {
            eraseModel(assoc);
        }
    }
    const char *alias = object->getAlias();
    const char *lipSyncMotion = LipSync::getMotionName();
    MotionPlayer *player = object->getMotionManager()->getMotionPlayerList();
    for (; player != NULL; player = player->next) {
        if (MMDAIStringEquals(player->name, lipSyncMotion)) {
            sendEvent1(SceneEventHandler::kLipSyncStopEvent, alias);
        }
        else {
            sendEvent2(SceneEventHandler::kMotionDeleteEvent, alias, player->name);
        }
        m_motion.unload(player->vmd);
    }
    /* remove model */
    object->release();
}

void SceneController::updateAfterSimulation()
{
    /* update after simulation */
    for (int i = 0; i < m_numModel; i++) {
        PMDObject *object = m_objects[i];
        object->updateAfterSimulation(m_enablePhysicsSimulation);
    }
}

void SceneController::updateDepthTextureViewParam()
{
    /* calculate rendering range for shadow mapping */
    if (m_preference->getBool(kPreferenceUseShadowMapping)) {
        int num = m_numModel;
        float d = 0, dmax = 0;
        float *r = new float[num];
        btVector3 *c = new btVector3[num];
        btVector3 cc = btVector3(0.0f, 0.0f, 0.0f);

        for (int i = 0; i < num; i++) {
            PMDObject *object = m_objects[i];
            if (!object->isEnable())
                continue;
            r[i] = object->getPMDModel()->calculateBoundingSphereRange(&(c[i]));
            cc += c[i];
        }
        if (num != 0)
            cc /= (float) num;

        dmax = 0.0f;
        for (int i = 0; i < num; i++) {
            if (!m_objects[i]->isEnable())
                continue;
            d = cc.distance(c[i]) + r[i];
            if (dmax < d)
                dmax = d;
        }
        m_engine->setShadowMapAutoView(cc, dmax);

        delete [] r;
        delete [] c;
    }
}

void SceneController::updateModelPositionAndRotation(double fps)
{
    for (int i = 0; i < m_numModel; i++) {
        PMDObject *object = m_objects[i];
        if (object->isEnable()) {
            const char *alias = object->getAlias();
            if (object->updateModelRootOffset(fps)) {
                sendEvent1(SceneEventHandler::kMoveStopEvent, alias);
            }
            if (object->updateModelRootRotation(fps)) {
                if (object->isTurning()) {
                    sendEvent1(SceneEventHandler::kTurnStopEvent, alias);
                    object->setTurning(false);
                } else {
                    sendEvent1(SceneEventHandler::kRotateStopEvent, alias);
                }
            }
        }
    }
}

inline void SceneController::sendEvent1(const char *type, const char *arg1)
{
    if (m_handler != NULL) {
        m_handler->handleEventMessage(type, 1, arg1);
    }
}

inline void SceneController::sendEvent2(const char *type, const char *arg1, const char *arg2)
{
    if (m_handler != NULL) {
        m_handler->handleEventMessage(type, 2, arg1, arg2);
    }
}

static void MMDAIFrustum(float result[16], float left, float right, float bottom, float top, float znear, float zfar)
{
    const float a = (right + left) / (right - left);
    const float b = (top + bottom) / (top - bottom);
    const float c = (zfar + znear) / (zfar - znear) * -1;
    const float d = (-2 * zfar * znear) / (zfar - znear);
    const float e = (2 * znear) / (right - left);
    const float f = (2 * znear) / (top - bottom);
    const float matrix[16] = {
        e, 0, 0, 0,
        0, f, 0, 0,
        a, b, c, -1,
        0, 0, d, 0
    };
    memcpy(result, matrix, sizeof(matrix));
}

void SceneController::updateProjection(int ellapsedTimeForMove)
{
    if (m_currentScale != m_scale) {
        if (m_viewMoveTime == 0) {
            /* immediately apply the target */
            m_currentScale = m_scale;
        } else if (m_viewMoveTime > 0) {
            /* constant move */
            if (ellapsedTimeForMove >= m_viewMoveTime) {
                m_currentScale = m_scale;
            } else {
                m_currentScale = m_viewMoveStartScale + (m_scale - m_viewMoveStartScale) * ellapsedTimeForMove / m_viewMoveTime;
            }
        } else {
            float diff = fabs(m_currentScale - m_scale);
            if (diff < RENDER_MINSCALEDIFF) {
                m_currentScale = m_scale;
            } else {
                m_currentScale = m_currentScale * (RENDER_SCALESPEEDRATE) + m_scale * (1.0f - RENDER_SCALESPEEDRATE);
            }
        }
    }
    float aspect = (m_width == 0) ? 1.0f : static_cast<float>(m_height) / m_width;
    float ratio = (m_currentScale == 0.0f) ? 1.0f : 1.0f / m_currentScale;
    float projection[16];
    MMDAIFrustum(projection, -ratio, ratio, -aspect * ratio, aspect * ratio, kRenderViewPointFrustumNear, kRenderViewPointFrustumFar);
    m_engine->setProjection(projection);
}

void SceneController::updateModelView(int ellapsedTimeForMove)
{
    if (m_currentRot != m_rot || m_currentTrans != m_trans) {
        if (m_viewMoveTime == 0) {
            /* immediately apply the target */
            m_currentRot = m_rot;
            m_currentTrans = m_trans;
        }
        else if (m_viewMoveTime > 0) {
            /* constant move */
            if (ellapsedTimeForMove >= m_viewMoveTime) {
                m_currentRot = m_rot;
                m_currentTrans = m_trans;
            }
            else {
                m_currentTrans = m_viewMoveStartTrans.lerp(m_trans, (btScalar) ellapsedTimeForMove / m_viewMoveTime);
                m_currentRot = m_viewMoveStartRot.slerp(m_rot, (btScalar) ellapsedTimeForMove / m_viewMoveTime);
            }
        }
        else {
            /* calculate difference */
            btVector3 trans = m_trans;
            trans -= m_currentTrans;
            float diff1 = trans.length2();
            btQuaternion rot = m_rot;
            rot -= m_currentRot;
            float diff2 = rot.length2();
            if (diff1 > RENDER_MINMOVEDIFF)
                m_currentTrans = m_currentTrans.lerp(m_trans, 1.0f - RENDER_MOVESPEEDRATE); /* current * 0.9 + target * 0.1 */
            else
                m_currentTrans = m_trans;
            if (diff2 > RENDER_MINSPINDIFF)
                m_currentRot = m_currentRot.slerp(m_rot, 1.0f - RENDER_SPINSPEEDRATE); /* current * 0.9 + target * 0.1 */
            else
                m_currentRot = m_rot;
        }
    }
    m_transMatrix.setRotation(m_currentRot);
    m_transMatrix.setOrigin(m_currentTrans + m_cameraTrans);
    m_engine->setModelView(m_transMatrix);
}

void SceneController::prerenderScene()
{
    sortRenderOrder();
    m_engine->setViewport(m_width, m_height);
    m_engine->prerender(m_objects, m_order, m_numModel);
}

void SceneController::renderScene()
{
    if (!isViewMoving())
        m_viewMoveTime = -1;
    m_engine->render(m_objects, m_order, m_numModel, m_stage);
}

void SceneController::renderBulletForDebug()
{
    m_engine->renderRigidBodies(&m_bullet);
}

void SceneController::renderPMDObjectsForDebug()
{
    for (int i = 0; i < m_numModel; i++) {
        PMDObject *object = m_objects[i];
        if (object->isEnable()) {
            object->renderDebug();
        }
    }
}

void SceneController::sortRenderOrder()
{
    if (m_numModel == 0)
        return;

    int size = 0;
    for (int i = 0; i < m_numModel; i++) {
        PMDObject *object = m_objects[i];
        if (!object->isEnable() || !object->allowMotionFileDrop())
            continue;
        btVector3 pos = object->getPMDModel()->getCenterBone()->getTransform().getOrigin();
        pos = m_transMatrix * pos;
        m_depth[size].distance = pos.z();
        m_depth[size].id = i;
        size++;
    }
    qsort(m_depth, size, sizeof(RenderDepth), compareDepth);
    for (int i = 0; i < size; i++)
        m_order[i] = m_depth[i].id;
    for (int i = 0; i < m_numModel; i++) {
        PMDObject *object = m_objects[i];
        if (!object->isEnable() || !object->allowMotionFileDrop())
            m_order[size++] = i;
    }
}

} /* namespace */

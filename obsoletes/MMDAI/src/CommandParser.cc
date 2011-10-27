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

namespace MMDAI {

/* arg2floatArray: parse float array from string */
static bool arg2floatArray(float *dst, int len, const char *arg)
{
    int n = 0;
    char *buf = NULL, *p = NULL, *psave = NULL;

    size_t allocated = sizeof(char) * MMDAIStringLength(arg);
    buf = static_cast<char *>(MMDAIMemoryAllocate(allocated + 1));
    if (buf == NULL)
        return false;

    MMDAIStringCopy(buf, arg, allocated);
    n = 0;
    for (p = MMDAIStringGetToken(buf, "(,)", &psave); p != NULL ; p = MMDAIStringGetToken(NULL, "(,)", &psave)) {
        if (n < len)
            dst[n] = MMDAIStringToFloat(p);
        n++;
    }
    MMDAIMemoryRelease(buf);

    return n == len;
}

/* arg2pos: get position from string */
static bool arg2pos(btVector3 &dst, const char *arg)
{
    float f[3] = { 0.0f, 0.0f, 0.0f };

    if (arg2floatArray(f, 3, arg) == false)
        return false;

    dst.setZero();
    dst.setValue(f[0], f[1], f[2]);

    return true;
}

/* arg2rot: get rotation from string */
static bool arg2rot(btQuaternion &dst, const char *arg)
{
    float angle[3] = { 0.0f, 0.0f, 0.0f };

    if (arg2floatArray(angle, 3, arg) == false)
        return false;

    dst.setEulerZYX(MMDAIMathRadian(angle[2]), MMDAIMathRadian(angle[1]), MMDAIMathRadian(angle[0]));

    return true;
}

CommandParser::CommandParser(SceneController *controller, IResourceFactory *factory)
    : m_controller(controller),
      m_factory(factory)
{
}

CommandParser::~CommandParser()
{
    m_controller = 0;
    m_factory = 0;
}

bool CommandParser::parse(const char *command, char **argv, int argc)
{
    PMDObject *object = NULL;
    IModelLoader *modelLoader = NULL;
    ILipSyncLoader *lipSyncLoader = NULL;
    IMotionLoader *motionLoader = NULL;
    float float3[3] = { 0.0f, 0.0f, 0.0f };
    bool ret = true;
    btVector3 pos(0.0f, 0.0f, 0.0f);
    btQuaternion rot;
    rot.setEulerZYX(0.0f, 0.0f, 0.0f);

    if (MMDAIStringEquals(command, ISceneEventHandler::kModelAddCommand)) {
        if (argc < 2 || argc > 6) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        if (argc >= 3) {
            if (arg2pos(pos, argv[2]) == false) {
                MMDAILogWarn("%s: not a position string: %s", command, argv[2]);
                return false;
            }
        }
        if (argc >= 4) {
            if (arg2rot(rot, argv[3]) == false) {
                MMDAILogWarn("%s: not a rotation string: %s", command, argv[3]);
                return false;
            }
        }
        modelLoader = m_factory->createModelLoader(argv[1]);
        lipSyncLoader = m_factory->createLipSyncLoader(argv[1]);
        ret = m_controller->addModel(argv[0], modelLoader, lipSyncLoader, &pos, &rot,
                                     argc >= 5 ? argv[4] : NULL, argc >= 6 ? argv[5] : NULL);
        m_factory->releaseModelLoader(modelLoader);
        m_factory->releaseLipSyncLoader(lipSyncLoader);
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kModelChangeCommand)) {
        /* change model */
        if (argc != 2) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            modelLoader = m_factory->createModelLoader(argv[1]);
            lipSyncLoader = m_factory->createLipSyncLoader(argv[1]);
            ret = m_controller->changeModel(object, modelLoader, lipSyncLoader);
            m_factory->releaseModelLoader(modelLoader);
            m_factory->releaseLipSyncLoader(lipSyncLoader);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kModelDeleteCommand)) {
        /* delete model */
        if (argc != 1) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            m_controller->deleteModel(object);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kMotionAddCommand)) {
        /* add motion */
        bool full = true; /* full */
        bool once = true; /* once */
        bool enableSmooth = true; /* enableSmooth */
        bool enableRepos = true; /* enableRePos */
        float priority = MotionManager::kDefaultPriority;
        if (argc < 3 || argc > 8) {
            MMDAILogWarn("%s: too few arguments", command);
            return false;
        }
        if (argc >= 4) {
            if (MMDAIStringEquals(argv[3], "FULL")) {
                full = true;
            }
            else if (MMDAIStringEquals(argv[3], "PART")) {
                full = false;
            }
            else {
                MMDAILogWarn("%s: 4th argument should be \"FULL\" or \"PART\"", command);
                return false;
            }
        }
        if (argc >= 5) {
            if (MMDAIStringEquals(argv[4], "ONCE")) {
                once = true;
            }
            else if (MMDAIStringEquals(argv[4], "LOOP")) {
                once = false;
            }
            else {
                MMDAILogWarn("%s: 5th argument should be \"ONCE\" or \"LOOP\"", command);
                return false;
            }
        }
        if (argc >= 6) {
            if (MMDAIStringEquals(argv[5], "ON")) {
                enableSmooth = true;
            }
            else if (MMDAIStringEquals(argv[5], "OFF")) {
                enableSmooth = false;
            }
            else {
                MMDAILogWarn("%s: 6th argument should be \"ON\" or \"OFF\"", command);
                return false;
            }
        }
        if (argc >= 7) {
            if (MMDAIStringEquals(argv[6], "ON")) {
                enableRepos = true;
            }
            else if (MMDAIStringEquals(argv[6], "OFF")) {
                enableRepos = false;
            }
            else {
                MMDAILogWarn("%s: 7th argument should be \"ON\" or \"OFF\"", command);
                return false;
            }
        }
        if (argc >= 8) {
            MMDAIStringToFloat(argv[7]);
        }
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            motionLoader = m_factory->createMotionLoader(argv[2]);
            ret = m_controller->addMotion(object, argv[1], motionLoader, full, once, enableSmooth, enableRepos, priority);
            m_factory->releaseMotionLoader(motionLoader);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kMotionChangeCommand)) {
        /* change motion */
        if (argc != 3) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            motionLoader = m_factory->createMotionLoader(argv[2]);
            ret = m_controller->changeMotion(object, argv[1], motionLoader);
            m_factory->releaseMotionLoader(motionLoader);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    } else if (MMDAIStringEquals(command, ISceneEventHandler::kMotionDeleteCommand)) {
        /* delete motion */
        if (argc != 2) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            return m_controller->deleteMotion(object, argv[1]);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kMoveStartCommand)) {
        /* start moving */
        bool local = false;
        float speed = -1.0;
        if (argc < 2 || argc > 4) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        if (arg2pos(pos, argv[1]) == false) {
            MMDAILogWarn("%s: not a position string: %s", command, argv[1]);
            return false;
        }
        if (argc >= 3) {
            if (MMDAIStringEquals(argv[2], "LOCAL")) {
                local = true;
            }
            else if (MMDAIStringEquals(argv[2], "GLOBAL")) {
                local = false;
            }
            else {
                MMDAILogWarn("%s: 3rd argument should be \"GLOBAL\" or \"LOCAL\"", command);
                return false;
            }
        }
        if (argc >= 4)
            speed = MMDAIStringToFloat(argv[3]);
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            m_controller->startMove(object, pos, local, speed);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kMoveStopCommand)) {
        /* stop moving */
        if (argc != 1) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            m_controller->stopMove(object);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kRotateStartCommand)) {
        /* start rotation */
        bool local = false;
        float speed = -1.0;
        if (argc < 2 || argc > 4) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        if (arg2rot(rot, argv[1]) == false) {
            MMDAILogWarn("%s: not a rotation string: %s", command, argv[1]);
            return false;
        }
        if (argc >= 3) {
            if (MMDAIStringEquals(argv[2], "LOCAL")) {
                local = true;
            }
            else if (MMDAIStringEquals(argv[2], "GLOBAL")) {
                local = false;
            }
            else {
                MMDAILogWarn("%s: 3rd argument should be \"GLOBAL\" or \"LOCAL\"", command);
                return false;
            }
        }
        if (argc >= 4)
            speed = MMDAIStringToFloat(argv[3]);
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            m_controller->startRotation(object, rot, local, speed);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kRotateStopCommand)) {
        /* stop rotation */
        if (argc != 1) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            m_controller->stopRotation(object);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kTurnStartCommand)) {
        /* turn start */
        bool local = false;
        float speed = -1.0;
        if (argc < 2 || argc > 4) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        if (arg2pos(pos, argv[1]) == false) {
            MMDAILogWarn("%s: not a position string: %s", command, argv[1]);
            return false;
        }
        if (argc >= 3) {
            if (MMDAIStringEquals(argv[2], "LOCAL")) {
                local = true;
            }
            else if (MMDAIStringEquals(argv[2], "GLOBAL")) {
                local = false;
            }
            else {
                MMDAILogWarn("%s: 3rd argument should be \"GLOBAL\" or \"LOCAL\"", command);
                return false;
            }
        }
        if (argc >= 4)
            speed = MMDAIStringToFloat(argv[3]);
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            m_controller->startTurn(object, pos, local, speed);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kTurnStopCommand)) {
        /* stop turn */
        if (argc != 1) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            m_controller->stopTurn(object);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kStageCommand)) {
        /* change stage */
        if (argc != 1) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        /* PMD stage or bitmap */
        char *filename = MMDAIStringClone(argv[0]);
        if (filename == NULL) {
            return false;
        }
        bool ret = false;
        char *background = strstr(filename, ",");
        if (background == NULL) {
            modelLoader = m_factory->createModelLoader(filename);
            ret = m_controller->loadStage(modelLoader);
            m_factory->releaseModelLoader(modelLoader);
        }
        else {
            *background = '\0';
            *background++;
            IModelLoader *floorLoader = m_factory->createModelLoader(filename);
            IModelLoader *backgroundLoader = m_factory->createModelLoader(background);
            ret = m_controller->loadFloor(floorLoader) && m_controller->loadBackground(backgroundLoader);
            m_factory->releaseModelLoader(floorLoader);
            m_factory->releaseModelLoader(backgroundLoader);
        }
        MMDAIMemoryRelease(filename);
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kLightColorCommand)) {
        /* change light color */
        if (argc != 1) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        if (arg2floatArray(float3, 3, argv[0]) == false) {
            MMDAILogWarn("%s: not \"R,G,B\" value: %s", command, argv[0]);
            return false;
        }
        btVector3 color(float3[0], float3[1], float3[2]);
        m_controller->setLightColor(color);
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kLightDirectionCommand)) {
        /* change light direction */
        if (argc != 1) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        if (arg2floatArray(float3, 3, argv[0]) == false) {
            MMDAILogWarn("%s: not \"x,y,z\" value: %s", command, argv[0]);
            return false;
        }
        btVector3 direction(float3[0], float3[1], float3[2]);
        m_controller->setLightDirection(direction);
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kLipSyncStartCommand)) {
        /* start lip sync */
        if (argc != 2) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            return m_controller->startLipSync(object, argv[1]);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kLipSyncStopCommand)) {
        /* stop lip sync */
        if (argc != 1) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        object = m_controller->findObject(argv[0]);
        if (object != NULL) {
            return m_controller->stopLipSync(object);
        }
        else {
            MMDAILogWarn("specified PMD object not found: %s", argv[0]);
            return false;
        }
    }
    else if (MMDAIStringEquals(command, ISceneEventHandler::kCameraCommand)) {
        if (argc == 1) {
            motionLoader = m_factory->createMotionLoader(argv[0]);
            m_controller->loadCameraMotion(motionLoader);
            m_factory->releaseMotionLoader(motionLoader);
        }
        else if (argc < 3 || argc > 4) {
            MMDAILogWarn("%s: wrong number of arguments", command);
            return false;
        }
        else {
            btVector3 pos, rot;
            if (arg2pos(pos, argv[0]) && arg2pos(rot, argv[1])) {
                float distance = MMDAIStringToFloat(argv[2]);
                float fovy = MMDAIStringToFloat(argv[3]);
                m_controller->resetCamera(pos, rot, distance, fovy);
            }
            else {
                MMDAILogWarn("%s: not a position string: %s:%s", command, argv[0], argv[1]);
            }
        }
        //float scale = MMDAIStringToFloat(argv[2]);
        //m_controller->resetLocation(pos, rot, scale);
        /* FIXME: support view timer */
    }
    return ret;
}

} /* namespace */

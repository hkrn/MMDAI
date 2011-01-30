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

#include "MMDAI/CommandParser.h"

#include <MMDFiles/MMDFiles.h>

#define MMDAGENT_MAXNUMCOMMAND    10
#define MMDAGENT_MAXCOMMANDBUFLEN 1024
#define MMDAGENT_MAXLIPSYNCBUFLEN MMDAGENT_MAXCOMMANDBUFLEN

/* arg2floatArray: parse float array from string */
static bool arg2floatArray(float *dst, int len, const char *arg)
{
  int n = 0, allocated = 0;
  char *buf = NULL, *p = NULL, *psave = NULL;

  allocated = sizeof(char) * strlen(arg);
  buf = static_cast<char *>(malloc(allocated + 1));
  if (buf == NULL)
    return false;

  strncpy(buf, arg, allocated);
  n = 0;
#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32)
  for (p = strtok_r(buf, "(,)", &psave); p != NULL ; p = strtok_r(NULL, "(,)", &psave)) {
#else
  (void)psave;
  for (p = strtok(buf, "(,)"); p != NULL ; p = strtok(NULL, "(,)")) {
#endif
    if (n < len)
      dst[n] = atof(p);
    n++;
  }
  free(buf);

  return n == len;
}

/* arg2pos: get position from string */
static bool arg2pos(btVector3 *dst, const char *arg)
{
  float f[3] = { 0.0f, 0.0f, 0.0f };

  if (arg2floatArray(f, 3, arg) == false)
    return false;

  dst->setZero();
  dst->setValue(f[0], f[1], f[2]);

  return true;
}

/* arg2rot: get rotation from string */
static bool arg2rot(btQuaternion *dst, const char *arg)
{
  float angle[3] = { 0.0f, 0.0f, 0.0f };

  if (arg2floatArray(angle, 3, arg) == false)
    return false;

  dst->setEulerZYX(MMDFILES_RAD(angle[2]), MMDFILES_RAD(angle[1]), MMDFILES_RAD(angle[0]));

  return true;
}

CommandParser::CommandParser(SceneController *controller, PMDModelLoaderFactory *factory)
  : m_controller(controller),
    m_factory(factory)
{
}

CommandParser::~CommandParser()
{
}

bool CommandParser::parse(const char *command, const char **argv, int argc)
{
  PMDObject *object = NULL;
  PMDModelLoader *pmd = NULL;
  VMDLoader *vmd = NULL;
  float tmpFloat = 0.0f, float3[3] = { 0.0f, 0.0f, 0.0f };
  bool ret = true;
  btVector3 pos;
  btQuaternion rot;

  /* divide string into arguments */
  if (argc >= MMDAGENT_MAXNUMCOMMAND) {
    g_logger.log("! Error: too many argument in command %s: %d", command, argc);
    return false;
  }

  if (strcmp(command, MMDAGENT_COMMAND_MODEL_ADD) == 0) {
    if (argc < 2 || argc > 6) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (argc >= 3) {
      if (arg2pos(&pos, argv[2]) == false) {
        g_logger.log("! Error: %s: not a position string: %s", command, argv[2]);
        return false;
      }
    }
    else {
      pos = btVector3(0.0, 0.0, 0.0);
    }
    if (argc >= 4) {
      if (arg2rot(&rot, argv[3]) == false) {
        g_logger.log("! Error: %s: not a rotation string: %s", command, argv[3]);
        return false;
      }
    }
    else {
      rot.setEulerZYX(0.0, 0.0, 0.0);
    }
    pmd = m_factory->createModelLoader(argv[1]);
    ret = m_controller->addModel(argv[0], pmd, &pos, &rot,
        argc >= 5 ? argv[4] : NULL, argc >= 6 ? argv[5] : NULL);
    m_factory->releaseModelLoader(pmd);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MODEL_CHANGE) == 0) {
    /* change model */
    if (argc != 2) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      pmd = m_factory->createModelLoader(argv[1]);
      ret = m_controller->changeModel(object, pmd);
      m_factory->releaseModelLoader(pmd);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MODEL_DELETE) == 0) {
    /* delete model */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      m_controller->deleteModel(object);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MOTION_ADD) == 0) {
    /* add motion */
    bool full = true; /* full */
    bool once = true; /* once */
    bool enableSmooth = true; /* enableSmooth */
    bool enableRepos = true; /* enableRePos */
    if (argc < 3 || argc > 7) {
      g_logger.log("! Error: %s: too few arguments", command);
      return false;
    }
    if (argc >= 4) {
      if (strcmp(argv[3], "FULL") == 0) {
        full = true;
      }
      else if (strcmp(argv[3], "PART") == 0) {
        full = false;
      }
      else {
        g_logger.log("! Error: %s: 4th argument should be \"FULL\" or \"PART\"", command);
        return false;
      }
    }
    if (argc >= 5) {
      if (strcmp(argv[4], "ONCE") == 0) {
        once = true;
      }
      else if (strcmp(argv[4], "LOOP") == 0) {
        once = false;
      }
      else {
        g_logger.log("! Error: %s: 5th argument should be \"ONCE\" or \"LOOP\"", command);
        return false;
      }
    }
    if (argc >= 6) {
      if (strcmp(argv[5], "ON") == 0) {
        enableSmooth = true;
      }
      else if (strcmp(argv[5], "OFF") == 0) {
        enableSmooth = false;
      }
      else {
        g_logger.log("! Error: %s: 6th argument should be \"ON\" or \"OFF\"", command);
        return false;
      }
    }
    if (argc >= 7) {
      if (strcmp(argv[6], "ON") == 0) {
        enableRepos = true;
      }
      else if (strcmp(argv[6], "OFF") == 0) {
        enableRepos = false;
      }
      else {
        g_logger.log("! Error: %s: 7th argument should be \"ON\" or \"OFF\"", command);
        return false;
      }
    }
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      vmd = m_factory->createMotionLoader(argv[2]);
      ret = m_controller->addMotion(object, argv[1], vmd, full, once, enableSmooth, enableRepos);
      m_factory->releaseMotionLoader(vmd);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MOTION_CHANGE) == 0) {
    /* change motion */
    if (argc != 3) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      vmd = m_factory->createMotionLoader(argv[2]);
      ret = m_controller->changeMotion(object, argv[1], vmd);
      m_factory->releaseMotionLoader(vmd);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  } else if (strcmp(command, MMDAGENT_COMMAND_MOTION_DELETE) == 0) {
    /* delete motion */
    if (argc != 2) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      return m_controller->deleteMotion(object, argv[1]);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MOVE_START) == 0) {
    /* start moving */
    bool local = false;
    tmpFloat = -1.0;
    if (argc < 2 || argc > 4) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (arg2pos(&pos, argv[1]) == false) {
      g_logger.log("! Error: %s: not a position string: %s", command, argv[1]);
      return false;
    }
    if (argc >= 3) {
      if (strcmp(argv[2], "LOCAL") == 0) {
        local = true;
      }
      else if (strcmp(argv[2], "GLOBAL") == 0) {
        local = false;
      }
      else {
        g_logger.log("! Error: %s: 3rd argument should be \"GLOBAL\" or \"LOCAL\"", command);
        return false;
      }
    }
    if (argc >= 4)
      tmpFloat = atof(argv[3]);
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      m_controller->startMove(object, &pos, local, tmpFloat);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MOVE_STOP) == 0) {
    /* stop moving */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      m_controller->stopMove(object);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_ROTATE_START) == 0) {
    /* start rotation */
    bool local = false;
    tmpFloat = -1.0;
    if (argc < 2 || argc > 4) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (arg2rot(&rot, argv[1]) == false) {
      g_logger.log("! Error: %s: not a rotation string: %s", command, argv[1]);
      return false;
    }
    if (argc >= 3) {
      if (strcmp(argv[2], "LOCAL") == 0) {
        local = true;
      }
      else if (strcmp(argv[2], "GLOBAL") == 0) {
        local = false;
      }
      else {
        g_logger.log("! Error: %s: 3rd argument should be \"GLOBAL\" or \"LOCAL\"", command);
        return false;
      }
    }
    if (argc >= 4)
      tmpFloat = (float) atof(argv[3]);
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      m_controller->startRotation(object, &rot, local, tmpFloat);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_ROTATE_STOP) == 0) {
    /* stop rotation */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      m_controller->stopRotation(object);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_TURN_START) == 0) {
    /* turn start */
    bool local = false;
    tmpFloat = -1.0;
    if (argc < 2 || argc > 4) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (arg2pos(&pos, argv[1]) == false) {
      g_logger.log("! Error: %s: not a position string: %s", command, argv[1]);
      return false;
    }
    if (argc >= 3) {
      if (strcmp(argv[2], "LOCAL") == 0) {
        local = true;
      }
      else if (strcmp(argv[2], "GLOBAL") == 0) {
        local = false;
      }
      else {
        g_logger.log("! Error: %s: 3rd argument should be \"GLOBAL\" or \"LOCAL\"", command);
        return false;
      }
    }
    if (argc >= 4)
      tmpFloat = atof(argv[3]);
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      m_controller->startTurn(object, &pos, local, tmpFloat);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_TURN_STOP) == 0) {
    /* stop turn */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      m_controller->stopTurn(object);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_STAGE) == 0) {
    /* change stage */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    /* pmd or bitmap */
    char *filename = strdup(argv[0]);
    if (filename == NULL) {
      return false;
    }
    bool ret = false;
    char *background = strstr(filename, ",");
    if (background == NULL) {
      pmd = m_factory->createModelLoader(filename);
      ret = m_controller->loadStage(pmd);
    }
    else {
      *background = '\0';
      *background++;
      PMDModelLoader *floorPMD = m_factory->createModelLoader(filename);
      PMDModelLoader *backgroundPMD = m_factory->createModelLoader(background);
      ret = m_controller->loadFloor(floorPMD) && m_controller->loadBackground(backgroundPMD);
      free(filename);
      return ret;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_LIGHTCOLOR) == 0) {
    /* change light color */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (arg2floatArray(float3, 3, argv[0]) == false) {
      g_logger.log("! Error: %s: not \"R,G,B\" value: %s", command, argv[0]);
      return false;
    }
    m_controller->changeLightColor(float3[0], float3[1], float3[2]);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_LIGHTDIRECTION) == 0) {
    /* change light direction */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (arg2floatArray(float3, 3, argv[0]) == false) {
      g_logger.log("! Error: %s: not \"x,y,z\" value: %s", command, argv[0]);
      return false;
    }
    m_controller->changeLightDirection(float3[0], float3[1], float3[2]);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_LIPSYNC_START) == 0) {
    /* start lip sync */
    if (argc != 2) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      return m_controller->startLipSync(object, argv[1]);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_LIPSYNC_STOP) == 0) {
    /* stop lip sync */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObject(argv[0]);
    if (object != NULL) {
      return m_controller->stopLipSync(object);
    }
    else {
      g_logger.log("! Error: specified PMD object not found: %s", argv[0]);
      return false;
    }
  }
  return ret;
}

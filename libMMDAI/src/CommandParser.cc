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

#include "CommandParser.h"

#include "btBulletDynamicsCommon.h"

#define MMDAGENT_MAXNUMCOMMAND    10
#define MMDAGENT_MAXCOMMANDBUFLEN 1024
#define MMDAGENT_MAXLIPSYNCBUFLEN MMDAGENT_MAXCOMMANDBUFLEN

/* strtokWithPattern: strtok with given pattern */
static char *strtokWithPattern(char *str,
                               const char *delim,
                               char left_paren,
                               char right_paren,
                               int mode,
                               char **strsave)
{
  char *p = NULL, *from = NULL, c;

  if (str != NULL) {
    *strsave = str;
  }

  /* find start point */
  p = *strsave;
  while (*p != '\0' && strchr(delim, *p)) p++;
  if (*p == '\0') return NULL; /* no token left */

  /* if mode == 1, exit here */
  if (mode == 1) {
    *strsave = p;
    return p;
  }

  /* copy to ret_buf until end point is found */
  c = *p;
  if (c == left_paren) {
    p++;
    if (*p == '\0') return NULL;
    from = p;
    while ((c = *p) != '\0' &&
           ((c != right_paren) || (*(p + 1) != '\0' && !strchr(delim, *(p + 1))))) p++;
    /* if quotation not terminated, allow the rest as one token */
  } else {
    from = p;
    while ((c = *p) != '\0' && (!strchr(delim, c))) p++;
  }
  if (*p != '\0') {
    *p = '\0';
    p++;
  }
  *strsave = p;

  return from;
}

/* strtokWithDoubleQuotation: strtok with double quotation */
static char *strtokWithDoubleQuotation(char *str, const char *delim, char **strsave)
{
  return strtokWithPattern(str, delim, '\"', '\"', 0, strsave);
}

/* arg2floatArray: parse float array from string */
static bool arg2floatArray(float *dst, int len, const char *arg)
{
  int n = 0, allocated = 0;
  char *buf = NULL, *p = NULL, *psave = NULL;

  allocated = sizeof(char) * strlen(arg);
  buf = (char *) malloc(allocated + 1);
  strncpy(buf, arg, allocated);
  n = 0;
  for (p = strtok_r(buf, "(,)", &psave); p != NULL ; p = strtok_r(NULL, "(,)", &psave)) {
    if (n < len)
      dst[n] = atof(p);
    n++;
  }
  free(buf);

  if (n != len)
    return false;
  else
    return true;
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

CommandParser::CommandParser(SceneController *controller)
  : m_controller(controller)
{
}

CommandParser::~CommandParser()
{
}

bool CommandParser::parse(const char *command, const char *arguments)
{
  PMDObject *object = NULL;
  static char argv[MMDAGENT_MAXNUMCOMMAND][MMDAGENT_MAXCOMMANDBUFLEN]; /* static buffer */
  int argc = 0, allocated = 0;
  char *buf = NULL, *tmpStr1 = NULL, *tmpStr2 = NULL;
  bool tmpBool1 = false, tmpBool2 = false, tmpBool3 = false, tmpBool4 = false;
  float tmpFloat = 0.0f, tmpFloatList[3] = { 0.0f, 0.0f, 0.0f };
  btVector3 tmpPos;
  btQuaternion tmpRot;

  /* divide string into arguments */
  if (arguments == NULL) {
    g_logger.log("<%s>", command);
    argc = 0;
  }
  else {
    g_logger.log("<%s|%s>", command, arguments);
    buf = strdup(arguments);
    for (tmpStr1 = strtokWithDoubleQuotation(buf, "|", &tmpStr2);
      tmpStr1 != NULL;
      tmpStr1 = strtokWithDoubleQuotation(NULL, "|", &tmpStr2)) {
      if (argc >= MMDAGENT_MAXNUMCOMMAND) {
        g_logger.log("! Error: too many argument in command %s: %s", command, arguments);
        free(buf);
        return false;
      }
      memset(argv[argc], 0, MMDAGENT_MAXCOMMANDBUFLEN);
      strncpy(argv[argc], tmpStr1, MMDAGENT_MAXCOMMANDBUFLEN - 1);
      argc++;
    }
    free(buf);
  }

  if (strcmp(command, MMDAGENT_COMMAND_MODEL_ADD) == 0) {
    tmpStr1 = NULL;
    tmpStr2 = NULL;
    if (argc < 2 || argc > 6) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (argc >= 3) {
      if (arg2pos(&tmpPos, argv[2]) == false) {
        g_logger.log("! Error: %s: not a position string: %s", command, argv[2]);
        return false;
      }
    }
    else {
      tmpPos = btVector3(0.0, 0.0, 0.0);
    }
    if (argc >= 4) {
      if (arg2rot(&tmpRot, argv[3]) == false) {
        g_logger.log("! Error: %s: not a rotation string: %s", command, argv[3]);
        return false;
      }
    }
    else {
      tmpRot.setEulerZYX(0.0, 0.0, 0.0);
    }
    if (argc >= 5) {
      tmpStr1 = argv[4];
    }
    if (argc >= 6) {
      tmpStr2 = argv[5];
    }
    return m_controller->addModel(argv[0], argv[1], &tmpPos, &tmpRot, tmpStr1, tmpStr2);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MODEL_CHANGE) == 0) {
    /* change model */
    if (argc != 2) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObjectByAlias(argv[0]);
    return m_controller->changeModel(object, argv[1]);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MODEL_DELETE) == 0) {
    /* delete model */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObjectByAlias(argv[0]);
    m_controller->deleteModel(object);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MOTION_ADD) == 0) {
    /* add motion */
    tmpBool1 = true; /* full */
    tmpBool2 = true; /* once */
    tmpBool3 = true; /* enableSmooth */
    tmpBool4 = true; /* enableRePos */
    if (argc < 3 || argc > 7) {
      g_logger.log("! Error: %s: too few arguments", command);
      return false;
    }
    if (argc >= 4) {
      if (strcmp(argv[3], "FULL") == 0) {
        tmpBool1 = true;
      }
      else if (strcmp(argv[3], "PART") == 0) {
        tmpBool1 = false;
      }
      else {
        g_logger.log("! Error: %s: 4th argument should be \"FULL\" or \"PART\"", command);
        return false;
      }
    }
    if (argc >= 5) {
      if (strcmp(argv[4], "ONCE") == 0) {
        tmpBool2 = true;
      }
      else if (strcmp(argv[4], "LOOP") == 0) {
        tmpBool2 = false;
      }
      else {
        g_logger.log("! Error: %s: 5th argument should be \"ONCE\" or \"LOOP\"", command);
        return false;
      }
    }
    if (argc >= 6) {
      if (strcmp(argv[5], "ON") == 0) {
        tmpBool3 = true;
      }
      else if (strcmp(argv[5], "OFF") == 0) {
        tmpBool3 = false;
      }
      else {
        g_logger.log("! Error: %s: 6th argument should be \"ON\" or \"OFF\"", command);
        return false;
      }
    }
    if (argc >= 7) {
      if (strcmp(argv[6], "ON") == 0) {
        tmpBool4 = true;
      }
      else if (strcmp(argv[6], "OFF") == 0) {
        tmpBool4 = false;
      }
      else {
        g_logger.log("! Error: %s: 7th argument should be \"ON\" or \"OFF\"", command);
        return false;
      }
    }
    object = m_controller->findPMDObjectByAlias(argv[0]);
    return m_controller->addMotion(object, argv[1] , argv[2], tmpBool1, tmpBool2, tmpBool3, tmpBool4);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MOTION_CHANGE) == 0) {
    /* change motion */
    if (argc != 3) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObjectByAlias(argv[0]);
    return m_controller->changeMotion(object, argv[1], argv[2]);
  } else if (strcmp(command, MMDAGENT_COMMAND_MOTION_DELETE) == 0) {
    /* delete motion */
    if (argc != 2) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObjectByAlias(argv[0]);
    return m_controller->deleteMotion(object, argv[1]);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MOVE_START) == 0) {
    /* start moving */
    tmpBool1 = false;
    tmpFloat = -1.0;
    if (argc < 2 || argc > 4) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (arg2pos(&tmpPos, argv[1]) == false) {
      g_logger.log("! Error: %s: not a position string: %s", command, argv[1]);
      return false;
    }
    if (argc >= 3) {
      if (strcmp(argv[2], "LOCAL") == 0) {
        tmpBool1 = true;
      }
      else if (strcmp(argv[2], "GLOBAL") == 0) {
        tmpBool1 = false;
      }
      else {
        g_logger.log("! Error: %s: 3rd argument should be \"GLOBAL\" or \"LOCAL\"", command);
        return false;
      }
    }
    if (argc >= 4)
      tmpFloat = atof(argv[3]);
    object = m_controller->findPMDObjectByAlias(argv[0]);
    m_controller->startMove(object, &tmpPos, tmpBool1, tmpFloat);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_MOVE_STOP) == 0) {
    /* stop moving */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObjectByAlias(argv[0]);
    m_controller->stopMove(object);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_ROTATE_START) == 0) {
    /* start rotation */
    tmpBool1 = false;
    tmpFloat = -1.0;
    if (argc < 2 || argc > 4) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (arg2rot(&tmpRot, argv[1]) == false) {
      g_logger.log("! Error: %s: not a rotation string: %s", command, argv[1]);
      return false;
    }
    if (argc >= 3) {
      if (strcmp(argv[2], "LOCAL") == 0) {
        tmpBool1 = true;
      }
      else if (strcmp(argv[2], "GLOBAL") == 0) {
        tmpBool1 = false;
      }
      else {
        g_logger.log("! Error: %s: 3rd argument should be \"GLOBAL\" or \"LOCAL\"", command);
        return false;
      }
    }
    if (argc >= 4)
      tmpFloat = (float) atof(argv[3]);
    object = m_controller->findPMDObjectByAlias(argv[0]);
    m_controller->startRotation(object, &tmpRot, tmpBool1, tmpFloat);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_ROTATE_STOP) == 0) {
    /* stop rotation */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObjectByAlias(argv[0]);
    m_controller->stopRotation(object);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_TURN_START) == 0) {
    /* turn start */
    tmpBool1 = false;
    tmpFloat = -1.0;
    if (argc < 2 || argc > 4) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (arg2pos(&tmpPos, argv[1]) == false) {
      g_logger.log("! Error: %s: not a position string: %s", command, argv[1]);
      return false;
    }
    if (argc >= 3) {
      if (strcmp(argv[2], "LOCAL") == 0) {
        tmpBool1 = true;
      }
      else if (strcmp(argv[2], "GLOBAL") == 0) {
        tmpBool1 = false;
      }
      else {
        g_logger.log("! Error: %s: 3rd argument should be \"GLOBAL\" or \"LOCAL\"", command);
        return false;
      }
    }
    if (argc >= 4)
      tmpFloat = atof(argv[3]);
    object = m_controller->findPMDObjectByAlias(argv[0]);
    m_controller->startTurn(object, &tmpPos, tmpBool1, tmpFloat);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_TURN_STOP) == 0) {
    /* stop turn */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObjectByAlias(argv[0]);
    m_controller->stopTurn(object);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_SOUND_START) == 0) {
    /* start sound */
    if (argc != 2) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    //m_controller->startSound(argv[0], argv[1], true);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_SOUND_STOP) == 0) {
    /* stop sound */
    if (argc < 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    //m_controller->stopSound(argv[0]);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_STAGE) == 0) {
    /* change stage */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    /* pmd or bitmap */
    tmpStr1 = strstr(argv[0], ",");
    if (tmpStr1 == NULL) {
      return m_controller->loadStage(argv[0]);
    }
    else {
      (*tmpStr1) = '\0';
      tmpStr1++;
      if (m_controller->loadFloor(argv[0]) && m_controller->loadBackground(tmpStr1))
        return true;
      else
        return false;
    }
  }
  else if (strcmp(command, MMDAGENT_COMMAND_LIGHTCOLOR) == 0) {
    /* change light color */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (arg2floatArray(tmpFloatList, 3, argv[0]) == false) {
      g_logger.log("! Error: %s: not \"R,G,B\" value: %s", command, argv[0]);
      return false;
    }
    m_controller->changeLightColor(tmpFloatList[0], tmpFloatList[1], tmpFloatList[2]);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_LIGHTDIRECTION) == 0) {
    /* change light direction */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    if (arg2floatArray(tmpFloatList, 3, argv[0]) == false) {
      g_logger.log("! Error: %s: not \"x,y,z\" value: %s", command, argv[0]);
      return false;
    }
    m_controller->changeLightDirection(tmpFloatList[0], tmpFloatList[1], tmpFloatList[2]);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_LIPSYNC_START) == 0) {
    /* start lip sync */
    if (argc != 2) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObjectByAlias(argv[0]);
    return m_controller->startLipSync(object, argv[1]);
  }
  else if (strcmp(command, MMDAGENT_COMMAND_LIPSYNC_STOP) == 0) {
    /* stop lip sync */
    if (argc != 1) {
      g_logger.log("! Error: %s: wrong number of arguments", command);
      return false;
    }
    object = m_controller->findPMDObjectByAlias(argv[0]);
    return m_controller->stopLipSync(object);
  }
  return true;
}

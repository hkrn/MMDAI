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

#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include "MMDAI/SceneController.h"
#include "MMDAI/PMDModelLoaderFactory.h"

/* command names */
#define MMDAGENT_COMMAND_MODEL_ADD      "MODEL_ADD"
#define MMDAGENT_COMMAND_MODEL_DELETE   "MODEL_DELETE"
#define MMDAGENT_COMMAND_MODEL_CHANGE   "MODEL_CHANGE"
#define MMDAGENT_COMMAND_MOTION_ADD     "MOTION_ADD"
#define MMDAGENT_COMMAND_MOTION_DELETE  "MOTION_DELETE"
#define MMDAGENT_COMMAND_MOTION_CHANGE  "MOTION_CHANGE"
#define MMDAGENT_COMMAND_MOVE_START     "MOVE_START"
#define MMDAGENT_COMMAND_MOVE_STOP      "MOVE_STOP"
#define MMDAGENT_COMMAND_TURN_START     "TURN_START"
#define MMDAGENT_COMMAND_TURN_STOP      "TURN_STOP"
#define MMDAGENT_COMMAND_ROTATE_START   "ROTATE_START"
#define MMDAGENT_COMMAND_ROTATE_STOP    "ROTATE_STOP"
#define MMDAGENT_COMMAND_SOUND_START    "SOUND_START"
#define MMDAGENT_COMMAND_SOUND_STOP     "SOUND_STOP"
#define MMDAGENT_COMMAND_STAGE          "STAGE"
#define MMDAGENT_COMMAND_FLOOR          "FLOOR"
#define MMDAGENT_COMMAND_BACKGROUND     "BACKGROUND"
#define MMDAGENT_COMMAND_LIGHTCOLOR     "LIGHTCOLOR"
#define MMDAGENT_COMMAND_LIGHTDIRECTION "LIGHTDIRECTION"
#define MMDAGENT_COMMAND_LIPSYNC_START  "LIPSYNC_START"
#define MMDAGENT_COMMAND_LIPSYNC_STOP   "LIPSYNC_STOP"

class CommandParser
{
public:
  explicit CommandParser(SceneController *controller, PMDModelLoaderFactory *factory);
  ~CommandParser();

  bool parse(const char *command, const char **argv, int argc);

private:
  SceneController *m_controller;
  PMDModelLoaderFactory *m_factory;
};

#endif // COMMANDPARSER_H

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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

#ifndef SCENEEVENTHANDLER_H
#define SCENEEVENTHANDLER_H

/* event names */
#define MMDAGENT_EVENT_MODEL_ADD      "MODEL_EVENT_ADD"
#define MMDAGENT_EVENT_MODEL_DELETE   "MODEL_EVENT_DELETE"
#define MMDAGENT_EVENT_MODEL_CHANGE   "MODEL_EVENT_CHANGE"
#define MMDAGENT_EVENT_MOTION_ADD     "MOTION_EVENT_ADD"
#define MMDAGENT_EVENT_MOTION_DELETE  "MOTION_EVENT_DELETE"
#define MMDAGENT_EVENT_MOTION_CHANGE  "MOTION_EVENT_CHANGE"
#define MMDAGENT_EVENT_MOTION_LOOP    "MOTION_EVENT_LOOP"
#define MMDAGENT_EVENT_MOVE_START     "MOVE_EVENT_START"
#define MMDAGENT_EVENT_MOVE_STOP      "MOVE_EVENT_STOP"
#define MMDAGENT_EVENT_TURN_START     "TURN_EVENT_START"
#define MMDAGENT_EVENT_TURN_STOP      "TURN_EVENT_STOP"
#define MMDAGENT_EVENT_ROTATE_START   "ROTATE_EVENT_START"
#define MMDAGENT_EVENT_ROTATE_STOP    "ROTATE_EVENT_STOP"
#define MMDAGENT_EVENT_SOUND_START    "SOUND_EVENT_START"
#define MMDAGENT_EVENT_SOUND_STOP     "SOUND_EVENT_STOP"
#define MMDAGENT_EVENT_STAGE          "STAGE"
#define MMDAGENT_EVENT_FLOOR          "FLOOR"
#define MMDAGENT_EVENT_BACKGROUND     "BACKGROUND"
#define MMDAGENT_EVENT_LIGHTCOLOR     "LIGHTCOLOR"
#define MMDAGENT_EVENT_LIGHTDIRECTION "LIGHTDIRECTION"
#define MMDAGENT_EVENT_LIPSYNC_START  "LIPSYNC_EVENT_START"
#define MMDAGENT_EVENT_LIPSYNC_STOP   "LIPSYNC_EVENT_STOP"
#define MMDAGENT_EVENT_KEY            "KEY"

class SceneEventHandler
{
public:
  virtual ~SceneEventHandler() {}

  virtual void handleEventMessage(const char *eventType, const char *format, ...) = 0;
};

#endif // SCENEEVENTHANDLER_H

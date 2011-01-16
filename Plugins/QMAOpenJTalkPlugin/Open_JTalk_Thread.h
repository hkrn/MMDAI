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

#ifndef OPEN_JTALK_THREAD_H_
#define OPEN_JTALK_THREAD_H_

#include "Open_JTalk.h"

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "CommandDispatcher.h"

#define OPENJTALKTHREAD_EVENTSTART "SYNTH_EVENT_START"
#define OPENJTALKTHREAD_EVENTSTOP  "SYNTH_EVENT_STOP"
#define OPENJTALKTHREAD_COMMANDLIP "LIPSYNC_START"

/* Open_JTalk_Event: input message buffer */
typedef struct _Open_JTalk_Event {
  char *event;
  struct _Open_JTalk_Event *next;
} Open_JTalk_Event;

/* Open_JTalk_EventQueue: queue of Open_JTalk_Event */
typedef struct _Open_JTalk_EventQueue {
  Open_JTalk_Event *head;
  Open_JTalk_Event *tail;
} Open_JTalk_EventQueue;

/* Open_JTalk_Thread: thread for Open JTalk */
class Open_JTalk_Thread : public QThread
{
public:
  /* Open_JTalk_Thraed: thread constructor */
  Open_JTalk_Thread(CommandDispatcher *dispatcher);

  /* ~Open_JTalk_Thread: thread destructor */
  ~Open_JTalk_Thread();

  /* loadAndStart: load models and start thread */
  void load(const char *dicDir, const char *config);

  /* isStarted: check running */
  bool isStarted();

  /* set_synth_parameter: set buffer for synthesis (chara|style|text) */
  void setSynthParameter(const char *str);

  /* stop: barge-in function */
  void stop();

  /* sendStartEventMessage: send start event message to MMDAgent */
  void sendStartEventMessage(const char *str);

  /* sendStopEventMessage: send stop event message to MMDAgent */
  void sendStopEventMessage(const char *str);

  /* sendLipCommandMessage: send lipsync command message to MMDAgent */
  void sendLipCommandMessage(const char *chara, const char *lip);

protected:
  void run();

private:
  Open_JTalk m_openJTalk; /* Japanese TTS system */
  Open_JTalk_EventQueue m_bufferQueue; /* buffer for synthesis (chara|style|text) */
  CommandDispatcher *m_dispathcer;

  QMutex m_mutex;
  QWaitCondition m_cond;
  volatile bool m_running;

  int m_numModels;     /* number of models */
  char **m_modelNames; /* model names */
  int m_numStyles;     /* number of styles */
  char **m_styleNames; /* style names */

  /* initialize: initialize thread */
  void initialize();

  /* clear: free thread */
  void clear();
};

#endif

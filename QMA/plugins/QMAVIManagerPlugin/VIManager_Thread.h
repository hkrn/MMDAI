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

#ifndef VIMANAGER_THREAD_H_
#define VIMANAGER_THREAD_H_

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "CommandDispatcher.h"
#include "VIManager.h"

/* VIManager_Event: input message buffer */
typedef struct _VIManager_Event {
  char *type;
  char *args;
  struct _VIManager_Event *next;
} VIManager_Event;

/* VIManager_Event_initialize: initialize input message buffer */
void VIManager_Event_initialize(VIManager_Event *e, const char *type, const char *args);

/* VIManager_Event_clear: free input message buffer */
void VIManager_Event_clear(VIManager_Event *e);

/* VIManager_EventQueue: queue of VIManager_Event */
typedef struct _VIManager_EventQueue {
  VIManager_Event *head;
  VIManager_Event *tail;
} VIManager_EventQueue;

/* VIManager_EventQueue_initialize: initialize queue */
void VIManager_EventQueue_initialize(VIManager_EventQueue *q);

/* VIManager_EventQueue_clear: free queue */
void VIManager_EventQueue_clear(VIManager_EventQueue *q);

/* VIManager_EventQueue_enqueue: enqueue */
void VIManager_EventQueue_enqueue(VIManager_EventQueue *q, const char *type, const char *args);

/* VIManager_EventQueue_dequeue: dequeue */
int VIManager_EventQueue_dequeue(VIManager_EventQueue *q, char *type, char *args);

/* VIManager_Thread: thread of VIManager */
class VIManager_Thread : public QThread
{
private:
  QMutex m_mutex;

  QWaitCondition m_cond;

  volatile bool m_running;

  VIManager m_vim; /* voicd interaction manager */

  VIManager_EventQueue eventQueue; /* queue of input message */

  /* initialize: initialize thread */
  void initialize();

  /* clear: free thread */
  void clear();

public:
  /* VIManager_Thraed: thread constructor */
  VIManager_Thread(CommandDispatcher *dispathcer);

  /* ~VIManager_Thread: thread destructor */
  ~VIManager_Thread();

  /* loadAndStart: load FST and start thread */
  void load(const char *filename);

  void stop();

  /* isStarted: check running */
  bool isStarted();

  /* enqueueBuffer: enqueue buffer to check */
  void enqueueBuffer(const char *type, const char *args);

  /* stateTransition: thread loop for VIManager */
  void stateTransition();

  /* sendMessage: send event message to MMDAgent */
  void sendMessage(const char *str1, const char *str2);

protected:
  void run();

private:
  CommandDispatcher *m_dispathcer;
};

#endif

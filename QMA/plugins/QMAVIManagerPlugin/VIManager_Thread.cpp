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

#include "QMAVIManagerPlugin.h"
#include "VIManager.h"
#include "VIManager_Thread.h"

/* VIManager_Event_initialize: initialize input message buffer */
void VIManager_Event_initialize(VIManager_Event *e, const char *type, const char *args)
{
    if (type != NULL)
        e->type = strdup(type);
    else
        e->type = NULL;
    if (args != NULL)
        e->args = strdup(args);
    else
        e->args = NULL;
    e->next = NULL;
}

/* VIManager_Event_clear: free input message buffer */
void VIManager_Event_clear(VIManager_Event *e)
{
    if (e->type != NULL)
        free(e->type);
    if (e->args != NULL)
        free(e->args);
    VIManager_Event_initialize(e, NULL, NULL);
}

/* VIManager_EventQueue_initialize: initialize queue */
void VIManager_EventQueue_initialize(VIManager_EventQueue *q)
{
    q->head = NULL;
    q->tail = NULL;
}

/* VIManager_EventQueue_clear: free queue */
void VIManager_EventQueue_clear(VIManager_EventQueue *q)
{
    VIManager_Event *tmp1, *tmp2;

    for (tmp1 = q->head; tmp1 != NULL; tmp1 = tmp2) {
        tmp2 = tmp1->next;
        VIManager_Event_clear(tmp1);
        free(tmp1);
    }
    VIManager_EventQueue_initialize(q);
}

/* VIManager_EventQueue_enqueue: enqueue */
void VIManager_EventQueue_enqueue(VIManager_EventQueue *q, const char *type, const char *args)
{
    if (q->tail == NULL) {
        q->tail = (VIManager_Event *) calloc(1, sizeof (VIManager_Event));
        VIManager_Event_initialize(q->tail, type, args);
        q->head = q->tail;
    } else {
        q->tail->next = (VIManager_Event *) calloc(1, sizeof (VIManager_Event));
        VIManager_Event_initialize(q->tail->next, type, args);
        q->tail = q->tail->next;
    }
}

/* VIManager_EventQueue_dequeue: dequeue */
int VIManager_EventQueue_dequeue(VIManager_EventQueue *q, char *type, char *args)
{
    VIManager_Event *tmp;

    if (q->head == NULL) {
        if (type != NULL)
            strcpy(type, "");
        if (args != NULL)
            strcpy(type, "");
        return 0;
    }
    if (type != NULL)
        strcpy(type, q->head->type);
    if (args != NULL)
        strcpy(args, q->head->args);
    tmp = q->head->next;
    VIManager_Event_clear(q->head);
    free(q->head);
    q->head = tmp;
    if (tmp == NULL)
        q->tail = NULL;
    return 1;
}

/* VIManager_Thread::initialize: initialize thread */
void VIManager_Thread::initialize()
{
    VIManager_EventQueue_initialize(&eventQueue);
}

/* VIManager_Thread::clear: free thread */
void VIManager_Thread::clear()
{
    VIManager_EventQueue_clear(&eventQueue);
    initialize();
}

/* VIManager_Thread::VIManager_Thread: thread constructor */
VIManager_Thread::VIManager_Thread(QMAVIManagerPlugin *dispatcher)
    : m_running(false),
      m_dispathcer(dispatcher)
{
    initialize();
}

/* VIManager_Thread::~VIManager_Thread: thread destructor */
VIManager_Thread::~VIManager_Thread()
{
    m_running = false;
    clear();
}

/* VIManager_Thread::loadAndStart: load FST and start thread */
void VIManager_Thread::load(QTextStream &stream)
{
    /* load FST for VIManager */
    if (m_vim.load(stream) == 0)
        return;
}

void VIManager_Thread::stop()
{
    m_running = false;
}

/* VIManager_Thread::isStarted: check running */
bool VIManager_Thread::isStarted()
{
    return m_running;
}

/* VIManager_Thread::enqueueBuffer: enqueue buffer to check */
void VIManager_Thread::enqueueBuffer(const char *type, const char *args)
{
    m_mutex.lock();
    /* save event */
    VIManager_EventQueue_enqueue(&eventQueue, type, args);
    m_cond.wakeAll();
    m_mutex.unlock();
}

/* VIManager_Thread::stateTransition: thread loop for VIManager */
void VIManager_Thread::run()
{
    char itype[VIMANAGER_MAXBUFLEN];
    char iargs[VIMANAGER_MAXBUFLEN];
    char otype[VIMANAGER_MAXBUFLEN];
    char oargs[VIMANAGER_MAXBUFLEN];
    int remain = 1;

    m_running = true;

    /* first epsilon step */
    while (m_vim.transition(VIMANAGER_EPSILON, NULL, otype, oargs)) {
        if (strcmp(otype, VIMANAGER_EPSILON) != 0)
            sendMessage(otype, oargs);
    }

    while (m_running) {
        /* wait transition event */
        m_mutex.lock();
        m_cond.wait(&m_mutex);
        m_mutex.unlock();

        do {
            /* load input message */
            remain = VIManager_EventQueue_dequeue(&eventQueue, itype, iargs);
            if (remain == 0)
                break;

            /* state transition with input symbol */
            m_vim.transition(itype, iargs, otype, oargs);
            if (strcmp(otype, VIMANAGER_EPSILON) != 0)
                sendMessage(otype, oargs);

            /* state transition with epsilon */
            while (m_vim.transition(VIMANAGER_EPSILON, NULL, otype, oargs)) {
                if (strcmp(otype, VIMANAGER_EPSILON) != 0)
                    sendMessage(otype, oargs);
            }
        } while (remain);
    }
}

/* VIManager_Thread::sendMessage: send message to MMDAgent */
void VIManager_Thread::sendMessage(const char *str1, const char *str2)
{
    m_dispathcer->sendCommand(str1, strdup(str2));
}

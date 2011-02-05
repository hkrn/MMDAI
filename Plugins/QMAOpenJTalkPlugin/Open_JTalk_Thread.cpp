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

#include <string.h>
#include <stdlib.h>

#include "Open_JTalk_Thread.h"

/* Open_JTalk_Event_initialize: initialize input message buffer */
void Open_JTalk_Event_initialize(Open_JTalk_Event *e, const char *str);

/* Open_JTalk_Event_clear: free input message buffer */
void Open_JTalk_Event_clear(Open_JTalk_Event *e);

/* Open_JTalk_EventQueue_initialize: initialize queue */
void Open_JTalk_EventQueue_initialize(Open_JTalk_EventQueue *q);

/* Open_JTalk_EventQueue_clear: free queue */
void Open_JTalk_EventQueue_clear(Open_JTalk_EventQueue *q);

/* Open_JTalk_EventQueue_enqueue: enqueue */
void Open_JTalk_EventQueue_enqueue(Open_JTalk_EventQueue *q, const char *str);

/* Open_JTalk_EventQueue_dequeue: dequeue */
int Open_JTalk_EventQueue_dequeue(Open_JTalk_EventQueue *q, char *str);

/* get_token_from_fp: get token from a file pointer */
static int get_token_from_fp(FILE * fp, char *str)
{
  char c;
  int i;

  str[0] = '\0';

  if (feof(fp))
    return 0;

  c = fgetc(fp);
  while (c == '\n' || c == '\r' || c == '\t' || c == ' ') {
    if (feof(fp))
      return 0;
    c = getc(fp);
  }

  for (i = 0; c != '\n' && c != '\r' && c != '\t' && c != ' ' && !feof(fp); i++) {
    str[i] = c;
    c = fgetc(fp);
  }

  str[i] = '\0';
  return i;
}

/* Open_JTalk_Event_initialize: initialize input message buffer */
void Open_JTalk_Event_initialize(Open_JTalk_Event *e, const char *str)
{
  if (str != NULL)
    e->event = strdup(str);
  else
    e->event = NULL;
  e->next = NULL;
}

/* Open_JTalk_Event_clear: free input message buffer */
void Open_JTalk_Event_clear(Open_JTalk_Event *e)
{
  if (e->event != NULL)
    free(e->event);
  Open_JTalk_Event_initialize(e, NULL);
}

/* Open_JTalk_EventQueue_initialize: initialize queue */
void Open_JTalk_EventQueue_initialize(Open_JTalk_EventQueue *q)
{
  q->head = NULL;
  q->tail = NULL;
}

/* Open_JTalk_EventQueue_clear: free queue */
void Open_JTalk_EventQueue_clear(Open_JTalk_EventQueue *q)
{
  Open_JTalk_Event *tmp1, *tmp2;

  for (tmp1 = q->head; tmp1 != NULL; tmp1 = tmp2) {
    tmp2 = tmp1->next;
    Open_JTalk_Event_clear(tmp1);
    free(tmp1);
  }
  Open_JTalk_EventQueue_initialize(q);
}

/* Open_JTalk_EventQueue_enqueue: enqueue */
void Open_JTalk_EventQueue_enqueue(Open_JTalk_EventQueue *q, const char *str)
{
  if (q->tail == NULL) {
    q->tail = (Open_JTalk_Event *) calloc(1, sizeof (Open_JTalk_Event));
    Open_JTalk_Event_initialize(q->tail, str);
    q->head = q->tail;
  } else {
    q->tail->next = (Open_JTalk_Event *) calloc(1, sizeof (Open_JTalk_Event));
    Open_JTalk_Event_initialize(q->tail->next, str);
    q->tail = q->tail->next;
  }
}

/* Open_JTalk_EventQueue_dequeue: dequeue */
int Open_JTalk_EventQueue_dequeue(Open_JTalk_EventQueue *q, char *str)
{
  Open_JTalk_Event *tmp;

  if (q->head == NULL) {
    if (str != NULL)
      str[0] = '\0';
    return 0;
  }
  if (str != NULL)
    strcpy(str, q->head->event);
  tmp = q->head->next;
  Open_JTalk_Event_clear(q->head);
  free(q->head);
  q->head = tmp;
  if (tmp == NULL) q->tail = NULL;
  return 1;
}

/* Open_JTalk_Thread::initialize: initialize thread */
void Open_JTalk_Thread::initialize()
{
  Open_JTalk_EventQueue_initialize(&m_bufferQueue);
  m_numModels = 0;
  m_modelNames = NULL;
  m_numStyles = 0;
  m_styleNames = NULL;
}

/* Open_JTalk_Thread::clear: free thread */
void Open_JTalk_Thread::clear()
{
  int i;
  Open_JTalk_EventQueue_clear(&m_bufferQueue);
  m_running = false;
  /* free model names */
  if (m_numModels > 0) {
    for (i = 0; i < m_numModels; i++)
      free(m_modelNames[i]);
    free(m_modelNames);
  }
  m_numModels = 0;
  m_modelNames = NULL;
  /* free style names */
  if (m_numStyles > 0) {
    for (i = 0; i < m_numStyles; i++)
      free(m_styleNames[i]);
    free(m_styleNames);
  }
  m_numStyles = 0;
  m_styleNames = NULL;
}

/* Open_JTalk_Thread::Open_JTalk_Thread: thread constructor */
Open_JTalk_Thread::Open_JTalk_Thread(CommandDispatcher *dispatcher)
  : m_dispathcer(dispatcher)
{
  initialize();
}

/* Open_JTalk_Thread::~Open_JTalk_Thread: thread destructor */
Open_JTalk_Thread::~Open_JTalk_Thread()
{
  clear();
}

/* Open_JTalk_Thread::loadAndStart: load models and start thread */
void Open_JTalk_Thread::load(const char *dicDir, const char *config)
{
  int i, j, k;
  char buff[OPENJTALK_MAXBUFLEN];
  FILE *fp;

  double *weights;

  /* load config file */
  fp = fopen(config, "r");
  if (fp == NULL)
    return;
  /* number of speakers */
  i = get_token_from_fp(fp, buff);
  if (i <= 0) {
    //MessageBox(NULL, L"ERROR: Cannot load the number of models in config file of Open JTalk.", L"Error", MB_OK);
    fclose(fp);
    return;
  }
  m_numModels = atoi(buff);
  if (m_numModels <= 0) {
    //MessageBox(NULL, L"ERROR: The number of models must be positive value.", L"Error", MB_OK);
    m_numModels = 0;
    fclose(fp);
    return;
  }
  /* model directory names */
  m_modelNames = (char **) calloc(m_numModels, sizeof(char *));
  for (i = 0; i < m_numModels; i++)
    m_modelNames[i] = (char *) calloc(OPENJTALK_MAXBUFLEN, sizeof(char));
  for (i = 0; i < m_numModels; i++) {
    j = get_token_from_fp(fp, buff);
    if (j <= 0) {
      //MessageBox(NULL, L"ERROR: Cannot load model directory names in config file of Open JTalk.", L"Error", MB_OK);
      fclose(fp);
      return;
    }
    strcpy(m_modelNames[i], buff);
  }

  /* number of speaking styles */
  i = get_token_from_fp(fp, buff);
  if (i <= 0) {
    //MessageBox(NULL, L"ERROR: Cannot load the number of speaking styles in config file of Open JTalk.", L"Error", MB_OK);
    fclose(fp);
    return;
  }
  m_numStyles = atoi(buff);
  if (m_numStyles <= 0) {
    //MessageBox(NULL, L"ERROR: The number of speaking styles must be positive value.", L"Error", MB_OK);
    m_numStyles = 0;
    fclose(fp);
    return;
  }
  /* style names and weights */
  m_styleNames = (char **) calloc(m_numStyles, sizeof(char *));
  for (i = 0; i < m_numStyles; i++)
    m_styleNames[i] = (char *) calloc(OPENJTALK_MAXBUFLEN, sizeof(char));
  weights = (double *) calloc((3 * m_numModels + 4) * m_numStyles, sizeof(double));
  for (i = 0; i < m_numStyles; i++) {
    j = get_token_from_fp(fp, buff);
    if (j <= 0) {
      //MessageBox(NULL, L"ERROR: Cannot load style definitions in config file of Open JTalk.", L"Error", MB_OK);
      fclose(fp);
      free(weights);
      return;
    }
    strcpy(m_styleNames[i], buff);
    for (j = 0; j < 3 * m_numModels + 4; j++) {
      k = get_token_from_fp(fp, buff);
      if (k <= 0) {
        //MessageBox(NULL, L"ERROR: Cannot load style definitions in config file of Open JTalk.", L"Error", MB_OK);
        fclose(fp);
        free(weights);
        return;
      }
      weights[(3 * m_numModels + 4) * i + j] = (double) atof(buff);
    }
  }
  fclose(fp);

  /* load models for TTS */
  if (m_openJTalk.load(dicDir, m_modelNames, m_numModels, weights, m_numStyles) != true) {
    //MessageBox(NULL, L"ERROR: Cannot initialize Open JTalk.", L"Error", MB_OK);
    free(weights);
    return;
  }
  //setlocale(LC_CTYPE, "japanese");

  free(weights);
}

/* Open_JTalk_Thread::isStarted: check running */
bool Open_JTalk_Thread::isStarted()
{
  return m_running;
}

/* Open_JTalk_Thread::setSynthParameter: set buffer for synthesis (chara|style|text) */
void Open_JTalk_Thread::setSynthParameter(const char *str)
{
  m_mutex.lock();
  /* save character name, speaking style, and text */
  Open_JTalk_EventQueue_enqueue(&m_bufferQueue, str);
  m_cond.wakeAll();
  m_mutex.unlock();
}

/* Open_JTalk_Thread::synthesis: thread loop for TTS */
void Open_JTalk_Thread::run()
{
  char tmp1[OPENJTALK_MAXBUFLEN];
  char tmp2[OPENJTALK_MAXBUFLEN];
  char *p, *c, *s, *t;
  int s_index;
  int remain = 1;

  m_running = true;
  while (remain) {
    m_mutex.lock();
    m_cond.wait(&m_mutex);
    remain = Open_JTalk_EventQueue_dequeue(&m_bufferQueue, tmp1);
    m_mutex.unlock();

    if (remain == 0) break;

    /* get character name, style name, and text */
    p = &tmp1[0];
    c = p;
    while (*p != '\0' && *p != '|') p++;
    *(p++) = '\0';
    s = p;
    while (*p != '\0' && *p != '|') p++;
    *(p++) = '\0';
    t = p;
    /* search style index */
    for (s_index = 0; s_index < m_numStyles; s_index++)
      if (strcmp(m_styleNames[s_index], s) == 0)
        break;
    if (s_index >= m_numStyles) /* unknown style */
      s_index = 0;

    /* send SYNTH_EVENT_STOP */
    sendStartEventMessage(c);

    /* synthesize */
    if (t[0] != '\0' && c[0] != '\0') {
      m_openJTalk.setStyle(s_index);
      m_openJTalk.prepare(t);
      m_openJTalk.getPhonemeSequence(tmp2);
      if (strlen(tmp2) > 0) {
        sendLipCommandMessage(c, tmp2);
        m_openJTalk.synthesis();
      }
    }

    /* send SYNTH_EVENT_STOP */
    sendStopEventMessage(c);
  }
}

/* Open_JTalk_Thread::stop: barge-in function */
void Open_JTalk_Thread::stop()
{
  m_openJTalk.stop();
}

/* Open_JTalk_Thread::sendStartEventMessage: send start event message to MMDAgent */
void Open_JTalk_Thread::sendStartEventMessage(const char *str)
{
  char *mes;

  if(str == NULL)
    return;

  mes = strdup(str);
  m_dispathcer->sendEvent(OPENJTALKTHREAD_EVENTSTART, mes);
}

/* Open_JTalk_Thread::sendStopEventMessage: send stop event message to MMDAgent */
void Open_JTalk_Thread::sendStopEventMessage(const char *str)
{
  char *mes;

  if(str == NULL)
    return;

  mes = strdup(str);
  m_dispathcer->sendEvent(OPENJTALKTHREAD_EVENTSTOP, mes);
}

/* Open_JTalk_Thread::sendLipCommandMessage: send lipsync command message to MMDAgent */
void Open_JTalk_Thread::sendLipCommandMessage(const char *chara, const char *lip)
{
  char *mes;

  if(chara == NULL || lip == NULL) return;

  mes = (char *) malloc(sizeof(char) * (strlen(chara) + 1 + strlen(lip) + 1));
  sprintf(mes, "%s|%s", chara, lip);

  m_dispathcer->sendCommand(OPENJTALKTHREAD_COMMANDLIP, mes);
}

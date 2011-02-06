/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
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

#include <QBuffer>
#include <QDebug>
#include <QtGlobal>

#include "Open_JTalk_Thread.h"

/* MMDAgent_fgettoken: get token from file pointer (copied from MMDAgent_util.c) */
static int MMDAgent_fgettoken(FILE *fp, char *buff)
{
  int i;
  char c;

  c = fgetc(fp);
  if(c == EOF) {
    buff[0] = '\0';
    return 0;
  }

  if(c == '#') {
    for(c = fgetc(fp); c != EOF; c = fgetc(fp))
      if(c == '\n')
        return MMDAgent_fgettoken(fp, buff);
    buff[0] = '\0';
    return 0;
  }

  if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
    return MMDAgent_fgettoken(fp, buff);

  buff[0] = c;
  for(i = 1, c = fgetc(fp); c != EOF && c != '#' && c != ' ' && c != '\t' && c != '\r' && c != '\n'; c = fgetc(fp))
    buff[i++] = c;
  buff[i] = '\0';

  if(c == '#')
    fseek(fp, -1, SEEK_CUR);
  if(c == EOF)
    fseek(fp, 0, SEEK_END);

  return i;
}

/* Open_JTalk_Thread::initialize: initialize thread */
void Open_JTalk_Thread::initialize()
{
  m_speaking = false;
  m_kill = false;

  m_buffer = 0;
  m_output = new Phonon::AudioOutput(Phonon::CommunicationCategory, this);
  m_object = new Phonon::MediaObject(this);
  Phonon::createPath(m_object, m_output);

  m_charaBuff = NULL;
  m_styleBuff = NULL;
  m_textBuff = NULL;

  m_numModels = 0;
  m_modelNames = NULL;
  m_numStyles = 0;
  m_styleNames = NULL;
  m_dispathcer = NULL;
}

/* Open_JTalk_Thread::clear: free thread */
void Open_JTalk_Thread::clear()
{
  int i;

  stop();
  m_kill = true;

  /* XXX: wait synthesis event */

  if(m_charaBuff) free(m_charaBuff);
  if(m_styleBuff) free(m_styleBuff);
  if(m_textBuff) free(m_textBuff);

  /* free model names */
  if (m_numModels > 0) {
    for (i = 0; i < m_numModels; i++)
      free(m_modelNames[i]);
    free(m_modelNames);
  }

  /* free style names */
  if (m_numStyles > 0) {
    for (i = 0; i < m_numStyles; i++)
      free(m_styleNames[i]);
    free(m_styleNames);
  }

  delete m_object;
  delete m_output;
  delete m_buffer;

  initialize();
}

/* Open_JTalk_Thread::Open_JTalk_Thread: thread constructor */
Open_JTalk_Thread::Open_JTalk_Thread()
{
  initialize();
}

/* Open_JTalk_Thread::~Open_JTalk_Thread: thread destructor */
Open_JTalk_Thread::~Open_JTalk_Thread()
{
  clear();
}

/* Open_JTalk_Thread::loadAndStart: load models and start thread */
void Open_JTalk_Thread::load(CommandDispatcher *dispatcher, const char *baseDir, const char *dicDir, const char *config)
{
  int i, j, k;
  char buff[OPENJTALK_MAXBUFLEN];
  char path[OPENJTALK_MAXBUFLEN];
  FILE *fp;
  bool err = false;

  double *weights;

  /* load config file */
  fp = fopen(config, "r");
  if (fp == NULL)
    return;

  /* number of speakers */
  i = MMDAgent_fgettoken(fp, buff);
  if (i <= 0) {
    qWarning() << "Error: cannot load the number of models in config file of Open JTalk.";
    fclose(fp);
    clear();
    return;
  }
  m_numModels = atoi(buff);
  if (m_numModels <= 0) {
    qWarning() <<  "Error: the number of models must be positive value.";
    fclose(fp);
    clear();
    return;
  }

  /* model directory names */
  m_modelNames = (char **) malloc(sizeof(char *) * m_numModels);
  for (i = 0; i < m_numModels; i++) {
    j = MMDAgent_fgettoken(fp, buff);
    if (j <= 0)
      err = true;
    snprintf(path, sizeof(path), "%s/%s", baseDir, buff);
    m_modelNames[i] = strdup(path);
  }
  if (err) {
    qWarning() << "Error: cannot load model directory names in config file of Open JTalk.";
    fclose(fp);
    clear();
    return;
  }

  /* number of speaking styles */
  i = MMDAgent_fgettoken(fp, buff);
  if (i <= 0) {
    qWarning() << "Error: cannot load the number of speaking styles in config file of Open JTalk.";
    fclose(fp);
    clear();
    return;
  }
  m_numStyles = atoi(buff);
  if (m_numStyles <= 0) {
    qWarning() << "Error: the number of speaking styles must be positive value.";
    m_numStyles = 0;
    fclose(fp);
    clear();
    return;
  }

  /* style names and weights */
  m_styleNames = (char **) calloc(m_numStyles, sizeof(char *));
  weights = (double *) calloc((3 * m_numModels + 4) * m_numStyles, sizeof(double));
  for (i = 0; i < m_numStyles; i++) {
    j = MMDAgent_fgettoken(fp, buff);
    if(j <= 0)
      err = true;
    m_styleNames[i] = strdup(buff);
    for (j = 0; j < 3 * m_numModels + 4; j++) {
      k = MMDAgent_fgettoken(fp, buff);
      if (k <= 0)
        err = true;
      weights[(3 * m_numModels + 4) * i + j] = atof(buff);
    }
  }
  fclose(fp);
  if(err) {
    qWarning() << "Error: cannot load style definitions in config file of Open JTalk.";
    free(weights);
    clear();
    return;
  }

  /* load models for TTS */
  if (m_openJTalk.load(dicDir, m_modelNames, m_numModels, weights, m_numStyles) != true) {
    qWarning() << "Error: cannot initialize Open JTalk.";
    free(weights);
    clear();
    return;
  }

  m_dispathcer = dispatcher;

  free(weights);
}

/* Open_JTalk_Thread::stopAndRelease: stop thread and free Open JTalk */
void Open_JTalk_Thread::stopAndRelease()
{
  clear();
}

/* Open_JTalk_Thread::start: main thread loop for TTS */
void Open_JTalk_Thread::run()
{
  char lip[OPENJTALK_MAXBUFLEN];
  char *chara, *style, *text;
  int index;

  while (m_kill == false) {
    m_mutex.lock();
    m_cond.wait(&m_mutex);
    if (m_kill) { /* for destruction (Open_JTalk_Thread::clear) */
      m_mutex.unlock();
      break;
    }
    chara = strdup(m_charaBuff);
    style = strdup(m_styleBuff);
    text = strdup(m_textBuff);
    m_speaking = true;
    m_mutex.unlock();

    /* search style index */
    for (index = 0; index < m_numStyles; index++)
      if (strcmp(m_styleNames[index], style) == 0)
        break;
    if (index >= m_numStyles) /* unknown style */
      index = 0;

    /* send SYNTH_EVENT_START */
    sendStartEventMessage(chara);

    /* synthesize */
    m_openJTalk.setStyle(index);
    m_openJTalk.prepare(text);
    m_openJTalk.getPhonemeSequence(lip);
    if (strlen(lip) > 0) {
      sendLipCommandMessage(chara, lip);
      delete m_buffer;
      m_buffer = new QBuffer();
      m_buffer->open(QBuffer::ReadWrite);
      m_openJTalk.synthesis(m_buffer);
      m_object->setCurrentSource(m_buffer);
      m_object->play();
      msleep(m_object->totalTime());
    }
    sendStopEventMessage(chara);

    if(chara) free(chara);
    if(style) free(style);
    if(text) free(text);
  }
}

/* Open_JTalk_Thread::isRunning: check running */
bool Open_JTalk_Thread::isRunning()
{
  return m_kill == false;
}

/* Open_JTalk_Thread::isSpeaking: check speaking */
bool Open_JTalk_Thread::isSpeaking()
{
  return m_speaking;
}

/* checkCharacter: check speaking character */
bool Open_JTalk_Thread::checkCharacter(const char *chara)
{
  bool ret;

  /* check */
  if(m_charaBuff == NULL || chara == NULL || !isRunning())
    return false;

  m_mutex.lock();
  /* save character name, speaking style, and text */
  ret = strcmp(m_charaBuff, chara) == 0;
  m_cond.wakeAll();
  m_mutex.unlock();

  return ret;
}

/* Open_JTalk_Thread::synthesis: start synthesis */
void Open_JTalk_Thread::synthesis(const char *chara, const char *style, const char *text)
{
  /* check */
  if(!isRunning() || chara == NULL || style == NULL || text == NULL ||
     strlen(chara) <= 0 || strlen(style) <= 0 || strlen(text) <= 0)
    return;

  m_mutex.lock();
  /* save character name, speaking style, and text */
  if(m_charaBuff) free(m_charaBuff);
  if(m_styleBuff) free(m_styleBuff);
  if(m_textBuff) free(m_textBuff);
  m_charaBuff = strdup(chara);
  m_styleBuff = strdup(style);
  m_textBuff = strdup(text);
  m_cond.wakeAll();
  m_mutex.unlock();
}

/* Open_JTalk_Thread::stop: barge-in function */
void Open_JTalk_Thread::stop()
{
  if(isRunning())
    m_openJTalk.stop();
}

/* Open_JTalk_Thread::sendStartEventMessage: send start event message to MMDAgent */
void Open_JTalk_Thread::sendStartEventMessage(const char *str)
{
  m_dispathcer->sendEvent(OPENJTALKTHREAD_EVENTSTART, strdup(str));
}

/* Open_JTalk_Thread::sendStopEventMessage: send stop event message to MMDAgent */
void Open_JTalk_Thread::sendStopEventMessage(const char *str)
{
  m_dispathcer->sendEvent(OPENJTALKTHREAD_EVENTSTOP, strdup(str));
}

/* Open_JTalk_Thread::sendLipCommandMessage: send lipsync command message to MMDAgent */
void Open_JTalk_Thread::sendLipCommandMessage(const char *chara, const char *lip)
{
  char *mes1;
  char *mes2;

  if(chara == NULL || lip == NULL) return;

  mes1 = strdup(OPENJTALKTHREAD_COMMANDLIP);
  mes2 = (char *) malloc(sizeof(char) * (strlen(chara) + 1 + strlen(lip) + 1));
  sprintf(mes2, "%s|%s", chara, lip);

  m_dispathcer->sendCommand(OPENJTALKTHREAD_COMMANDLIP, mes2);
}

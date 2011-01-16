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

#include "Julius_Thread.h"

/* callback_recog_begin: callback for beginning of recognition */
static void callback_recog_begin(Recog * /* recog */, void *data)
{
  qDebug("recog_begin called");
  Julius_Thread *j = (Julius_Thread *) data;
  j->sendMessage(JULIUSTHREAD_EVENTSTART, NULL);
}

/* callback_result_final: callback for recognitional result */
static void callback_result_final(Recog *recog, void *data)
{
  int i;
  int word_num;
  int first;
  Sentence *s = NULL;
  RecogProcess *r = NULL;
  WORD_ID *word = NULL;
  static char str[JULIUSTHREAD_MAXBUFLEN];
  Julius_Thread *j = (Julius_Thread *) data;

  qDebug("recog_result_final called");
  /* get status */
  r = recog->process_list;
  if (!r->live)
    return;
  if (r->result.status < 0)
    return;

  s = &(r->result.sent[0]);
  word = s->word;
  word_num = s->word_num;
  strcpy(str, "");
  first = 1;
  for (i = 0; i < word_num; i++) {
    if (strlen(r->lm->winfo->woutput[word[i]]) > 0) {
      if (first == 0)
        strcat(str, ",");
      strncat(str, r->lm->winfo->woutput[word[i]], JULIUSTHREAD_MAXBUFLEN);
      if (first == 1)
        first = 0;
    }
  }

  if (first == 0) {
    j->sendMessage(JULIUSTHREAD_EVENTSTOP, str);
  }
}

/* Julius_Thread::Julius_Thread: thread constructor */
Julius_Thread::Julius_Thread(CommandDispatcher *dispatcher, QMAJuliusInitializer *initializer)
  : m_dispatcher(dispatcher),
    m_initializer(initializer)
{
}

/* Julius_Thread::~Julius_Thread: thread destructor */
Julius_Thread::~Julius_Thread()
{
}

void Julius_Thread::run()
{
  Recog *recog = m_initializer->getRecognizeEngine();
  if (recog != NULL) {
    /* register callback functions */
    callback_add(recog, CALLBACK_EVENT_RECOGNITION_BEGIN, ::callback_recog_begin, this);
    callback_add(recog, CALLBACK_RESULT, ::callback_result_final, this);
    if (!j_adin_init(recog) || j_open_stream(recog, NULL) != 0) {
      qWarning("Failed initializing or opening stream");
      return;
    }
    qDebug("j_recognize_stream started");
    int ret = j_recognize_stream(recog);
    qDebug("j_recognize_stream finished: %d", ret);
  }
}

/* Julius_Thread::sendMessage: send message to MMDAgent */
void Julius_Thread::sendMessage(const char *str1, const char *str2)
{
  m_dispatcher->sendCommand(str1, str2);
}

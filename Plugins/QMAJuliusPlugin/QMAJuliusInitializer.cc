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

#include "QMAJuliusInitializer.h"

QMAJuliusInitializer::QMAJuliusInitializer(QStringList &conf)
  : m_conf(conf),
  m_jconf(NULL),
  m_recog(NULL)
{
}

QMAJuliusInitializer::~QMAJuliusInitializer()
{
  if (m_jconf != NULL) {
    j_jconf_free(m_jconf);
    m_jconf = NULL;
  }
  if (m_recog != NULL) {
    j_close_stream(m_recog);
    j_recog_free(m_recog);
    m_recog = NULL;
  }
}

void QMAJuliusInitializer::run()
{
  int argc = m_conf.length();
  if (argc == 0)
    return;
  char **argv = (char **)calloc(1, sizeof(char *) * (argc + 1));
  if (argv == NULL)
    return;
  for (int i = 0; i < argc; i++) {
    argv[i + 1] = strdup(m_conf.at(i).toUtf8().constData());
  }

  /* load config file */
  m_jconf = j_config_load_args_new(argc, argv);
  for (int i = 0; i < argc; i++) {
    char *arg = argv[i + 1];
    if (arg != NULL)
      free(arg);
  }
  free(argv);

  if (m_jconf == NULL) {
    qWarning("Failed loading jconf.txt");
    return;
  }

  /* create instance */
  m_recog = j_create_instance_from_jconf(m_jconf);
  if (m_recog != NULL) {
    qDebug("Julius is ready");
  }
  else {
    qWarning("Failed creating an instance Julius");
  }
}

Recog *QMAJuliusInitializer::getRecognizeEngine() const
{
  return m_recog;
}

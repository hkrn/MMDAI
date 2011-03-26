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

#include <QtConcurrentRun>
#include <QDir>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>

#include <MMDME/Common.h>

#include "QMAJuliusPlugin.h"
#undef open // undef stddef.h in Julius

class QMAJuliusPluginThread : public QThread
{
public:
  QMAJuliusPluginThread(Recog *recog) {
    m_recog = recog;
  }
  ~QMAJuliusPluginThread() {}

protected:
  void run() {
    int ret = j_recognize_stream(m_recog);
    MMDAILogInfo("j_recognize_stream returned %d", ret);
  }

private:
  Recog *m_recog;
};

void QMAJuliusPluginBeginRecognition(Recog *recog, void *ptr)
{
  Q_UNUSED(recog);
  QMAJuliusPlugin *plugin = static_cast<QMAJuliusPlugin*>(ptr);
  QStringList arguments;
  emit plugin->eventPost("RECOG_EVENT_START", arguments);
}

void QMAJuliusPluginGetRecognitionResult(Recog *recog, void *ptr)
{
  /* get status */
  RecogProcess *process = recog->process_list;
  if (!process->live || process->result.status < 0)
    return;

  QString ret;
  QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
  Sentence *sentence = &process->result.sent[0];
  WORD_ID *words = sentence->word;
  int nwords = sentence->word_num;
  bool first = true;
  for (int i = 0; i < nwords; i++) {
    char *str = process->lm->winfo->woutput[words[i]];
    if (MMDAIStringLength(str) > 0) {
      if (!first)
        ret += ",";
      ret += codec->toUnicode(str);
      if (first)
        first = false;
    }
  }

  if (!first) {
    QMAJuliusPlugin *plugin = static_cast<QMAJuliusPlugin*>(ptr);
    QStringList arguments;
    arguments << ret;
    MMDAILogInfo("Recognized as %s", ret.toUtf8().constData());
    emit plugin->eventPost("RECOG_EVENT_STOP", arguments);
  }
  else {
    MMDAILogDebugString("Failed recognition");
  }
}

QMAJuliusPlugin::QMAJuliusPlugin(QObject *parent)
  : QMAPlugin(parent),
  m_thread(NULL),
  m_jconf(NULL),
  m_recog(NULL)
{
  connect(&m_watcher, SIGNAL(finished()), this, SLOT(initialized()));
  m_tray.show();
}

QMAJuliusPlugin::~QMAJuliusPlugin()
{
  delete m_thread;
  if (m_recog != NULL) {
    j_close_stream(m_recog);
    j_recog_free(m_recog);
    m_recog = NULL;
  }
  if (m_jconf != NULL) {
    j_jconf_free(m_jconf);
    m_jconf = NULL;
  }
  m_tray.hide();
}

void QMAJuliusPlugin::initialize(MMDAI::SceneController *controller)
{
  Q_UNUSED(controller);
    m_watcher.setFuture(QtConcurrent::run(this, &QMAJuliusPlugin::initializeRecognitionEngine));
    if (QSystemTrayIcon::supportsMessages())
      m_tray.showMessage(tr("Started initialization of Julius"),
                         tr("Please wait a moment until end of initialization of Julius engine."
                            "This process takes about 10-20 seconds."));
}

void QMAJuliusPlugin::start()
{
  /* do nothing */
}

void QMAJuliusPlugin::stop()
{
  /* do nothing */
}

void QMAJuliusPlugin::receiveCommand(const QString &command, const QStringList &arguments)
{
  Q_UNUSED(command);
  Q_UNUSED(arguments);
  /* do nothing */
}

void QMAJuliusPlugin::receiveEvent(const QString &type, const QStringList &arguments)
{
  Q_UNUSED(type);
  Q_UNUSED(arguments);
  /* do nothing */
}

void QMAJuliusPlugin::update(const QRect &rect, const QPoint &pos, const double delta)
{
  Q_UNUSED(rect);
  Q_UNUSED(pos);
  Q_UNUSED(delta);
  /* do nothing */
}

void QMAJuliusPlugin::prerender()
{
  /* do nothing */
}

void QMAJuliusPlugin::postrender()
{
  /* do nothing */
}

void QMAJuliusPlugin::initialized()
{
  bool result = m_watcher.future();
  if (result) {
    if (QSystemTrayIcon::supportsMessages())
      m_tray.showMessage(tr("Completed initialization of Julius"),
                         tr("You can now talk with the models."));
    m_thread = new QMAJuliusPluginThread(m_recog);
    m_thread->start();
  }
  else {
    if (QSystemTrayIcon::supportsMessages())
      m_tray.showMessage(tr("Failed initialization of Julius"),
                         tr("Recognization feature is disabled."),
                         QSystemTrayIcon::Warning);
  }
}

void QMAJuliusPlugin::sendEvent(const char *type, char *arguments)
{
  if (arguments != NULL) {
    QTextCodec *codec = QTextCodec::codecForName("EUC-JP");
    QStringList argv;
    argv << codec->toUnicode(arguments, strlen(arguments));
    emit eventPost(QString(type), argv);
    free(arguments);
  }
}

bool QMAJuliusPlugin::initializeRecognitionEngine()
{
  const char *path = NULL;
  char buf[BUFSIZ];
  QDir dir = QDir::searchPaths("mmdai").at(0) + "/AppData/Julius";

  path = dir.absoluteFilePath("lang_m/web.60k.8-8.bingramv5.gz").toUtf8().constData();
  MMDAIStringFormat(buf, sizeof(buf), "-d %s", path);
  buf[sizeof(buf) - 1] = 0;
  m_jconf = j_config_load_string_new(buf);
  if (m_jconf == NULL) {
    MMDAILogWarn("Failed loading language model for Julius: %s", buf);
    return false;
  }
  path = dir.absoluteFilePath("lang_m/web.60k.htkdic").toUtf8().constData();
  MMDAIStringFormat(buf, sizeof(buf), "-v %s", path);
  buf[sizeof(buf) - 1] = 0;
  if (j_config_load_string(m_jconf, buf) < 0) {
    MMDAILogWarn("Failed loading system dictionary for Julius: %s", buf);
    return false;
  }
  path = dir.absoluteFilePath("phone_m/clustered.mmf.16mix.all.julius.binhmm").toUtf8().constData();
  MMDAIStringFormat(buf, sizeof(buf), "-h %s", path);
  buf[sizeof(buf) - 1] = 0;
  if (j_config_load_string(m_jconf, buf) < 0) {
    MMDAILogWarn("Failed loading acoustic model for Julius: %s", buf);
    return false;
  }
  path = dir.absoluteFilePath("phone_m/tri_tied.list.bin").toUtf8().constData();
  MMDAIStringFormat(buf, sizeof(buf), "-hlist %s", path);
  buf[sizeof(buf) - 1] = 0;
  if (j_config_load_string(m_jconf, buf) < 0) {
    MMDAILogWarn("Failed loading triphone list for Julius: %s", buf);
    return false;
  }
  path = dir.absoluteFilePath("jconf.txt").toUtf8().constData();
  MMDAIStringCopy(buf, path, sizeof(buf));
  buf[sizeof(buf) - 1] = 0;
  if (j_config_load_file(m_jconf, buf)) {
    MMDAILogWarn("Failed loading configuration for Julius: %s", path);
    return false;
  }

  /* create instance */
  m_recog = j_create_instance_from_jconf(m_jconf);
  if (m_recog != NULL) {
    callback_add(m_recog, CALLBACK_EVENT_RECOGNITION_BEGIN, QMAJuliusPluginBeginRecognition, this);
    callback_add(m_recog, CALLBACK_RESULT, QMAJuliusPluginGetRecognitionResult, this);
    if (!j_adin_init(m_recog) || j_open_stream(m_recog, NULL) != 0) {
      MMDAILogWarnString("Failed initialize adin or stream");
      return false;
    }
    MMDAILogInfoString("Get ready to recognize with Julius");
    return true;
  }
  else {
    MMDAILogWarnString("Failed creating an instance of Julius");
    return false;
  }
}

Q_EXPORT_PLUGIN2(qma_julius_plugin, QMAJuliusPlugin)

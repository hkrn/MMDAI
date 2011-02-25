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
  QTextCodec *codec = QTextCodec::codecForName("EUC-JP");
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

  if (first) {
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
  QDir dir = QDir::searchPaths("mmdai").at(0) + "/AppData/Julius";
  QString filename = dir.absoluteFilePath("jconf.txt");
  QFile jconf(filename);
  QStringList conf;
  if (jconf.open(QFile::ReadOnly)) {
    QTextStream stream(&jconf);
    // language model
    conf << "-d" << dir.absoluteFilePath("lang_m/web.60k.8-8.bingramv5.gz");
    // dictionary
    conf << "-v" << dir.absoluteFilePath("lang_m/web.60k.htkdic");
    // acoustic model
    conf << "-h" << dir.absoluteFilePath("phone_m/clustered.mmf.16mix.all.julius.binhmm");
    // triphone list
    conf << "-hlist" << dir.absoluteFilePath("phone_m/tri_tied.list.bin");
    while (!stream.atEnd()) {
      QString line = stream.readLine();
      QStringList pair = line.split(QRegExp("\\s+"));
      if (pair.size() == 2) {
        QString key = pair[0];
        QString value = pair[1];
        conf << key << value;
      }
      else {
        foreach (QString value, pair) {
          conf << value;
        }
      }
    }
    m_watcher.setFuture(QtConcurrent::run(this, &QMAJuliusPlugin::initializeRecognitionEngine, conf));
    if (QSystemTrayIcon::supportsMessages())
      m_tray.showMessage(tr("Started initialization of Julius"),
                         tr("Please wait a moment until end of initialization of Julius engine."
                            "This process takes about 10-20 seconds."));
  }
  else {
    MMDAILogWarn("Failed open file %s: %s",
                 jconf.fileName().toUtf8().constData(),
                 jconf.errorString().toUtf8().constData());
    m_tray.showMessage(tr("Failed initialization of Julius"),
                       tr("Cannot read Julius configuration file"),
                       QSystemTrayIcon::Warning);
  }
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

void QMAJuliusPlugin::render()
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

bool QMAJuliusPlugin::initializeRecognitionEngine(const QStringList &conf)
{
  int argc = conf.length();
  if (argc == 0) {
    MMDAILogWarnString("Julius configuration file is empty");
    return false;
  }

  size_t size = sizeof(char *) * (argc + 1);
  char **argv = static_cast<char **>(MMDAIMemoryAllocate(size));
  if (argv == NULL) {
    MMDAILogWarnString("Failed allocating memory");
    return false;
  }

  memset(argv, 0, size);
  for (int i = 0; i < argc; i++) {
    argv[i + 1] = MMDAIStringClone(conf.at(i).toUtf8().constData());
  }

  /* load config file */
  m_jconf = j_config_load_args_new(argc, argv);
  for (int i = 0; i < argc; i++) {
    char *arg = argv[i + 1];
    if (arg != NULL)
      MMDAIMemoryRelease(arg);
  }
  MMDAIMemoryRelease(argv);

  if (m_jconf == NULL) {
    MMDAILogWarnString("Failed loading Julius configuration");
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

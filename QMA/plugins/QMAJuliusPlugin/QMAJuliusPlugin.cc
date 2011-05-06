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

#include "QMAJuliusPlugin.h"

#ifdef Q_OS_WIN32
#undef open // undef stddef.h in Julius
#endif
#include <QtCore>
#include <QtGui/QApplication>
#include <MMDME/Common.h>

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
    emit plugin->eventPost("RECOG_EVENT_START", QMAPlugin::getEmptyArguments());
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
        QList<QVariant> arguments;
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
      m_thread(0),
      m_tray(0),
      m_jconf(NULL),
      m_recog(NULL)
{
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(initialized()));
}

QMAJuliusPlugin::~QMAJuliusPlugin()
{
    delete m_thread;
    delete m_tray;
    if (m_recog != NULL) {
        j_close_stream(m_recog);
        j_recog_free(m_recog);
        m_recog = NULL;
    }
    if (m_jconf != NULL) {
        j_jconf_free(m_jconf);
        m_jconf = NULL;
    }
}

void QMAJuliusPlugin::load(MMDAI::SceneController *controller, const QString &baseName)
{
    Q_UNUSED(controller);
    QFile path("MMDAITranslations:/QMAJuliusPlugin_" + QLocale::system().name());
    m_translator.load(path.fileName());
    qApp->installTranslator(&m_translator);
    m_watcher.setFuture(QtConcurrent::run(this, &QMAJuliusPlugin::initializeRecognitionEngine, baseName));
    m_tray = new QSystemTrayIcon(qApp->windowIcon(), this);
    m_tray->show();
    if (QSystemTrayIcon::supportsMessages()) {
        m_tray->showMessage(tr("Started initialization of Julius"),
                            tr("Please wait a moment until end of initialization of Julius engine."
                               "This process takes about 10-20 seconds."));
    }
}

void QMAJuliusPlugin::unload()
{
    m_tray->hide();
}

void QMAJuliusPlugin::receiveCommand(const QString &command, const QList<QVariant> &arguments)
{
    Q_UNUSED(command);
    Q_UNUSED(arguments);
    /* do nothing */
}

void QMAJuliusPlugin::receiveEvent(const QString &type, const QList<QVariant> &arguments)
{
    Q_UNUSED(type);
    Q_UNUSED(arguments);
    /* do nothing */
}

void QMAJuliusPlugin::initialized()
{
    bool result = m_watcher.future();
    if (result) {
        if (QSystemTrayIcon::supportsMessages()) {
            m_tray->showMessage(tr("Completed initialization of Julius"),
                                tr("You can now talk with the models."));
        }
        m_thread = new QMAJuliusPluginThread(m_recog);
        m_thread->start();
    }
    else {
        if (QSystemTrayIcon::supportsMessages()) {
            m_tray->showMessage(tr("Failed initialization of Julius"),
                                tr("Recognization feature is disabled."),
                                QSystemTrayIcon::Warning);
        }
    }
}

void QMAJuliusPlugin::sendEvent(const char *type, char *arguments)
{
    if (arguments != NULL) {
        QTextCodec *codec = QTextCodec::codecForName("EUC-JP");
        QList<QVariant> argv;
        argv << codec->toUnicode(arguments, strlen(arguments));
        emit eventPost(QString(type), argv);
        free(arguments);
    }
}

bool QMAJuliusPlugin::initializeRecognitionEngine(const QString &baseName)
{
    char buf[BUFSIZ];
    QByteArray path;
    QDir dir("MMDAIResources:/AppData/Julius");

    path = dir.absoluteFilePath("lang_m/web.60k.8-8.bingramv5.gz").toUtf8();
    MMDAIStringFormatSafe(buf, sizeof(buf), "-d %s", path.constData());
    m_jconf = j_config_load_string_new(buf);
    if (m_jconf == NULL) {
        MMDAILogWarn("Failed loading language model for Julius: %s", path.constData());
        return false;
    }
    path = dir.absoluteFilePath("lang_m/web.60k.htkdic").toUtf8();
    MMDAIStringFormatSafe(buf, sizeof(buf), "-v %s", path.constData());
    if (j_config_load_string(m_jconf, buf) < 0) {
        MMDAILogWarn("Failed loading system dictionary for Julius: %s", path.constData());
        return false;
    }
    path = dir.absoluteFilePath("phone_m/clustered.mmf.16mix.all.julius.binhmm").toUtf8();
    MMDAIStringFormatSafe(buf, sizeof(buf), "-h %s", path.constData());
    if (j_config_load_string(m_jconf, buf) < 0) {
        MMDAILogWarn("Failed loading acoustic model for Julius: %s", path.constData());
        return false;
    }
    path = dir.absoluteFilePath("phone_m/tri_tied.list.bin").toUtf8();
    MMDAIStringFormatSafe(buf, sizeof(buf), "-hlist %s", path.constData());
    if (j_config_load_string(m_jconf, buf) < 0) {
        MMDAILogWarn("Failed loading triphone list for Julius: %s", path.constData());
        return false;
    }
    path = dir.absoluteFilePath("jconf.txt").toUtf8();
    MMDAIStringCopySafe(buf, path.constData(), sizeof(buf));
    if (j_config_load_file(m_jconf, buf)) {
        MMDAILogWarn("Failed loading configuration for Julius: %s", path.constData());
        return false;
    }
    QFile userDict(QString("MMDAIUserData:/%1.dic").arg(baseName));
    if (userDict.exists()) {
        path = userDict.fileName().toUtf8();
        MMDAIStringCopySafe(buf, path.constData(), sizeof(buf));
        j_add_dict(m_jconf->lm_root, buf);
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

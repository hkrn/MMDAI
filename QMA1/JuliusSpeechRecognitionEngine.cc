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

#include "JuliusSpeechRecognitionEngine.h"

#include <QtCore/QtCore>
#include <QtGui/QtGui>

#ifdef Q_OS_WIN32
#include <winsock2.h>
#pragma warning(push)
#pragma warning(disable : 4819)
#endif /* Q_OS_WIN32 */
#undef TRUE
#undef FALSE
#include <julius/julius.h>
#ifdef Q_OS_WIN32
#pragma warning(pop)
#undef open /* prevent an error on QFile class */
#endif /* Q_OS_WIN32 */

class JuliusSpeechRegonitionThread : public QThread
{
public:
    JuliusSpeechRegonitionThread(Recog *recog) {
        m_recog = recog;
    }
    ~JuliusSpeechRegonitionThread() {
        m_recog = 0;
    }

    void stop() {
        j_close_stream(m_recog);
    }

protected:
    void run() {
        /* j_recognize_stream は処理をブロックするので、スレッドとして分離しておく */
        int ret = j_recognize_stream(m_recog);
        qDebug("j_recognize_stream returned %d", ret);
    }

private:
    Recog *m_recog;
};

struct JuliusSpeechRecognitionEngineInternal
{
    Jconf *jconf;
    Recog *recog;
};

static void BeginRecognition(Recog *recog, void *ptr)
{
    Q_UNUSED(recog);
    JuliusSpeechRecognitionEngine *engine = static_cast<JuliusSpeechRecognitionEngine*>(ptr);
    JuliusSpeechRecognitionEngineSendEvent(engine, JuliusSpeechRecognitionEngine::kRecogStartEvent, QString());
}

static void GetRecognitionResult(Recog *recog, void *ptr)
{
    /* 音声認識のプロセスが動いているかを確認する */
    RecogProcess *process = recog->process_list;
    if (!process->live || process->result.status < 0)
        return;

    QString ret;
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    Sentence *sentence = &process->result.sent[0];
    WORD_ID *words = sentence->word;
    int nwords = sentence->word_num;
    bool first = true;
    /* 単語ごとにわかれているので、カンマでつなげてひとつの文章にしておく */
    for (int i = 0; i < nwords; i++) {
        char *str = process->lm->winfo->woutput[words[i]];
        if (qstrlen(str) > 0) {
            if (!first)
                ret += ",";
            ret += codec->toUnicode(str);
            if (first)
                first = false;
        }
    }

    /* 単語が取得できていれば kRecogStopEvent を投げて対応するコマンドの処理を行う */
    if (!first) {
        JuliusSpeechRecognitionEngine *engine = static_cast<JuliusSpeechRecognitionEngine *>(ptr);
        qDebug("Recognized as %s", qPrintable(ret));
        JuliusSpeechRecognitionEngineSendEvent(engine, JuliusSpeechRecognitionEngine::kRecogStopEvent, ret);
    }
    else {
        qWarning("Failed recognition");
    }
}

void JuliusSpeechRecognitionEngineSendEvent(JuliusSpeechRecognitionEngine *engine,
                                            const QString &type,
                                            const QString &value)
{
    QList<QVariant> a;
    if (!value.isEmpty())
        a << value;
    emit engine->eventDidPost(type, a);
}

const QString JuliusSpeechRecognitionEngine::kRecogStartEvent = "RECOG_EVENT_START";
const QString JuliusSpeechRecognitionEngine::kRecogStopEvent = "RECOG_EVENT_STOP";

JuliusSpeechRecognitionEngine::JuliusSpeechRecognitionEngine(QObject *parent)
    : QObject(parent),
      m_thread(0),
      m_tray(0),
      m_internal(0)
{
    QFile path("MMDAITranslations:/JuliusSpeechRecognitionEngine_" + QLocale::system().name());
    m_translator.load(path.fileName());
    m_tray = new QSystemTrayIcon();
    qApp->installTranslator(&m_translator);
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(engineDidInitalize()));
}

JuliusSpeechRecognitionEngine::~JuliusSpeechRecognitionEngine()
{
    delete m_tray;
    m_tray = 0;
    release();
}

void JuliusSpeechRecognitionEngine::load(const QDir &dir, const QString &baseName)
{
    /* 音声認識エンジンの初期化が行われているかもしれないので、終了するまで待機(ブロック)する */
    if (m_watcher.isRunning())
        m_watcher.waitForFinished();
    release();
    /* 音声認識エンジン初期化の処理は重いので、非同期処理(スレッドを作成して分離)として行う */
    m_watcher.setFuture(QtConcurrent::run(this, &JuliusSpeechRecognitionEngine::initialize, dir, baseName));
#ifndef Q_OS_MAC // freeze all menu on MacOSX
    m_tray->show();
#endif
    if (QSystemTrayIcon::supportsMessages()) {
        m_tray->showMessage(tr("Started initialization of Julius"),
                            tr("Please wait a moment until end of initialization of Julius engine."
                               "This process takes about 5-10 seconds."));
    }
}

bool JuliusSpeechRecognitionEngine::initialize(const QDir &dir, const QString &baseName)
{
    char buf[BUFSIZ];
    QString path;
    QDir resdir("MMDAIResources:AppData/Julius");

    /* ログが標準出力に書き出されるので、無視しておく */
    jlog_set_output(NULL);

    /* 言語モデルの読み込み */
    path = resdir.absoluteFilePath("lang_m/web.60k.8-8.bingramv5.gz");
    qstrncpy(buf, QString("-d %1").arg(path).toLocal8Bit().constData(), sizeof(buf));
    Jconf *jconf = j_config_load_string_new(buf);
    if (!jconf) {
        qWarning("%s", qPrintable(tr("Failed loading language model for Julius: %1").arg(path)));
        return false;
    }
    /* システム辞書の読み込み */
    path = resdir.absoluteFilePath("lang_m/web.60k.htkdic");
    qstrncpy(buf, QString("-v %1").arg(path).toLocal8Bit().constData(), sizeof(buf));
    if (j_config_load_string(jconf, buf) < 0) {
        qWarning("%s", qPrintable(tr("Failed loading system dictionary for Julius: %1").arg(path)));
        return false;
    }
    /* 音響モデルの読み込み */
    path = resdir.absoluteFilePath("phone_m/clustered.mmf.16mix.all.julius.binhmm");
    qstrncpy(buf, QString("-h %1").arg(path).toLocal8Bit().constData(), sizeof(buf));
    if (j_config_load_string(jconf, buf) < 0) {
        qWarning("%s", qPrintable(tr("Failed loading acoustic model for Julius: %1").arg(path)));
        return false;
    }
    /* トライフォンの読み込み */
    path = resdir.absoluteFilePath("phone_m/tri_tied.list.bin");
    qstrncpy(buf, QString("-hlist %1").arg(path).toLocal8Bit().constData(), sizeof(buf));
    if (j_config_load_string(jconf, buf) < 0) {
        qWarning("%s", qPrintable(tr("Failed loading triphone list for Julius: %1").arg(path)));
        return false;
    }
    /* Julius 設定ファイルの読み込み */
    path = resdir.absoluteFilePath("jconf.txt").toUtf8();
    qstrncpy(buf, path.toLocal8Bit().constData(), sizeof(buf));
    if (j_config_load_file(jconf, buf)) {
        qWarning("%s", qPrintable(tr("Failed loading configuration for Julius: %1").arg(path)));
        return false;
    }
    /* ユーザ辞書の読み込み */
    QFile userDict(dir.absoluteFilePath(QString("%1.dic").arg(baseName)));
    if (userDict.exists()) {
        path = userDict.fileName().toUtf8();
        qstrncpy(buf, path.toLocal8Bit().constData(), sizeof(buf));
        j_add_dict(jconf->lm_root, buf);
    }

    /* 設定から音声認識エンジンのインスタンスを作成し、コールバックを設定しておく */
    Recog *recog = j_create_instance_from_jconf(jconf);
    if (recog) {
        callback_add(recog, CALLBACK_EVENT_RECOGNITION_BEGIN, BeginRecognition, this);
        callback_add(recog, CALLBACK_RESULT, GetRecognitionResult, this);
        if (!j_adin_init(recog) || j_open_stream(recog, NULL) != 0) {
            release();
            qWarning("%s", qPrintable(tr("Failed initialize adin or stream")));
            return false;
        }
        /* 音声認識処理はブロックするので非同期処理として行う */
        m_internal = new JuliusSpeechRecognitionEngineInternal();
        m_internal->jconf = jconf;
        m_internal->recog = recog;
        m_thread = new JuliusSpeechRegonitionThread(recog);
        m_thread->start();
        qDebug("%s", qPrintable(tr("Get ready to recognize with Julius")));
        return true;
    }
    else {
        qWarning("%s", qPrintable(tr("Failed creating an instance of Julius")));
        return false;
    }
}

void JuliusSpeechRecognitionEngine::engineDidInitalize()
{
    bool result = m_watcher.future();
    if (result) {
        if (QSystemTrayIcon::supportsMessages()) {
            m_tray->showMessage(tr("Completed initialization of Julius"),
                                tr("You can now talk with the models."));
        }
    }
    else {
        release();
        if (QSystemTrayIcon::supportsMessages()) {
            m_tray->showMessage(tr("Failed initialization of Julius"),
                                tr("Recognization feature is disabled."),
                                QSystemTrayIcon::Warning);
        }
    }
}

void JuliusSpeechRecognitionEngine::release()
{
    /* 音声認識処理スレッドの停止 */
    if (m_thread) {
        m_thread->stop();
        m_thread->wait();
    }
    delete m_thread;
    m_thread = 0;
    if (m_internal) {
        Recog *recog = m_internal->recog;
        if (recog)
            j_recog_free(recog);
        /* 何故かここでクラッシュしてしまう。もちろんメモリリークを起こしてしまう */
#if 0
        JConf *jconf = m_internal->jconf;
        if (jconf)
            j_jconf_free(jconf);
#endif
    }
    delete m_internal;
    m_internal = 0;
}

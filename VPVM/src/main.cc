/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/config.h" /* vpvl2::libraryVersionString() and vpvl2::libraryCommitRevisionString() */
#include "vpvl2/extensions/BaseApplicationContext.h"

#include "Common.h"
#include <QtQuick>
#include <QApplication>
#include <QCommandLineParser>

#include "ALAudioContext.h"
#include "ALAudioEngine.h"
#include "BoneKeyframeRefObject.h"
#include "BoneMotionTrack.h"
#include "BoneRefObject.h"
#include "CameraKeyframeRefObject.h"
#include "CameraMotionTrack.h"
#include "CameraRefObject.h"
#include "GraphicsDevice.h"
#include "Grid.h"
#include "LabelRefObject.h"
#include "LightKeyframeRefObject.h"
#include "LightMotionTrack.h"
#include "LightRefObject.h"
#include "ModelProxy.h"
#include "MorphKeyframeRefObject.h"
#include "MorphMotionTrack.h"
#include "MorphRefObject.h"
#include "MotionProxy.h"
#include "Preference.h"
#include "RenderTarget.h"
#include "ProjectProxy.h"
#include "UIAuxHelper.h"
#include "Util.h"
#include "WorldProxy.h"

using namespace vpvl2::extensions;

namespace {

class LoggerThread : public QRunnable {
public:
    static const QString &toString(QtMsgType type) {
        switch (type) {
        case QtDebugMsg: {
            static QString s("info");
            return s;
        }
        case QtWarningMsg: {
            static QString s("warning");
            return s;
        }
        case QtCriticalMsg: {
            static QString s("error");
            return s;
        }
        case QtFatalMsg: {
            static QString s("fatal");
            return s;
        }
        default: {
            static QString s("unknown");
            return s;
        }
        }
    }
    static void delegateMessage(QtMsgType type, const QMessageLogContext &context, const QString &message);

    LoggerThread()
        : m_active(true)
    {
        setAutoDelete(false);
    }
    ~LoggerThread() {
    }

    void post(const QString &message) {
        QMutexLocker locker(&m_mutex); Q_UNUSED(locker);
        m_queue.enqueue(message);
        m_cond.wakeAll();
    }
    void stop() {
        QMutexLocker locker(&m_mutex); Q_UNUSED(locker);
        m_active = false;
        m_cond.wakeAll();
    }
    void setDirectory(const QString &value) {
        m_directory = value;
    }

private:
    void run() {
        static const QString commitRevision = QString(vpvl2::libraryCommitRevisionString()).split(" ").at(1);
        QStringList stringList;
        stringList.reserve(4);
        qInstallMessageHandler(&LoggerThread::delegateMessage);
        qDebug("libvpvl2: version=%s commit=%s", vpvl2::libraryVersionString(), qPrintable(commitRevision));
        while (m_active) {
            QMutexLocker locker(&m_mutex); Q_UNUSED(locker);
            m_cond.wait(&m_mutex);
            const QDateTime &currentDateTime = QDateTime::currentDateTime();
            QFile f(m_directory.absoluteFilePath("%1.log").arg(currentDateTime.toString("yyyyMMdd")));
            f.open(QFile::Append);
            while (!m_queue.isEmpty()) {
                const QString &message = m_queue.dequeue();
                stringList.clear();
                stringList << currentDateTime.toString(Qt::ISODate);
                stringList << " ";
                stringList << message;
                stringList << "\n";
                f.write(stringList.join("").toUtf8());
#if !defined(QT_NO_DEBUG)
                fprintf(stderr, "%s", stringList.join("").toUtf8().constData());
#endif
            }
        }
        qInstallMessageHandler(0);
    }

    QMutex m_mutex;
    QWaitCondition m_cond;
    QQueue<QString> m_queue;
    QDir m_directory;
    volatile bool m_active;
} g_loggerThread;

class ApplicationBootstrapOption : public QObject {
    Q_OBJECT

public:
    Q_PROPERTY(QUrl json READ json CONSTANT FINAL)
    Q_PROPERTY(bool hasJson READ hasJson CONSTANT FINAL)

    ApplicationBootstrapOption(QCommandLineParser *parser)
        : QObject(0),
          m_parser(parser),
          m_json(QStringList() << "j" << "json", "Configuration JSON from <file> to load at startup.", "file")
    {
        parser->setApplicationDescription(QApplication::tr("VPVM (a.k.a MMDAI2) is an application to create/edit motion like MikuMikuDance (MMD)"));
        parser->addHelpOption();
        parser->addVersionOption();
        parser->addOption(m_json);
    }
    ~ApplicationBootstrapOption() {
    }

    QUrl json() const {
        return QUrl::fromLocalFile(m_parser->value(m_json));
    }
    bool hasJson() const {
        return m_parser->isSet(m_json);
    }

private:
    const QCommandLineParser *m_parser;
    QCommandLineOption m_json;
};

void LoggerThread::delegateMessage(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    Q_UNUSED(type);
    Q_UNUSED(context);
#if !defined(QT_NO_DEBUG)
    /* supress warnings of Invalid font from Context2D */
    if (message.startsWith("Context2D:")) {
        return;
    }
#endif
    g_loggerThread.post(QString("[%1] %4 in %2 at line %3").arg(toString(type)).arg(context.function).arg(context.line).arg(message));
}

static void prepareRegal()
{
    static const QByteArray kRegalEnableVariables[] = {
        "REGAL_EMULATION",
        0
    };
    static const QByteArray kRegalDisableVariables[] = {
        "REGAL_NO_EMULATION",
        0
    };
    for (int i = 0; !kRegalEnableVariables[i].isNull(); i++) {
        qputenv(kRegalEnableVariables[i], "1");
    }
    for (int i = 0; !kRegalDisableVariables[i].isNull(); i++) {
        qputenv(kRegalEnableVariables[i], "0");
    }
}

static QObject *createALAudioContext(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine);
    QObject *value = new ALAudioContext(engine);
    return value;
}

static QObject *createUIAuxHelper(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(scriptEngine);
    QObject *value = new UIAuxHelper(engine);
    return value;
}

void registerQmlTypes()
{
    qmlRegisterSingletonType<ALAudioContext>("com.github.mmdai.VPVM", 1, 0, "ALAudioContext", createALAudioContext);
    qmlRegisterType<ALAudioEngine>("com.github.mmdai.VPVM", 1, 0, "ALAudioEngine");
    qmlRegisterUncreatableType<BaseKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "BaseKeyframe", "");
    qmlRegisterUncreatableType<BaseMotionTrack>("com.github.mmdai.VPVM", 1, 0, "BaseMotionTrack", "");
    qmlRegisterUncreatableType<BoneKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "BoneKeyframe", "");
    qmlRegisterUncreatableType<BoneMotionTrack>("com.github.mmdai.VPVM", 1, 0, "BoneMotionTrack", "");
    qmlRegisterUncreatableType<BoneRefObject>("com.github.mmdai.VPVM", 1, 0, "Bone", "");
    qmlRegisterUncreatableType<CameraKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "CameraKeyframe", "");
    qmlRegisterUncreatableType<CameraMotionTrack>("com.github.mmdai.VPVM", 1, 0, "CameraMotionTrack", "");
    qmlRegisterUncreatableType<CameraRefObject>("com.github.mmdai.VPVM", 1, 0, "Camera", "");
    qmlRegisterUncreatableType<GraphicsDevice>("com.github.mmdai.VPVM", 1, 0, "GraphicsDevice", "");
    qmlRegisterUncreatableType<Grid>("com.github.mmdai.VPVM", 1, 0, "Grid", "");
    qmlRegisterUncreatableType<LabelRefObject>("com.github.mmdai.VPVM", 1, 0, "Label", "");
    qmlRegisterUncreatableType<LightKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "LightKeyframe", "");
    qmlRegisterUncreatableType<LightMotionTrack>("com.github.mmdai.VPVM", 1, 0, "LightMotionTrack", "");
    qmlRegisterUncreatableType<LightRefObject>("com.github.mmdai.VPVM", 1, 0, "Light", "");
    qmlRegisterUncreatableType<ModelProxy>("com.github.mmdai.VPVM", 1, 0, "Model", "");
    qmlRegisterUncreatableType<MorphKeyframeRefObject>("com.github.mmdai.VPVM", 1, 0, "MorphKeyframe", "");
    qmlRegisterUncreatableType<MorphMotionTrack>("com.github.mmdai.VPVM", 1, 0, "MorphMotionTrack", "");
    qmlRegisterUncreatableType<MorphRefObject>("com.github.mmdai.VPVM", 1, 0, "Morph", "");
    qmlRegisterUncreatableType<MotionProxy>("com.github.mmdai.VPVM", 1, 0, "Motion", "");
    qmlRegisterUncreatableType<Preference>("com.github.mmdai.VPVM", 1, 0, "Preference", "");
    qmlRegisterUncreatableType<WorldProxy>("com.github.mmdai.VPVM", 1, 0, "World", "");
    qmlRegisterSingletonType<UIAuxHelper>("com.github.mmdai.VPVM", 1, 0, "UIAuxHelper", createUIAuxHelper);
    qmlRegisterType<RenderTarget>("com.github.mmdai.VPVM", 1, 0, "RenderTarget");
    qmlRegisterType<ProjectProxy>("com.github.mmdai.VPVM", 1, 0, "Project");
}

}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setApplicationDisplayName("VPVM");
    application.setApplicationName("VPVM");
    application.setApplicationVersion("0.33.2");
    application.setOrganizationName("MMDAI Project");
    application.setOrganizationDomain("mmdai.github.com");
    QTranslator translator;
    translator.load(QLocale::system(), "VPVM", ".", Util::resourcePath("translations"), ".qm");
    application.installTranslator(&translator);

    QCommandLineParser parser;
    ApplicationBootstrapOption applicationBootstrapOption(&parser);
    parser.process(application);

    Preference applicationPreference;
    const QString &loggingDirectory = applicationPreference.initializeLoggingDirectory();
    int verboseLogLevel = applicationPreference.verboseLogLevel();
    BaseApplicationContext::initializeOnce(argv[0],  qPrintable(loggingDirectory), verboseLogLevel);
    if (applicationPreference.isFontFamilyToGUIShared()) {
        application.setFont(applicationPreference.fontFamily());
    }
    prepareRegal();
    registerQmlTypes();

    QQuickWindow::setDefaultAlphaBuffer(applicationPreference.isTransparentWindowEnabled());
    QQmlApplicationEngine engine;
    QQmlContext *rootContext = engine.rootContext();
    rootContext->setContextProperty("applicationPreference", &applicationPreference);
    rootContext->setContextProperty("applicationBootstrapOption", &applicationBootstrapOption);
    g_loggerThread.setDirectory(loggingDirectory);
    QThreadPool::globalInstance()->start(&g_loggerThread);
#ifdef QT_NO_DEBUG
    engine.load(QUrl("qrc:///qml/VPVM/main.qml"));
#else
    engine.load(Util::resourcePath("qml/main.qml"));
#endif

    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
    Q_ASSERT(window);
    QSurfaceFormat format = window->format();
    format.setSamples(applicationPreference.samples());
#if 0
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
#endif
    window->setFormat(format);
#ifdef Q_OS_MACX
    window->setFlags(window->flags() | Qt::WindowFullscreenButtonHint);
#endif
    window->show();

    int result = application.exec();
    g_loggerThread.stop();
    BaseApplicationContext::terminate();

    return result;
}

#include "main.moc"

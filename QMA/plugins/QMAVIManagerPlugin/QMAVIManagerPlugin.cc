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

#include "QMAVIManagerPlugin.h"

#include <QtGui/QDesktopServices>
#include <QtCore>

#include <MMDME/MMDME.h>

const QString QMAVIManagerPlugin::kValueSet = "VALUE_SET";
const QString QMAVIManagerPlugin::kValueUnset = "VALUE_UNSET";
const QString QMAVIManagerPlugin::kValueEvaluate = "VALUE_EVAL";
const QString QMAVIManagerPlugin::kTimerStart = "TIMER_START";
const QString QMAVIManagerPlugin::kTimerStop = "TIMER_STOP";

const QString QMAVIManagerPlugin::kValueSetEvent = "VALUE_EVENT_SET";
const QString QMAVIManagerPlugin::kValueUnsetEvent = "VALUE_EVENT_UNSET";
const QString QMAVIManagerPlugin::kValueEvaluateEvent = "VALUE_EVENT_EVAL";
const QString QMAVIManagerPlugin::kTimerStartEvent = "TIMER_EVENT_START";
const QString QMAVIManagerPlugin::kTimerStopEvent = "TIMER_EVENT_STOP";

const QString QMAVIManagerPlugin::kKeyPost = "KEY_POST";
const QString QMAVIManagerPlugin::kExecute = "EXECUTE";

QMAVIManagerPlugin::QMAVIManagerPlugin(QObject *parent)
    : QMAPlugin(parent),
      m_thread(this)
{
}

QMAVIManagerPlugin::~QMAVIManagerPlugin()
{
}

void QMAVIManagerPlugin::load(MMDAI::SceneController *controller, const QString &baseName)
{
    Q_UNUSED(controller);
    if (!m_thread.isStarted()) {
        QFile config(QString("MMDAIUserData:/%1.fst").arg(baseName));
        if (config.exists() && config.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&config);
            stream.setCodec("Shift-JIS");
            m_thread.load(stream);
            config.close();
            m_thread.start();
        }
    }
}

void QMAVIManagerPlugin::unload()
{
    m_thread.stop();
}

void QMAVIManagerPlugin::receiveCommand(const QString &command, const QList<QVariant> &arguments)
{
    int argc = arguments.count();
    if (command == kValueSet && argc >= 2) {
        const QString key = arguments[0].toString();
        const QString value = arguments[0].toString();
        QString value2 = "";
        if (argc >= 3)
            value2 = arguments[2].toString();
        setValue(key, value, value2);
    }
    else if (command == kValueUnset && argc >= 1) {
        const QString key = arguments[0].toString();
        deleteValue(key);
    }
    else if (command == kValueEvaluate && argc >= 3) {
        const QString key = arguments[0].toString();
        const QString op = arguments[1].toString();
        const QString value = arguments[2].toString();
        evaluate(key, op, value);
    }
    else if (command == kTimerStart && argc >= 2) {
        const QString key = arguments[0].toString();
        const QString value = arguments[1].toString();
        startTimer0(key, value);
    }
    else if (command == kTimerStop && argc >= 1) {
        const QString key = arguments[0].toString();
        stopTimer0(key);
    }
    else if (command == kExecute && argc >= 1) {
        const QString argument = arguments[0].toString();
        const QUrl url(argument);
        if (url.isValid()) {
            QDesktopServices::openUrl(url);
        }
        else {
            const QProcess process;
            const QString program = argument;
            process.execute(program);
        }
    }
}

void QMAVIManagerPlugin::receiveEvent(const QString &type, const QList<QVariant> &arguments)
{
    if (m_thread.isStarted() && !QMAPlugin::isRenderEvent(type)) {
        QStringList strings;
        foreach (QVariant arg, arguments) {
            strings << arg.toString();
        }
        QByteArray typeBytes = type.toUtf8();
        QByteArray stringsBytes = strings.join("|").toUtf8();
        m_thread.enqueueBuffer(typeBytes.constData(), stringsBytes.constData());
    }
}

void QMAVIManagerPlugin::sendCommand(const char *command, char *arguments)
{
    static QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QList<QVariant> args;
    QString argv = codec->toUnicode(arguments);
    foreach (QString value, argv.split("|")) {
        args << value;
    }
    emit commandPost(command, args);
    free(arguments);
}

void QMAVIManagerPlugin::sendEvent(const char *type, char *arguments)
{
    Q_UNUSED(type);
    Q_UNUSED(arguments);
    /* do nothing */
}

void QMAVIManagerPlugin::setValue(const QString &key, const QString &value, const QString &value2)
{
    if (key.isEmpty()) {
        MMDAILogInfoString("specified key is empty");
        return;
    }
    if (value2.isNull()) {
        m_values[key] = value.toFloat();
    }
    else {
        float min = value.toFloat();
        float max = value2.toFloat();
        if (max < min) {
            float swap = max;
            max = min;
            min = swap;
        }
        const float random = min + (max - min) * qrand() * (1.0f / RAND_MAX);
        m_values[key] = random;
    }
    QList<QVariant> arguments;
    arguments << key;
    emit eventPost(kValueSetEvent, arguments);
}

void QMAVIManagerPlugin::deleteValue(const QString &key)
{
    if (m_values.contains(key)) {
        m_values.remove(key);
        QList<QVariant> arguments;
        arguments << key;
        emit eventPost(kValueUnsetEvent, arguments);
    }
}

void QMAVIManagerPlugin::evaluate(const QString &key, const QString &op, const QString &value)
{
    if (!m_values.contains(key)) {
        MMDAILogInfo("Evaluating %s is not found", key.toUtf8().constData());
        return;
    }
    const float v1 = value.toFloat();
    const float v2 = m_values[key];
    bool ret = false;
    if (op == "EQ") {
        ret = v1 == v2;
    }
    else if (op == "NE") {
        ret = v1 != v2;
    }
    else if (op == "LT") {
        ret = v1 > v2;
    }
    else if (op == "LE") {
        ret = v1 >= v2;
    }
    else if (op == "GT") {
        ret = v1 < v2;
    }
    else if (op == "GE") {
        ret = v1 <= v2;
    }
    else {
        MMDAILogInfo("Operation %s is invalid", op.toUtf8().constData());
    }
    QList<QVariant> arguments;
    arguments << key << op << value;
    arguments << (ret ? "TRUE" : "FALSE");
    emit eventPost(kValueEvaluateEvent, arguments);
}

void QMAVIManagerPlugin::startTimer0(const QString &key, const QString &value)
{
    if (key.isEmpty()) {
        MMDAILogInfoString("specified key is empty");
        return;
    }
    const float seconds = value.toFloat();
    if (m_timers.contains(key)) {
        QBasicTimer *timer = m_timers.value(key);
        timer->stop();
        delete timer;
    }
    if (seconds > 0) {
        const int msec = seconds * 1000;
        QBasicTimer *timer = new QBasicTimer();
        m_timers.insert(key, timer);
        timer->start(msec, this);
        QList<QVariant> arguments;
        arguments << key;
        emit eventPost(kTimerStartEvent, arguments);
    }
    else {
        MMDAILogInfo("Invalid second: %s", value.toUtf8().constData());
    }
}

void QMAVIManagerPlugin::stopTimer0(const QString &key)
{
    if (m_timers.contains(key)) {
        QBasicTimer *timer = m_timers.value(key);
        m_timers.remove(key);
        timer->stop();
        delete timer;
        QList<QVariant> arguments;
        arguments << key;
        emit eventPost(kTimerStopEvent, arguments);
    }
}

void QMAVIManagerPlugin::timerEvent(QTimerEvent *event)
{
    QMapIterator<QString, QBasicTimer *> iterator(m_timers);
    QString key;
    QBasicTimer *timer = NULL;
    const int id = event->timerId();
    while (iterator.hasNext()) {
        iterator.next();
        timer = iterator.value();
        if (timer->timerId() == id) {
            key = iterator.key();
            break;
        }
    }
    if (!key.isNull()) {
        timer->stop();
        delete timer;
        m_timers.remove(key);
        QList<QVariant> arguments;
        arguments << key;
        emit eventPost(kTimerStopEvent, arguments);
    }
    else {
        MMDAILogWarn("%s seems deleted", key.toUtf8().constData());
    }
}

Q_EXPORT_PLUGIN2(qma_vimanager_plugin, QMAVIManagerPlugin)

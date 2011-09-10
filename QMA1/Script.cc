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

#include "Script.h"

#include "SceneWidget.h"
#include "TiledStage.h"
#include "util.h"

#include <QtCore/QtCore>
#include <vpvl/vpvl.h>

namespace
{
/* command names */
const QString &kModelAddCommand = "MODEL_ADD";
const QString &kModelChangeCommand = "MODEL_CHANGE";
const QString &kModelDeleteCommand = "MODEL_DELETE";
const QString &kMotionAddCommand = "MOTION_ADD";
const QString &kMotionChangeCommand = "MOTION_CHANGE";
const QString &kMotionDeleteCommand = "MOTION_DELETE";
const QString &kMoveStartCommand = "MOVE_START";
const QString &kMoveStopCommand = "MOVE_STOP";
const QString &kTurnStartCommand = "TURN_START";
const QString &kTurnStopCommand = "TURN_STOP";
const QString &kRotateStartCommand = "ROTATE_START";
const QString &kRotateStopCommand = "ROTATE_STOP";
const QString &kStageCommand = "STAGE";
const QString &kFloorCommand = "FLOOR";
const QString &kBackgroundCommand = "BACKGROUND";
const QString &kLightColorCommand = "LIGHTCOLOR";
const QString &kLightDirectionCommand = "LIGHTDIRECTION";
const QString &kLipSyncStartCommand = "LIPSYNC_START";
const QString &kLipSyncStopCommand = "LIPSYNC_STOP";
const QString &kCameraCommand = "CAMERA";
const QString kValueSet = "VALUE_SET";
const QString kValueUnset = "VALUE_UNSET";
const QString kValueEvaluate = "VALUE_EVAL";
const QString kTimerStart = "TIMER_START";
const QString kTimerStop = "TIMER_STOP";
/* event */
const QString &kModelAddEvent = "MODEL_EVENT_ADD";
const QString &kModelChangeEvent = "MODEL_EVENT_CHANGE";
const QString &kModelDeleteEvent = "MODEL_EVENT_DELETE";
const QString &kMotionAddEvent = "MOTION_EVENT_ADD";
const QString &kMotionDeleteEvent = "MOTION_EVENT_DELETE";
const QString &kMotionChangeEvent = "MOTION_EVENT_CHANGE";
const QString &kMotionLoopEvent = "MOTION_EVENT_LOOP";
const QString &kMoveStartEvent = "MOVE_EVENT_START";
const QString &kMoveStopEvent = "MOVE_EVENT_STOP";
const QString &kTurnStartEvent = "TURN_EVENT_START";
const QString &kTurnStopEvent = "TURN_EVENT_STOP";
const QString &kRotateStartEvent = "ROTATE_EVENT_START";
const QString &kRotateStopEvent = "ROTATE_EVENT_STOP";
const QString &kStageEvent = "MMDAI_EVENT_STAGE";
const QString &kFloorEvent = "MMDAI_EVENT_FLOOR";
const QString &kBackgroundEvent = "MMDAI_EVENT_BACKGROUND";
const QString &kLightColorEvent = "MMDAI_EVENT_LIGHTCOLOR";
const QString &kLightDirectionEvent = "MMDAI_EVENT_LIGHTDIRECTION";
const QString &kLipSyncStartEvent = "LIPSYNC_EVENT_START";
const QString &kLipSyncStopEvent = "LIPSYNC_EVENT_STOP";
const QString kValueSetEvent = "VALUE_EVENT_SET";
const QString kValueUnsetEvent = "VALUE_EVENT_UNSET";
const QString kValueEvaluateEvent = "VALUE_EVENT_EVAL";
const QString kTimerStartEvent = "TIMER_EVENT_START";
const QString kTimerStopEvent = "TIMER_EVENT_STOP";
const QString &kKeyEvent = "KEY";

}

struct ScriptArc {
    ScriptArgument input;
    ScriptArgument output;
    State *nextState;
    ScriptArc(const ScriptArgument &i,
              const ScriptArgument &o,
              State *state)
        : input(i), output(o), nextState(state) {
    }
};

struct State {
    uint32_t index;
    QList<ScriptArc *> arcs;
    State *next;
    State(uint32_t i, State *state)
        : index(i), next(state) {
    }
    ~State() {
        qDeleteAll(arcs);
    }
};

#if 0
static void DumpScriptState(State *state, int &depth)
{
    if (!state)
        return;
    depth++;
    qDebug("state dump start (depth=%d)", depth);
    uint32_t index = state->index;
    foreach (ScriptArc *arc, state->arcs) {
        qDebug() << index << arc->input.type << arc->input.arguments
                 << arc->output.type << arc->output.arguments;
        DumpScriptState(state->next, depth);
    }
    depth--;
    qDebug("state dump end (depth=%d)", depth);
}

static void DumpScriptStates(QLinkedList<State *> states)
{
    int depth = 0;
    foreach (State *state, states)
        DumpScriptState(state, depth);
}
#else
#define DumpScriptStates (void)
#endif

const QString Script::kEPS = "<eps>";

Script::Script(SceneWidget *parent)
    : QObject(parent),
      m_parent(parent),
      m_currentState(0),
      m_stage(0)
{
    connect(this, SIGNAL(eventDidPost(QString,Arguments)), this, SLOT(queueEvent(QString,Arguments)));
    connect(m_parent, SIGNAL(motionDidFinished(QMultiMap<vpvl::PMDModel*,vpvl::VMDMotion*>)),
            this, SLOT(handleFinishedMotion(QMultiMap<vpvl::PMDModel*,vpvl::VMDMotion*>)));
}

Script::~Script()
{
    stop();
    qDeleteAll(m_states);
    qDeleteAll(m_timers);
    m_parent = 0;
    m_currentState = 0;
    m_parent = 0;
}

bool Script::load(QTextStream &stream)
{
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    QString sep = codec->toUnicode("\\");

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (!line.isEmpty() && line[0] != '#') {
            QStringList tokens = line.replace(sep, "/").split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (tokens.count() == 4) {
                uint32_t from = tokens.at(0).toUInt();
                uint32_t to = tokens.at(1).toUInt();
                QStringList i = tokens.at(2).split(QRegExp("\\|"), QString::SkipEmptyParts);
                QStringList o = tokens.at(3).split(QRegExp("\\|"), QString::SkipEmptyParts);
                if (i.count() > 0 && o.count() > 0) {
                    ScriptArgument input(i.takeFirst(), i), output(o.takeFirst(), o);
                    addScriptArc(from, to, input, output);
                }
                else {
                    qDebug("Empty input arguments or empty output arguments found: %s", qPrintable(line));
                }
            }
            else {
                qWarning("%s", qPrintable(tr("Invalid script line: %s").arg(line)));
            }
        }
    }
    m_currentState = newScriptState(0);
    DumpScriptStates(m_states);

    return false;
}

void Script::start()
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(execute()));
    executeEplisons();
    m_timer.start(20);
}

void Script::stop()
{
    disconnect(this, SLOT(execute()));
    m_timer.stop();
}

void Script::timerEvent(QTimerEvent *event)
{
    QMapIterator<QString, QBasicTimer *> iterator(m_timers);
    QString key;
    QBasicTimer *timer = 0;
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
        Arguments a; a << key;
        emit eventDidPost(kTimerStopEvent, a);
    }
    else {
        qWarning("%s", qPrintable(tr("[%1] %2 seems deleted").arg(key)));
    }
}

void Script::execute()
{
    while (m_queue.size() > 0) {
        ScriptArgument output, input = m_queue.dequeue();
        setTransition(input, output);
        if (output.type != Script::kEPS)
            handleCommand(output);
        executeEplisons();
    }
}

void Script::queueEvent(const QString &type, const Arguments &arguments)
{
    QStringList strings;
    foreach (QVariant arg, arguments) {
        strings << arg.toString();
    }
    qDebug() << type << arguments;
    m_queue.enqueue(ScriptArgument(type, strings));
}

void Script::handleFinishedMotion(const QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> &motions)
{
    Arguments a;
    QMapIterator<vpvl::PMDModel *, vpvl::VMDMotion *> iterator(motions);
    while (iterator.hasNext()) {
        iterator.next();
        vpvl::VMDMotion *motion = iterator.value();
        const QString &name = m_motions.key(motion);
        if (!name.isEmpty()) {
            vpvl::PMDModel *model = iterator.key();
            m_parent->deleteMotion(motion, model);
            m_motions.remove(name);
            a.clear(); a << name;
            emit eventDidPost(kMotionDeleteEvent, a);
        }
    }
}

void Script::addScriptArc(int from,
                          int to,
                          const ScriptArgument &input,
                          const ScriptArgument &output)
{
    State *state1 = newScriptState(from);
    State *state2 = newScriptState(to);
    ScriptArc *arc = new ScriptArc(input, output, state2);
    state1->arcs.append(arc);
}

const QString Script::canonicalizePath(const QString &path)
{
    const QString filename = QString(path).replace("\\", "/");
    QString result = m_dir.absoluteFilePath(filename);
    if (!QFile::exists(result))
        result = QDir(":MMDAIUserData/").absoluteFilePath(filename);
    return result;
}

void Script::executeEplisons()
{
    ScriptArgument output, eps(Script::kEPS, QStringList());
    while (setTransition(eps, output)) {
        if (output.type != Script::kEPS)
            handleCommand(output);
    }
}

void Script::handleCommand(const ScriptArgument &output)
{
    const QString &type = output.type;
    const QStringList &argv = output.arguments;
    const QString kInvalidArgumentFixed = tr("[%1] Invalid argument count (expected %2, actual is %3)");
    const QString kInvalidArgumentRanged = tr("[%1] Invalid argument count (expected between %2 and %3, actual is %4)");
    const QString kInvalidArgumentVariant = tr("[%1] Invalid argument count (expected %2 or between %3 and %4, actual is %5)");
    const QString kModelNotFound = tr("[%1] Model %2 is not found");
    int argc = output.arguments.count();
    if (type == kModelAddCommand) {
        if (argc < 2 || argc > 6) {
            qWarning("%s", qPrintable(kInvalidArgumentRanged.arg(type).arg(2).arg(6).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        const QString &path = canonicalizePath(argv[1]);
        vpvl::PMDModel *model = m_parent->addModel(path);
        if (model) {
            if (argc >= 3) {
                btVector3 position;
                parsePosition(argv[2], position);
                model->setPositionOffset(position);
                vpvl::Bone *rootBone = model->mutableRootBone();
                rootBone->setOffset(position);
                rootBone->updateTransform();
                if (argc >= 4) {
                    btQuaternion rotation;
                    parseRotation(argv[3], rotation);
                    model->setRotationOffset(rotation);
                }
            }
            else {
                model->setPositionOffset(model->rootBone().offset());
            }
            if (argc >= 5) {
                vpvl::PMDModel *parentModel = m_models.value(argv[4]);
                if (argc >= 6) {
                    const QByteArray &name = internal::fromQString(argv[5]);
                    vpvl::Bone *bone = model->findBone(reinterpret_cast<const uint8_t *>(name.constData()));
                    model->setBaseBone(bone);
                }
                else {
                    vpvl::Bone *bone = vpvl::Bone::centerBone(parentModel->mutableBones());
                    model->setBaseBone(bone);
                }
            }
            model->updateImmediate();
            m_models.insert(modelName, model);
            Arguments a; a << modelName;
            emit eventDidPost(kModelAddEvent, a);
        }
        else {
            qWarning("%s", qPrintable(tr("[%1] Failed loading the model %2: %3").arg(type).arg(modelName).arg(path)));
        }
    }
    else if (type == kModelChangeCommand) {
        if (argc != 2) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(2).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        if (m_models.contains(modelName)) {
            vpvl::PMDModel *model = m_models.value(modelName);
            const QString &path = canonicalizePath(argv[1]);
            m_parent->deleteModel(model);
            model = m_parent->addModel(path);
            if (model) {
                m_models.remove(modelName);
                m_models.insert(modelName, model);
                Arguments a; a << modelName;
                emit eventDidPost(kModelChangeEvent, a);
            }
            else
                qWarning("%s", qPrintable(tr("[%1] Failed loading the model %2: %3").arg(type).arg(modelName).arg(path)));
        }
        else {
            qWarning("%s", kModelNotFound.arg(type).arg(modelName).toUtf8().constData());
        }
    }
    else if (type == kModelDeleteCommand) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        if (m_models.contains(modelName)) {
            m_parent->deleteModel(m_models.value(modelName));
            Arguments a; a << modelName;
            emit eventDidPost(kModelDeleteEvent, a);
        }
        else {
            qWarning("%s", qPrintable(kModelNotFound.arg(type).arg(modelName)));
        }
    }
    else if (type == kMotionAddCommand) {
        if (argc < 3 || argc > 8) {
            qWarning("%s", qPrintable(kInvalidArgumentRanged.arg(type).arg(3).arg(8).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        const QString &motionName = argv[1];
        if (m_models.contains(modelName)) {
            const QString &path = canonicalizePath(argv[2]);
            vpvl::PMDModel *model = m_models.value(modelName);
            vpvl::VMDMotion *motion = m_parent->insertMotionToModel(path, model);
            if (motion) {
                bool value = false;
                motion->setEnableRelocation(true);
                motion->setEnableSmooth(true);
                if (argc >= 4) {
                    parseEnable(argv[3], "FULL", "PART", value);
                    motion->setFull(value);
                }
                if (argc >= 5) {
                    parseEnable(argv[4], "LOOP", "ONCE", value);
                    motion->setLoop(value);
                }
                if (argc >= 6) {
                    parseEnable(argv[5], "ON", "OFF", value);
                    motion->setEnableSmooth(value);
                }
                if (argc >= 7) {
                    parseEnable(argv[6], "ON", "OFF", value);
                    motion->setEnableRelocation(value);
                }
                if (argc >= 8) {
                    motion->setPriority(argv[7].toFloat());
                }
                m_motions.insert(motionName, motion);
                Arguments a; a << modelName << motionName;
                emit eventDidPost(kMotionAddEvent, a);
            }
            else {
                qWarning("%s", qPrintable(tr("[%1] Failed loading the motion %2 to %3: %4")
                                          .arg(type).arg(motionName).arg(modelName).arg(path)));
            }
        }
        else {
            qWarning("%s", qPrintable(kModelNotFound.arg(type).arg(modelName)));
        }
    }
    else if (type == kMotionChangeCommand) {
        if (argc != 3) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(3).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        const QString &motionName = argv[1];
        const QString &path = canonicalizePath(argv[2]);
        if (m_models.contains(modelName) && m_motions.contains(motionName)) {
            vpvl::PMDModel *model = m_models.value(modelName);
            vpvl::VMDMotion *motion = m_motions.value(motionName);
            bool full = motion->isFull();
            bool loop = motion->isLoop();
            bool enableSmooth = motion->enableSmooth();
            bool enableRelocation = motion->enableRelocation();
            m_parent->deleteMotion(motion, model);
            motion = m_parent->insertMotionToModel(path, model);
            if (motion) {
                motion->setFull(full);
                motion->setLoop(loop);
                motion->setEnableSmooth(enableSmooth);
                motion->setEnableRelocation(enableRelocation);
                m_motions.remove(motionName);
                m_motions.insert(motionName, motion);
                Arguments a;  a << modelName << motionName;
                emit eventDidPost(kMotionChangeEvent, a);
            }
            else {
                qWarning("%s", qPrintable(tr("[%1] Failed loading the motion %2 to %3: %4")
                                          .arg(type).arg(motionName).arg(modelName).arg(path)));
            }
        }
        else
            qWarning("%s", qPrintable(kModelNotFound.arg(type).arg(modelName)));
    }
    else if (type == kMotionDeleteCommand) {
        if (argc != 2) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(2).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        const QString &motionName = argv[1];
        if (m_models.contains(modelName) && m_motions.contains(motionName)) {
            vpvl::PMDModel *model = m_models.value(modelName);
            vpvl::VMDMotion *motion = m_motions.value(motionName);
            m_parent->deleteMotion(motion, model);
        }
        else {
            qWarning("%s", qPrintable(kModelNotFound.arg(type).arg(modelName)));
        }
    }
    else if (type == kStageCommand) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        QString filename = argv[0];
        QStringList pair = filename.split(',');
        if (pair.count() == 2) {
            TiledStage *stage = m_parent->tiledStage();
            stage->loadFloor(canonicalizePath(pair[0]));
            stage->loadBackground(canonicalizePath(pair[1]));
        }
        else {
            m_parent->deleteModel(m_stage);
            const QString &path = canonicalizePath(argv[0]);
            vpvl::PMDModel *model = m_parent->addModel(path);
            if (model) {
                m_stage = model;
                emit eventDidPost(kStageEvent, Arguments());
            }
            else {
                qWarning("%s", qPrintable(tr("[%1] Failed loading the stage: %2").arg(type).arg(path)));
            }
        }
    }
    else if (type == kLightColorCommand) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        btVector4 color;
        parseColor(argv[0], color);
        m_parent->setLightColor(color);
        Arguments a; a << color.x() << color.y() << color.z() << color.w();
        emit eventDidPost(kLightColorEvent, a);
    }
    else if (type == kLightDirectionCommand) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        btVector3 position;
        parsePosition(argv[0], position);
        m_parent->setLightPosition(position);
        Arguments a; a << position.x() << position.y() << position.z();
        emit eventDidPost(kLightDirectionEvent, a);
    }
    else if (type == kLipSyncStartCommand) {
    }
    else if (type == kLipSyncStopCommand) {
    }
    else if (type == kCameraCommand) {
        if (argc == 1) {
            const QString &path = canonicalizePath(argv[0]);
            if (!m_parent->setCamera(path))
                qWarning("%s", qPrintable(tr("[%1] Failed loading camera motion: %2").arg(type).arg(path)));
        }
        else if (argc == 3 || argc == 4) {
            btVector3 position, angle;
            parsePosition(argv[0], position);
            parsePosition(argv[1], angle);
            float distance = argv.at(2).toFloat();
            float fovy = argv.at(3).toFloat();
            m_parent->setCameraPerspective(&position, &angle, &fovy, &distance);
        }
        else {
            qWarning("%s", qPrintable(kInvalidArgumentVariant.arg(type).arg(1).arg(3).arg(4).arg(argc)));
        }
    }
    else if (type == kValueSet) {
        if (argc != 2 && argc != 3) {
            qWarning("%s", qPrintable(kInvalidArgumentRanged.arg(type).arg(2).arg(3).arg(argc)));
            return;
        }
        const QString &key = argv[0];
        const QString &value = argv[1];
        if (key.isEmpty()) {
            qWarning("%s", qPrintable(tr("[%1] Specified key is empty").arg(type)));
            return;
        }
        if (argc == 2) {
            m_values[key] = value.toFloat();
        }
        else {
            float min = value.toFloat();
            float max = argc == 3 ? argv.at(2).toFloat() : 0.0f;
            if (max < min)
                qSwap(max, min);
            const float random = min + (max - min) * qrand() * (1.0f / RAND_MAX);
            m_values[key] = random;
        }
        Arguments a; a << key;
        emit eventDidPost(kValueSetEvent, a);
    }
    else if (type == kValueUnset) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        const QString &key = argv[0];
        if (m_values.contains(key)) {
            m_values.remove(key);
            Arguments a; a << key;
            emit eventDidPost(kValueUnsetEvent, a);
        }
    }
    else if (type == kValueEvaluate) {
        if (argc != 3) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(3).arg(argc)));
            return;
        }
        const QString &key = argv[0];
        const QString &value = argv[1];
        const QString &op = argv[2];
        if (!m_values.contains(key)) {
            qWarning("%s", qPrintable(tr("[%1] Evaluating %1 is not found").arg(type).arg(key)));
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
            qWarning("%s", qPrintable(tr("[%1] Operation %s is invalid").arg(type).arg(op)));
        }
        Arguments a; a << key << op << value << (ret ? "TRUE" : "FALSE");
        emit eventDidPost(kValueEvaluateEvent, a);
    }
    else if (type == kTimerStart) {
        if (argc != 2) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(2).arg(argc)));
            return;
        }
        const QString &key = argv[0];
        const QString &value = argv[1];
        if (key.isEmpty()) {
            qWarning("%s", qPrintable(tr("[%1] Specified key is empty").arg(type)));
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
            Arguments a; a << key;
            emit eventDidPost(kTimerStartEvent, a);
        }
        else {
            qWarning("%s", qPrintable(tr("[%1] Invalid second: %2").arg(type).arg(value)));
        }
    }
    else if (type == kTimerStop) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        const QString &key = argv[0];
        if (m_timers.contains(key)) {
            QBasicTimer *timer = m_timers.value(key);
            m_timers.remove(key);
            timer->stop();
            delete timer;
            Arguments a; a << key;
            emit eventDidPost(kTimerStopEvent, a);
        }
    }
    qDebug() << type << argv;
}

State *Script::newScriptState(quint32 index)
{
    State *head, *res;
    if (m_states.count() == 0) {
        res = new State(index, 0);
        m_states.append(res);
    }
    head = m_states.first();
    if (head->index == index) {
        res = head;
    }
    else if (head->index > index) {
        res = new State(index, head);
        m_states.prepend(res);
    }
    else {
        for (State *state = head; state; state = state->next) {
            State *&p = state->next;
            if (!p) {
                res = p = new State(index, 0);
                break;
            }
            else if (p->index == index) {
                res = p;
                break;
            }
            else if (index < p->index) {
                State *q = p;
                res = p = new State(index, q);
                break;
            }
        }
        if (!res)
            qWarning("unknown state: %d", index);
    }
    return res;
}

bool Script::setTransition(const ScriptArgument &input,
                           ScriptArgument &output)
{
    bool jumped = false;
    output.type = kEPS;
    output.arguments.clear();

    if (!m_currentState || m_currentState->arcs.isEmpty())
        return jumped;

    foreach (const ScriptArc *arc, m_currentState->arcs) {
        const QStringList &args = input.arguments;
        const QStringList &arcargs = arc->input.arguments;
        const int argc = args.count();
        const int arcargc = arcargs.count();
        if (input.type == "RECOG_EVENT_STOP") {
            for (int i = 0; i < argc; i++) {
                jumped = false;
                for (int j = 0; j < arcargc; j++) {
                    if (args[i] == arcargs[i]) {
                        jumped = true;
                        break;
                    }
                }
                if (!jumped)
                    break;
            }
        }
        else if (input.type == arc->input.type && argc == arcargc) {
            jumped = true;
            for (int i = 0; i < argc; i++) {
                if (args[i] != arcargs[i]) {
                    jumped = false;
                    break;
                }
            }
        }
        if (jumped) {
            output = arc->output;
            m_currentState = arc->nextState;
            break;
        }
    }

    return jumped;
}

bool Script::parseEnable(const QString &value, const QString &enable, const QString &disable, bool &output) const
{
    if (enable == value) {
        output = true;
        return true;
    }
    else if (disable == value) {
        output = false;
        return true;
    }
    else {
        output = false;
        qWarning("%s", qPrintable(tr("Unexpected value %s to boolean (%s or %s)")
                                  .arg(value).arg(enable).arg(disable)));
    }
    return false;
}

bool Script::parsePosition(const QString &value, btVector3 &v) const
{
    QStringList xyz = value.split(',');
    if (xyz.count() == 3) {
        v.setZero();
        v.setX(xyz.at(0).toFloat());
        v.setY(xyz.at(1).toFloat());
        v.setZ(xyz.at(2).toFloat());
        return true;
    }
    else {
        v.setZero();
        qWarning("%s", qPrintable(tr("Unexpected value %s to position").arg(value)));
    }
    return false;
}

bool Script::parseColor(const QString &value, btVector4 &v) const
{
    QStringList xyz = value.split(',');
    if (xyz.count() == 4) {
        v.setX(xyz.at(0).toFloat());
        v.setY(xyz.at(1).toFloat());
        v.setZ(xyz.at(2).toFloat());
        v.setW(xyz.at(3).toFloat());
        return true;
    }
    else {
        v.setZero();
        v.setW(1.0f);
        qWarning("%s", qPrintable(tr("Unexpected value %s to color").arg(value)));
    }
    return false;
}

bool Script::parseRotation(const QString &value, btQuaternion &v) const
{
    QStringList xyz = value.split(',');
    if (xyz.count() == 3) {
        float z = vpvl::radian(xyz.at(2).toFloat());
        float y = vpvl::radian(xyz.at(1).toFloat());
        float x = vpvl::radian(xyz.at(0).toFloat());
        v.setEulerZYX(z, y, x);
        return true;
    }
    else {
        v.setValue(0.0f, 0.0f, 0.0f, 1.0f);
        qWarning("%s", qPrintable(tr("Unexpected value %s to rotation").arg(value)));
    }
    return false;
}

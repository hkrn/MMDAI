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
#include "SceneLoader.h"

#include "ExtendedSceneWidget.h"
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
const QString kValueSetCommand = "VALUE_SET";
const QString kValueUnsetCommand = "VALUE_UNSET";
const QString kValueEvaluateCommand = "VALUE_EVAL";
const QString kTimerStartCommand = "TIMER_START";
const QString kTimerStopCommand = "TIMER_STOP";
const QString kExecuteCommand = "EXECUTE";
const QString kKeyPostCommand = "KEY_POST";
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
const QString Script::kLipSyncName = "LipSync";

Script::Script(ExtendedSceneWidget *parent)
    : QObject(parent),
      m_parent(parent),
      m_currentState(0),
      m_stage(0)
{
    SceneLoader *loader = parent->sceneLoader();
    loader->createProject();
    connect(this, SIGNAL(eventDidPost(QString,QList<QVariant>)), this, SLOT(handleEvent(QString,QList<QVariant>)));
    connect(loader, SIGNAL(modelWillDelete(vpvl::PMDModel*,QUuid)), this, SLOT(handleModelDelete(vpvl::PMDModel*)));
    connect(parent, SIGNAL(motionDidFinished(QMultiMap<vpvl::PMDModel*,vpvl::VMDMotion*>)),
            this, SLOT(handleFinishedMotion(QMultiMap<vpvl::PMDModel*,vpvl::VMDMotion*>)));
    connect(&m_recog, SIGNAL(eventDidPost(QString,QList<QVariant>)), this, SLOT(handleEvent(QString,QList<QVariant>)));
    connect(&m_speech, SIGNAL(commandDidPost(QString,QList<QVariant>)), this, SLOT(handleCommand(QString,QList<QVariant>)));
    connect(&m_speech, SIGNAL(eventDidPost(QString,QList<QVariant>)), this, SLOT(handleEvent(QString,QList<QVariant>)));
}

Script::~Script()
{
    stop();
    qDeleteAll(m_states);
    qDeleteAll(m_timers);
    m_parent = 0;
    m_currentState = 0;
}

bool Script::load(QTextStream &stream)
{
    /* モデル特有のリップシンク設定ファイルがない時用のリップシンク設定ファイルを予め読み込む */
    bool ret = loadScript(stream);
    QFile file(":/lipsync/global");
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);
        loadGlobalLipSync(stream);
        file.close();
    }
    else {
        qWarning("Cannot load :/lipsync/global (and should not show this message)");
    }
    return ret;
}

void Script::loadGlobalLipSync(QTextStream &stream)
{
    m_globalLipSync.load(stream);
}

bool Script::loadScript(QTextStream &stream)
{
    /* スクリプトは仕様上 Shift_JIS として書かれているので Shift_JIS を強制させる */
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    QString sep = codec->toUnicode("\\");
    stream.setCodec("Shift-JIS");
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        /* 空白を除去し、先頭がコメント(#)の場合はその行は無視する */
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
                qWarning("%s", qPrintable(tr("Invalid script line: %1").arg(line)));
            }
        }
    }
    m_currentState = newScriptState(0);
    DumpScriptStates(m_states);

    return false;
}

void Script::loadSpeechEngine(const QDir &dir, const QString &baseName)
{
    m_speech.load(dir ,baseName);
}

void Script::loadSpeechRecognitionEngine(const QDir &dir, const QString &baseName)
{
    m_recog.load(dir, baseName);
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
    /* スクリプト特有のタイマーを探し、見つかったら停止してイベント (kTimerStopEvent) を発動させる */
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
    /* イベントキューからコマンドを処理する */
    while (m_queue.size() > 0) {
        ScriptArgument output, input = m_queue.dequeue();
        setTransition(input, output);
        if (output.type != Script::kEPS)
            handleCommand(output);
        executeEplisons();
    }
}

void Script::handleEvent(const QString &type, const QList<QVariant> &arguments)
{
    /* イベントが飛んできた場合イベントキューに積ませる (次回の execute で実行される) */
    QStringList strings;
    foreach (QVariant arg, arguments) {
        strings << arg.toString();
    }
    qDebug() << "[EVENT]  " << type << arguments;
    m_queue.enqueue(ScriptArgument(type, strings));
}

void Script::handleCommand(const QString &type, const QList<QVariant> &arguments)
{
    QStringList strings;
    foreach (QVariant arg, arguments) {
        strings << arg.toString();
    }
    handleCommand(ScriptArgument(type, strings));
}

void Script::handleModelDelete(vpvl::PMDModel *model)
{
    QString name = m_models.key(model);
    if (!name.isNull())
        m_models.remove(name);
}

void Script::handleFinishedMotion(const QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> &motions)
{
    /* ループさせる場合を除いてモーションが終了したらそのモーションは削除する */
    Arguments a;
    QMapIterator<vpvl::PMDModel *, vpvl::VMDMotion *> iterator(motions);
    SceneLoader *loader = m_parent->sceneLoader();
    while (iterator.hasNext()) {
        iterator.next();
        vpvl::VMDMotion *motion = iterator.value();
        if (m_motionParameters.contains(motion)) {
            const MotionParameter &parameter = m_motionParameters.value(motion);
            if (parameter.isLoopEnabled) {
                motion->reset();
                continue;
            }
        }
        const QString &name = m_motions.key(motion);
        if (!name.isEmpty()) {
            loader->deleteMotion(motion);
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
    /* 条件指定なしのコマンド実行を行う */
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
    SceneLoader *loader = m_parent->sceneLoader();
    int argc = output.arguments.count();
    /* モデルの追加 */
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
                vpvl::Vector3 position;
                parsePosition(argv[2], position);
                model->setPositionOffset(position);
                if (argc >= 4) {
                    vpvl::Quaternion rotation;
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
                    model->setPositionOffset(parentModel->positionOffset() + bone->position());
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
    /* モデルの変更 */
    else if (type == kModelChangeCommand) {
        if (argc != 2) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(2).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        if (m_models.contains(modelName)) {
            vpvl::PMDModel *model = m_models.value(modelName);
            const QString &path = canonicalizePath(argv[1]);
            loader->deleteModel(model);
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
    /* モデル削除 */
    else if (type == kModelDeleteCommand) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        if (m_models.contains(modelName)) {
            vpvl::PMDModel *model = m_models.value(modelName);
            /* 処理内容の関係で deleteModel() じゃないと modelWillDelete が呼ばれないのでここで signal を発行 */
            emit modelWillDelete(model);
            loader->deleteModel(model);
            m_parent->setSelectedModel(0);
            Arguments a; a << modelName;
            emit eventDidPost(kModelDeleteEvent, a);
        }
        else {
            qWarning("%s", qPrintable(kModelNotFound.arg(type).arg(modelName)));
        }
    }
    /* モーション追加 */
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
                MotionParameter parameter;
                bool value = false;
                parameter.isLoopEnabled = false;
                if (argc >= 4) {
                    parseEnable(argv[3], "PART", "FULL", value);
                    motion->setNullFrameEnable(value);
                }
                if (argc >= 5) {
                    parseEnable(argv[4], "LOOP", "ONCE", value);
                    parameter.isLoopEnabled = value;
                }
                if (argc >= 6) {
                    parseEnable(argv[5], "ON", "OFF", value);
                    //motion->setEnableSmooth(value);
                }
                if (argc >= 7) {
                    parseEnable(argv[6], "ON", "OFF", value);
                    //motion->setEnableRelocation(value);
                }
                if (argc >= 8) {
                    //motion->setPriority(argv[7].toFloat());
                }
                m_motions.insert(motionName, motion);
                m_motionParameters.insert(motion, parameter);
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
    /* モーション変更 */
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
            bool isNullFrameEnabled = motion->isNullFrameEnabled();
            bool isLoopEnabled = false;
            if (m_motionParameters.contains(motion)) {
                const MotionParameter &parameter = m_motionParameters.value(motion);
                isLoopEnabled = parameter.isLoopEnabled;
            }
            //bool enableSmooth = motion->enableSmooth();
            //bool enableRelocation = motion->enableRelocation();
            loader->deleteMotion(motion);
            motion = m_parent->insertMotionToModel(path, model);
            if (motion) {
                MotionParameter parameter;
                parameter.isLoopEnabled = isLoopEnabled;
                motion->setNullFrameEnable(isNullFrameEnabled);
                //motion->setEnableSmooth(enableSmooth);
                //motion->setEnableRelocation(enableRelocation);
                m_motions.remove(motionName);
                m_motions.insert(motionName, motion);
                m_motionParameters.insert(motion, parameter);
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
    /* モーション削除 */
    else if (type == kMotionDeleteCommand) {
        if (argc != 2) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(2).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        const QString &motionName = argv[1];
        if (m_models.contains(modelName) && m_motions.contains(motionName)) {
            vpvl::VMDMotion *motion = m_motions.value(motionName);
            loader->deleteMotion(motion);
            m_motions.remove(motionName);
        }
        else {
            qWarning("%s", qPrintable(kModelNotFound.arg(type).arg(modelName)));
        }
    }
    /* ステージ追加
     * - 引数が2つの場合はフロアと背景の画像を読み込む
     * - 引数が1つの場合はPMDとして読み込む
     * - いずれも SceneWidget ではなく Script が管理するため、GUI 上から変更できない
     */
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
            loader->deleteModel(m_stage);
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
    /* 光源の色変更 */
    else if (type == kLightColorCommand) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        vpvl::Vector4 color;
        parseColor(argv[0], color);
        m_parent->setLightColor(color);
        Arguments a; a << color.x() << color.y() << color.z() << color.w();
        emit eventDidPost(kLightColorEvent, a);
    }
    /* 光源の方向変更 */
    else if (type == kLightDirectionCommand) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        vpvl::Vector3 position;
        parsePosition(argv[0], position);
        m_parent->setLightPosition(position);
        Arguments a; a << position.x() << position.y() << position.z();
        emit eventDidPost(kLightDirectionEvent, a);
    }
    /* OpenJTalk による音声出力の開始 */
    else if (type == OpenJTalkSpeechEngine::kSynthStartCommand) {
        if (argc != 3) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(3).arg(argc)));
            return;
        }
        const QString &name = argv[0];
        const QString &style = argv[1];
        const QString &text = argv[2];
        m_speech.speech(name, style, text);
    }
    /* OpenJTalk による音声出力の終了(実装していない) */
    else if (type == OpenJTalkSpeechEngine::kSynthStopCommand) {
        if (argc != 2) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        // NOT SUPPORTED
    }
    /* OpenJTalk によるリップシンクモーションの開始 */
    else if (type == OpenJTalkSpeechEngine::kLipSyncStartCommand) {
        if (argc != 2) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(2).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        if (m_models.contains(modelName)) {
            const QString &sequence = argv[1];
            vpvl::PMDModel *model = m_models.value(modelName);
            vpvl::VMDMotion *newLipSyncMotion = m_globalLipSync.createMotion(sequence);
            if (newLipSyncMotion) {
                vpvl::VMDMotion *oldLipSyncMotion = m_motions.value(kLipSyncName);
                //newLipSyncMotion->setFull(false);
                if (oldLipSyncMotion)
                    loader->deleteMotion(oldLipSyncMotion);
                loader->setModelMotion(newLipSyncMotion, model);
            }
        }
    }
    /* OpenJTalk によるリップシンクモーションの終了 */
    else if (type == OpenJTalkSpeechEngine::kLipSyncStopCommand) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(1).arg(argc)));
            return;
        }
        const QString &modelName = argv[0];
        if (m_models.contains(modelName)) {
            vpvl::VMDMotion *motion = m_motions.value(kLipSyncName);
            loader->deleteMotion(motion);
            m_motions.remove(kLipSyncName);
        }
    }
    /*
     * カメラコマンド
     * - 引数が1つの場合はカメラモーションの読み込み
     * - 引数が3つまたは4つの場合はカメラ視点の変更を行う
     */
    else if (type == kCameraCommand) {
        if (argc == 1) {
            const QString &path = canonicalizePath(argv[0]);
            if (!m_parent->setCamera(path))
                qWarning("%s", qPrintable(tr("[%1] Failed loading camera motion: %2").arg(type).arg(path)));
        }
        else if (argc == 3 || argc == 4) {
            vpvl::Vector3 position, angle;
            parsePosition(argv[0], position);
            parsePosition(argv[1], angle);
            float distance = argv.at(2).toFloat();
            float fovy = 0;
            if (argc >= 4)
                fovy = argv.at(3).toFloat();
            m_parent->setCameraPerspective(&position, &angle, fovy > 0 ? &fovy : 0, &distance);
        }
        else {
            qWarning("%s", qPrintable(kInvalidArgumentVariant.arg(type).arg(1).arg(3).arg(4).arg(argc)));
        }
    }
    /* スクリプト特有の変数の設定 */
    else if (type == kValueSetCommand) {
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
    /* スクリプト特有の変数の削除 */
    else if (type == kValueUnsetCommand) {
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
    /*
     * スクリプト特有の変数の評価
     * - EQ: 変数Aと変数Bが同一であることの評価 (A == B)
     * - NE: 変数Aと変数Bが同一ではないことの評価 (A != B)
     * - LT: 変数Aが変数Bより大きいことの評価 (A > B)
     * - LE: 変数Aが変数Bと同一または大きいことの評価 (A >= B)
     * - GT: 変数Aが変数Bより小さいことの評価 (A < B)
     * - GE: 変数Aが変数Bと同一または小さいことの評価 (A <= B)
     */
    else if (type == kValueEvaluateCommand) {
        if (argc != 3) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(3).arg(argc)));
            return;
        }
        const QString &key = argv[0];
        const QString &op = argv[1];
        const QString &value = argv[2];
        if (!m_values.contains(key)) {
            qWarning("%s", qPrintable(tr("[%1] Evaluating %2 is not found").arg(type).arg(key)));
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
            qWarning("%s", qPrintable(tr("[%1] Operation %2 is invalid").arg(type).arg(op)));
        }
        Arguments a; a << key << op << value << (ret ? "TRUE" : "FALSE");
        emit eventDidPost(kValueEvaluateEvent, a);
    }
    /* スクリプト特有のタイマーの開始 */
    else if (type == kTimerStartCommand) {
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
    /* スクリプト特有のタイマーの停止 */
    else if (type == kTimerStopCommand) {
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
    /* プログラムの開始 */
    else if (type == kExecuteCommand) {
        if (argc != 1) {
            qWarning("%s", qPrintable(kInvalidArgumentFixed.arg(type).arg(2).arg(argc)));
            return;
        }
        const QString &argument = argv[0];
        const QUrl url(argument);
        if (url.isValid()) {
            QDesktopServices::openUrl(url);
        }
        else {
            QProcess process;
            process.startDetached(argument);
        }
    }
    else {
        qWarning("%s", qPrintable(tr("[%1] Command %1 is not supported").arg(type)));
    }
    qDebug() << "[COMMAND]" << type << argv;
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

    /* 現在の状態がないまたは現在の状態に対して次の状態がない場合は何もさせないようにする */
    if (!m_currentState || m_currentState->arcs.isEmpty())
        return jumped;

    foreach (const ScriptArc *arc, m_currentState->arcs) {
        const QStringList &args = input.arguments;
        const QStringList &arcargs = arc->input.arguments;
        const int argc = args.count();
        const int arcargc = arcargs.count();
        /* RECOG_EVENT_STOP は引数(単語)が複数あるため、特別扱いになっている */
        if (input.type == "RECOG_EVENT_STOP") {
            for (int i = 0; i < argc; i++) {
                const QString &arg = args[i];
                jumped = false;
                /* 一致する単語を探し、ひとつでも見つかれば該当の状態遷移に進ませる */
                for (int j = 0; j < arcargc; j++) {
                    if (arg == arcargs[j]) {
                        jumped = true;
                        break;
                    }
                }
                if (jumped)
                    break;
            }
        }
        /* 条件となるイベントと引数が同一であれば該当の状態遷移に進ませる */
        else if (input.type == arc->input.type && argc == arcargc) {
            jumped = true;
            for (int i = 0; i < argc; i++) {
                if (args[i] != arcargs[i]) {
                    jumped = false;
                    break;
                }
            }
        }
        /* 条件に一致するものがあったら該当の状態遷移を設定する */
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
    /* TRUE または FALSE を boolean にキャストする処理 */
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
        qWarning("%s", qPrintable(tr("Unexpected value %1 to boolean (%2 or %3)")
                                  .arg(value).arg(enable).arg(disable)));
    }
    return false;
}

bool Script::parsePosition(const QString &value, vpvl::Vector3 &v) const
{
    /* x,y,z を vpvl::Vector3(x, y, z) にキャストする処理 */
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
        qWarning("%s", qPrintable(tr("Unexpected value %1 to position").arg(value)));
    }
    return false;
}

bool Script::parseColor(const QString &value, vpvl::Vector4 &v) const
{
    /* r,g,b,a を vpvl::Color(r,g,b,a) にキャストする処理 */
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
        qWarning("%s", qPrintable(tr("Unexpected value %1 to color").arg(value)));
    }
    return false;
}

bool Script::parseRotation(const QString &value, vpvl::Quaternion &v) const
{
    /* x,y,z を vpvl::Quaternion (setEulerZYX 経由で設定する) にキャストする処理 */
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
        qWarning("%s", qPrintable(tr("Unexpected value %1 to rotation").arg(value)));
    }
    return false;
}

const QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> Script::stoppedMotions() const
{
    /* 停止されたモーションを取得 */
    SceneLoader *loader = m_parent->sceneLoader();
    QMultiMap<vpvl::PMDModel *, vpvl::VMDMotion *> ret;
    const QList<vpvl::PMDModel *> &models = loader->allModels();
    foreach (vpvl::PMDModel *model, models) {
        const vpvl::Array<vpvl::VMDMotion *> &motions = model->motions();
        const int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            vpvl::VMDMotion *motion = motions[i];
            if (!motion->isActive() && motion->isReachedTo(motion->maxFrameIndex()))
                ret.insert(model, motion);
        }
    }
    return ret;
}

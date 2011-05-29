#include "QMAVIScript.h"

#include <QtCore>
#include <MMDME/Common.h>

struct QMAVIScriptArc {
    QMAVIScriptArgument input;
    QMAVIScriptArgument output;
    struct QMAVIScriptState *nextState;
    QMAVIScriptArc(const QMAVIScriptArgument &i,
                   const QMAVIScriptArgument &o,
                   QMAVIScriptState *state)
        : input(i), output(o), nextState(state) {
    }
};

struct QMAVIScriptState {
    uint32_t index;
    QList<QMAVIScriptArc *> arcs;
    struct QMAVIScriptState *next;
    QMAVIScriptState(uint32_t i,
                     QMAVIScriptState *state)
        : index(i), next(state) {
    }
    ~QMAVIScriptState() {
        qDeleteAll(arcs);
    }
};

#if 0
static void DumpScriptState(QMAVIScriptState *state)
{
    if (!state)
        return;
    qDebug() << "state dump start";
    uint32_t index = state->index;
    foreach (QMAVIScriptArc *arc, state->arcs) {
        qDebug() << index << arc->input.type << arc->input.arguments
                 << arc->output.type << arc->output.arguments;
        DumpScriptState(state->next);
    }
    qDebug() << "state dump end";
}

static void DumpScriptStates(QLinkedList<QMAVIScriptState *> states)
{
    foreach (QMAVIScriptState *state, states)
        DumpScriptState(state);
}
#else
#define DumpScriptStates (void)
#endif

const QString QMAVIScript::kEPS = "<eps>";

QMAVIScript::QMAVIScript()
    : m_currentState(0)
{
}

QMAVIScript::~QMAVIScript()
{
    qDeleteAll(m_states);
}

bool QMAVIScript::load(QTextStream &stream)
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
                    QMAVIScriptArgument input(i.takeFirst(), i), output(o.takeFirst(), o);
                    addScriptArc(from, to, input, output);
                }
                else {
                    MMDAILogWarn("empty input arguments or empty output arguments found: %s", line.toUtf8().constData());
                }
            }
            else {
                MMDAILogWarn("invalid script line: %s", line.toUtf8().constData());
            }
        }
    }
    m_currentState = newScriptState(0);
    DumpScriptStates(m_states);

    return false;
}

bool QMAVIScript::setTransition(const QMAVIScriptArgument &input,
                                QMAVIScriptArgument &output)
{
    bool jumped = false;
    output.type = kEPS;
    output.arguments.clear();

    if (!m_currentState || m_currentState->arcs.isEmpty())
        return jumped;

    foreach (const QMAVIScriptArc *arc, m_currentState->arcs) {
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

void QMAVIScript::addScriptArc(int from,
                               int to,
                               const QMAVIScriptArgument &input,
                               const QMAVIScriptArgument &output)
{
    QMAVIScriptState *state1 = newScriptState(from);
    QMAVIScriptState *state2 = newScriptState(to);
    QMAVIScriptArc *arc = new QMAVIScriptArc(input, output, state2);
    state1->arcs.append(arc);
}

QMAVIScriptState *QMAVIScript::newScriptState(uint32_t index)
{
    QMAVIScriptState *head, *res;
    if (m_states.count() == 0) {
        res = new QMAVIScriptState(index, 0);
        m_states.append(res);
    }
    head = m_states.first();
    if (head->index == index) {
        res = head;
    }
    else if (head->index > index) {
        res = new QMAVIScriptState(index, head);
        m_states.prepend(res);
    }
    else {
        for (QMAVIScriptState *state = head; state; state = state->next) {
            QMAVIScriptState *&p = state->next;
            if (!p) {
                res = p = new QMAVIScriptState(index, 0);
                break;
            }
            else if (p->index == index) {
                res = p;
                break;
            }
            else if (index < p->index) {
                QMAVIScriptState *q = p;
                res = p = new QMAVIScriptState(index, q);
                break;
            }
        }
        if (!res)
            MMDAILogWarn("unknown state: %d", index);
    }
    return res;
}

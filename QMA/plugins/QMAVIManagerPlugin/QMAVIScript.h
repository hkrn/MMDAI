#ifndef QMAVISCRIPT_H
#define QMAVISCRIPT_H

#include <QtCore/QLinkedList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

struct QMAVIScriptArgument {
    QString type;
    QStringList arguments;
    QMAVIScriptArgument()
        : type(QString()), arguments(QStringList()) {
    }
    QMAVIScriptArgument(const QMAVIScriptArgument &value)
        : type(value.type), arguments(value.arguments) {
    }
    QMAVIScriptArgument(const QString &t, const QStringList &args)
        : type(t), arguments(args) {
    }
    void operator =(const QMAVIScriptArgument &value) {
        this->type = value.type;
        this->arguments = value.arguments;
    }
};

typedef struct QMAVIScriptState QMAVIScriptState;

class QMAVIScript
{
public:
    static const QString kEPS;

    QMAVIScript();
    ~QMAVIScript();

    bool load(QTextStream &stream);
    bool setTransition(const QMAVIScriptArgument &input,
                       QMAVIScriptArgument &output);

private:
    void addScriptArc(int from,
                      int to,
                      const QMAVIScriptArgument &input,
                      const QMAVIScriptArgument &output);
    QMAVIScriptState *newScriptState(uint32_t index);

    QLinkedList<QMAVIScriptState *> m_states;
    QMAVIScriptState *m_currentState;
};

#endif // QMAVISCRIPT_H

#ifndef QMAVILUAPLUGIN_H
#define QMAVILUAPLUGIN_H

#include <lua.hpp>
#include <QMap>

#include "QMAPlugin.h"

class QMAVILuaPlugin : public QMAPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMAPlugin)

public:
    QMAVILuaPlugin();
    ~QMAVILuaPlugin();

public slots:
    void load(MMDAI::SceneController *controller, const QString &baseName);
    void unload();
    void receiveCommand(const QString &command, const QList<QVariant> &arguments);
    void receiveEvent(const QString &type, const QList<QVariant> &arguments);

    void pushCommand(const QString &command, const QList<QVariant> &arguments);
    void pushEvent(const QString &command, const QList<QVariant> &arguments);
    void broadcast();

signals:
    void commandPost(const QString &command, const QList<QVariant> &arguments);
    void eventPost(const QString &type, const QList<QVariant> &arguments);

private:
    void handleInvoke(const QString &command, const QList<QVariant> &arguments, const char *invoke);

    QMap<QString, QList<QVariant> > m_commands;
    QMap<QString, QList<QVariant> > m_events;
    lua_State *m_state;
};

#endif // QMAVILUAPLUGIN_H

#include <QDir>
#include <QTextCodec>

#include "MMDME/Common.h"

#include "QMAVILuaPlugin.h"

static int mmdai_command(lua_State *state)
{
    int top = lua_gettop(state);
    QMAVILuaPlugin *plugin = static_cast<QMAVILuaPlugin *>(lua_touserdata(state, lua_upvalueindex(1)));
    const char *command = lua_tostring(state, 1);
    if (plugin != NULL && command != NULL) {
        QList<QVariant> arguments;
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        for (int i = 2; i <= top; i++) {
            const char *s = lua_tostring(state, i);
            arguments << codec->toUnicode(s, strlen(s));
        }
        plugin->pushCommand(QString(command), arguments);
    }
    lua_settop(state, top);
    return 1;
}

static int mmdai_event(lua_State *state)
{
    int top = lua_gettop(state);
    QMAVILuaPlugin *plugin = static_cast<QMAVILuaPlugin *>(lua_touserdata(state, lua_upvalueindex(1)));
    const char *type = lua_tostring(state, 1);
    if (plugin != NULL && type != NULL) {
        QList<QVariant> arguments;
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        for (int i = 2; i <= top; i++) {
            const char *s = lua_tostring(state, i);
            arguments << codec->toUnicode(s, strlen(s));
        }
        plugin->pushEvent(QString(type), arguments);
    }
    lua_settop(state, top);
    return 1;
}

QMAVILuaPlugin::QMAVILuaPlugin()
    : m_state(0)
{
    m_state = lua_open();
    luaL_openlibs(m_state);
}

QMAVILuaPlugin::~QMAVILuaPlugin()
{
    lua_close(m_state);
    m_state = 0;
}

void QMAVILuaPlugin::load(MMDAI::SceneController *controller, const QString &baseName)
{
    Q_UNUSED(controller);
    static struct luaL_reg funcs[] = {
        { "command", mmdai_command },
        { "event", mmdai_event },
    };
    Q_UNUSED(controller);
    lua_newtable(m_state);
    for (unsigned int i = 0; i < sizeof(funcs)/sizeof(funcs[0]); i++) {
        lua_pushstring(m_state, funcs[i].name);
        lua_pushlightuserdata(m_state, this);
        lua_pushcclosure(m_state, funcs[i].func, 1);
        lua_settable(m_state, -3);
    }
    lua_setglobal(m_state, "mmdai");

    QString path = QFile(QString("MMDAIUserData:/%s.lua").arg(baseName)).fileName();
    int top = lua_gettop(m_state);
    int ret = luaL_dofile(m_state, path.toUtf8().constData());
    if (ret != 0) {
        MMDAILogWarn("Executing a lua script error: %s", lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
    }
    lua_settop(m_state, top);
    broadcast();
}

void QMAVILuaPlugin::unload()
{
}

void QMAVILuaPlugin::receiveCommand(const QString &command, const QList<QVariant> &arguments)
{
    handleInvoke(command, arguments, "mmdai_handle_command");
}

void QMAVILuaPlugin::receiveEvent(const QString &type, const QList<QVariant> &arguments)
{
    handleInvoke(type, arguments, "mmdai_handle_event");
}

void QMAVILuaPlugin::handleInvoke(const QString &command, const QList<QVariant> &arguments, const char *invoke)
{
    QByteArray bytes;
    int top = lua_gettop(m_state);
    lua_getglobal(m_state, invoke);
    bytes = command.toUtf8();
    lua_pushstring(m_state, bytes.constData());
    foreach (QVariant argv, arguments) {
        bytes = argv.toString().toUtf8();
        lua_pushstring(m_state, bytes.constData());
    }
    int ret = lua_pcall(m_state, arguments.count() + 1, 0, 0);
    if (ret != 0) {
        qWarning("Invoke %s error: %s", invoke, lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
    }
    lua_settop(m_state, top);
    broadcast();
}

void QMAVILuaPlugin::pushCommand(const QString &command, const QList<QVariant> &arguments)
{
    m_commands.insert(command, arguments);
}

void QMAVILuaPlugin::pushEvent(const QString &command, const QList<QVariant> &arguments)
{
    m_events.insert(command, arguments);
}

void QMAVILuaPlugin::broadcast()
{
    QMap<QString, QList<QVariant> > commands = m_commands;
    m_commands.clear();
    QMapIterator<QString, QList<QVariant> > commandIterator(commands);
    while (commandIterator.hasNext()) {
        commandIterator.next();
        emit commandPost(commandIterator.key(), commandIterator.value());
    }
    QMap<QString, QList<QVariant> > events = m_events;
    m_events.clear();
    QMapIterator<QString, QList<QVariant> > eventIterator(events);
    while (eventIterator.hasNext()) {
        eventIterator.next();
        emit eventPost(eventIterator.key(), eventIterator.value());
    }
}

Q_EXPORT_PLUGIN2(qma_vilua_plugin, QMAVILuaPlugin);
